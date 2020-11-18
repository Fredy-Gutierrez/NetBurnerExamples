/* Revision: 3.2.0 */

/******************************************************************************
* Copyright 1998-2020 NetBurner, Inc.  ALL RIGHTS RESERVED
*
*    Permission is hereby granted to purchasers of NetBurner Hardware to use or
*    modify this computer program for any use as long as the resultant program
*    is only executed on NetBurner provided hardware.
*
*    No other rights to use this program or its derivatives in part or in
*    whole are granted.
*
*    It may be possible to license this or other NetBurner software for use on
*    non-NetBurner Hardware. Contact sales@Netburner.com for more information.
*
*    NetBurner makes no representation or warranties with respect to the
*    performance of this computer program, and specifically disclaims any
*    responsibility for any damages, special or consequential, connected with
*    the use of this program.
*
* NetBurner
* 5405 Morehouse Dr.
* San Diego, CA 92121
* www.netburner.com
******************************************************************************/

#include <ftpd.h>
#include <htmlfiles.h>
#include <init.h>
#include <netinterface.h>
#include <stdlib.h>

const char *AppName = "FPTD Expose HTML";

void UserMain(void *pd)
{
    init();                                       // Initialize network stack
    StartHttp();                                  // Start web server, default port 80
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

    // Start FTP server with task priority higher than UserMain()
    InterfaceBlock *pifb = GetInterfaceBlock();

    // Wait here until we know we have a valid IP address
    while (pifb->dhcpClient.GetDHCPState() != SDHCP_CMPL)
    {
        OSTimeDly(1);
    }

    int status = FTPDStart(21, MAIN_PRIO - 1);
    if (status == FTPD_OK)
    {
        iprintf("Started FTP Server\r\n");
        if (pifb != nullptr) { iprintf("You can use Internet Explorer to view files at URL \"ftp://%hI\"\r\n", pifb->ip4.cur_addr.i4); }
    }
    else
        iprintf("** Error: %d. Could not start FTP Server\r\n", status);

    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND * 1);
    }
}

/*******************************************************************************
 * Expose the HTML file system as read only
 ******************************************************************************/

// A set of structures to join together into a simple file system
struct directory_struct;

struct file_struct   // One for each file
{
    char file_name[40];
    directory_struct *pFileLivesInDir;   // The directory this file lives in
    file_struct *pNext;                  // The next file in the linked list
    int file_size;                       // The size of the file
};

struct directory_struct   // One for each directory
{
    char directory_name[40];      // The directory name
    file_struct *pFiles;          // The list of files in this dir, possible nullptr
    directory_struct *pNext;      // The next subdir in a linked list of subdirs
    directory_struct *pSubDirs;   // Subdirs under this dir
    directory_struct *pParent;   // The parent dir, null For the root dir
};

static directory_struct *pDirRoot;   // The root directory

/**
 *   getDirString
 *
 *   This function is used to format a file name into a directory string
 *   that the FTP client will recognize. In this very simple case we
 *   are just going to hard code the values, with the exception of the name
 *   and size.
 */
void getDirString(bool dir, char *FileName, char *DirStr, int size)
{
    char tmp[80];

    DirStr[0] = dir ? 'd' : '-';   // '-' for file, 'd' for directory
    DirStr[1] = 0;

    // permissions, hard link, user, group
    strcat(DirStr, "rw-rw-rw- 1 user group ");

    sniprintf(tmp, 80, "%9d ", size);
    strcat(DirStr, tmp);
    strcat(DirStr, "Jan 1 00:00 ");
    strcat(DirStr, FileName);
}

/**
 *  BuildDirTree
 *
 *  This function populates a directory/file tree from the HTML directory
 *  definition stored in HTML data.cpp and defined in htmlfiles.h
 */
void BuildDirTree()
{
    if (pDirRoot) { return; }

    // Create the root
    pDirRoot = (directory_struct *)malloc(sizeof(directory_struct));
    pDirRoot->directory_name[0] = 0;
    pDirRoot->pParent = nullptr;
    pDirRoot->pNext = nullptr;
    pDirRoot->pSubDirs = nullptr;
    pDirRoot->pFiles = nullptr;

    char temp_buf[80];

    iprintf("Building the tree...\r\n");

    // Now walk over each stored file populating the tree
    // n_file_record is determined during compilation
    for (unsigned int i = 0; i < n_file_record; i++)
    {
        directory_struct *pd = pDirRoot;
        strcpy(temp_buf, file_record[i].fname);
        char *cp = temp_buf;
        char *cpn = nullptr;

        while ((cpn = strchr(cp, '/')) != 0)
        {
            *cpn = 0;
            directory_struct *pn = pd->pSubDirs;

            while (pn)
            {
                if (strcmp(cp, pn->directory_name) == 0)
                {
                    pd = pn;
                    break;
                }

                pn = pn->pNext;
            }

            if (pd != pn)
            {
                // Add dir
                pn = (directory_struct *)malloc(sizeof(directory_struct) + 32);
                strncpy(pn->directory_name, cp, 40);
                pn->pParent = pd;
                pn->pNext = pd->pSubDirs;
                pd->pSubDirs = pn;
                pn->pSubDirs = nullptr;
                pn->pFiles = nullptr;
                pd = pn;
            }

            cp = cpn + 1;
        }

        // We have descended the dir tree - now to add file
        file_struct *pf = (file_struct *)malloc(sizeof(file_struct) + 32);
        pf->pFileLivesInDir = pd;
        pf->pNext = pd->pFiles;
        pf->file_size = file_record[i].siz;
        pd->pFiles = pf;
        strncpy(pf->file_name, cp, 40);
    }
}

/**
 *  GetDir
 *  Find a given directory in the tree, or return null
 */
directory_struct *GetDir(const char *d)
{
    if (pDirRoot == nullptr) { BuildDirTree(); }
    if (d[0] == 0) { return pDirRoot; }

    char temp_buf[81] = {};   // Initialize array - extra byte reserved for null termination
    strncpy(temp_buf, d, 80);
    char *cp = temp_buf;
    char *cpn = nullptr;

    directory_struct *pd = pDirRoot->pSubDirs;

    // Now we start searching
    while ((cpn = strchr(cp, '/')) != 0)
    {
        *cpn = 0;

        while (pd)
        {
            if (strcmp(pd->directory_name, cp) == 0) { break; }
            pd = pd->pNext;
        }

        if (pd == nullptr) { return nullptr; }

        cp = cpn + 1;
        pd = pd->pSubDirs;
    }

    while (pd)
    {
        if (strcmp(pd->directory_name, cp) == 0) { return pd; }
        pd = pd->pNext;
    }

    return nullptr;
}

/**
 *  FTPD_ListFile
 *  List all the files FTPD callback - see docs in ftpd.h
 */
int FTPD_ListFile(const char *current_directory, void *pSession, FTPDCallBackReportFunct *pFunc, int handle)
{
    directory_struct *pdir = GetDir(current_directory);

    if (pdir)
    {
        file_struct *pfil = pdir->pFiles;

        while (pfil)
        {
            char name[255];
            getDirString(false, pfil->file_name, name, pfil->file_size);
            pFunc(handle, name);   // Report the file
            pfil = pfil->pNext;
        }
    }

    return FTPD_OK;
}

/**
 *  FTPD_ListSubDirectories
 *  List all the subdirectories FTPD callback - see docs in ftpd.h
 */
int FTPD_ListSubDirectories(const char *current_directory, void *pSession, FTPDCallBackReportFunct *pFunc, int handle)
{
    directory_struct *pdir = GetDir(current_directory);

    if (pdir)
    {
        pdir = pdir->pSubDirs;

        while (pdir)
        {
            char name[255];
            getDirString(true, pdir->directory_name, name, 0);
            pFunc(handle, name);   // Report the dir
            pdir = pdir->pNext;
        }
    }

    return FTPD_OK;
}

/**
 *  FTPD_DirectoryExists
 *  Check to see if a directory exists
 */
int FTPD_DirectoryExists(const char *full_directory, void *pSession)
{
    if (full_directory[0] == 0) { return FTPD_OK; }
    if (GetDir(full_directory)) { return FTPD_OK; }
    return FTPD_FAIL;
}

/**
 *  FTPD_SendFileToClient
 *  Send a file to the client
 */
int FTPD_SendFileToClient(const char *full_directory, const char *file_name, void *pSession, int fd)
{
    char Temp_FileName_Buffer[80];
    Temp_FileName_Buffer[0] = 0;

    if (full_directory[0])
    {
        strcat(Temp_FileName_Buffer, full_directory);
        strcat(Temp_FileName_Buffer, "/");
    }

    strcat(Temp_FileName_Buffer, file_name);

    // This is a function from HTMLFiles.h that sends files stored in the HTML subdir
    if (SendFileFragment(Temp_FileName_Buffer, fd) > 0) { return FTPD_OK; }
    else
    {
        return FTPD_FAIL;
    }
}

/**
 *  FTPD_FileExists
 */
int FTPD_FileExists(const char *full_directory, const char *file_name, void *pSession)
{
    char Temp_FileName_Buffer[80];
    Temp_FileName_Buffer[0] = 0;

    if (full_directory[0])
    {
        strcat(Temp_FileName_Buffer, full_directory);
        strcat(Temp_FileName_Buffer, "/");
    }

    strcat(Temp_FileName_Buffer, file_name);

    if (GetRecordFromName(Temp_FileName_Buffer)) { return FTPD_OK; }
    else
    {
        return FTPD_FAIL;
    }
}

/**
 *  FTPDSessionEnd
 *  Report on a session end
 */
void FTPDSessionEnd(void *pSession)
{
    iprintf("Ending session %d\r\n", (int)pSession);
}

/**
 *  FTPDSessionStart
 *  Always accept a session, real app would check user/password here
 */
void *FTPDSessionStart(const char *user, const char *passwd, const IPADDR4 hi_ip)
{
    static int n_Sessions = 0;
    // Session can always be created
    return (void *)(++n_Sessions);
}

/**********************************************************************************
 * Functions need to be declared, but are not supported
 **********************************************************************************/

/**
 *  FTPD_CreateSubDirectory
 */
int FTPD_CreateSubDirectory(const char *current_directory, const char *new_dir, void *pSession)
{
    // Always fails
    return FTPD_FAIL;
}

/**
 *  FTPD_DeleteSubDirectory
 */
int FTPD_DeleteSubDirectory(const char *current_directory, const char *sub_dir, void *pSession)
{
    // Always fails
    return FTPD_FAIL;
}

/**
 *  FTPD_GetFileFromClient
 */
int FTPD_GetFileFromClient(const char *full_directory, const char *file_name, void *pSession, int fd)
{
    // Always fails
    return FTPD_FAIL;
}

/**
 *  FTPD_DeleteFile
 */
int FTPD_DeleteFile(const char *current_directory, const char *file_name, void *pSession)
{
    // Always fails
    return FTPD_FAIL;
}

/**
 *  FTPD_AbleToCreateFile
 */
int FTPD_AbleToCreateFile(const char *full_directory, const char *file_name, void *pSession)
{
    // Always fails
    return FTPD_FAIL;
}

/**
 *  FTPD_Rename
 */
int FTPD_Rename(const char *current_directory, const char *cur_file_name, const char *new_file_name, void *pSession)
{
    return FTPD_FAIL;
}

/**
 *  FTPD_GetFileSize
 */
int FTPD_GetFileSize(const char *full_directory, const char *file_name)
{
    return FTPD_FILE_SIZE_NOSUCH_FILE;
}
