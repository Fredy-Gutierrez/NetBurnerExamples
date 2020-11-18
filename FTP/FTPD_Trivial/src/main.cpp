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
#include <init.h>
#include <iosys.h>
#include <nbrtos.h>
#include <string.h>

const char *AppName = "FTPD Trivial";

#define READ_FILENAME "ReadFile.txt"
#define READ_FILEDATA "This will be the contents of ReadFile.txt \r\n"

void UserMain(void *pd)
{
    init();

    iprintf("Starting FTP Server on port 21 ...");
    OSTimeDly(TICKS_PER_SECOND * 2);
    FTPDStart(21, MAIN_PRIO - 1);
    iprintf(" complete\r\n");

    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND);
    }
}

/********************************************************************************************
         FILE READ/LIST FUNCTIONS
********************************************************************************************/
/**
 * getdirstring
 *
 * This function is used to format a file name into a directory string
 * that the FTP client will recognize. In this very simple case we
 * are just going to hard code the values.
 */
void getdirstring(char *FileName, long FileSize, char *DirStr)
{
    char tmp[80];

    DirStr[0] = '-';   // '-' for file, 'd' for directory
    DirStr[1] = 0;

    // permissions, hard link, user, group
    strcat(DirStr, "rw-rw-rw- 1 user group ");

    sniprintf(tmp, 80, "%9ld ", FileSize);
    strcat(DirStr, tmp);

    strcat(DirStr, "Jan 01 00:00 ");

    strcat(DirStr, FileName);
}

/**
 *  FTPD_ListFile
 *
 *  This function is called by the FTP Server in response to a FTP Client's
 *  request to list the files (e.g. the "ls" command)
 */
int FTPD_ListFile(const char *current_directory, void *pSession, FTPDCallBackReportFunct *pFunc, int handle)
{
    char DirStr[256];

    // Only one file exists ReadFile.txt
    getdirstring((char *)READ_FILENAME, strlen(READ_FILEDATA), DirStr);
    pFunc(handle, DirStr);
    return FTPD_OK;
}

/**
 *  FTPD_SendFileToClient
 *
 *  This function is called by the FTP Server in response to a FTP
 *  Client's request to receive a file. In this example, only ReadFile.txt
 *  is available for download, and it's contents are hard coded to the
 *  string in the writestring() function.
 */
int FTPD_SendFileToClient(const char *full_directory, const char *file_name, void *pSession, int fd)
{
    // Only one file exists
    if (strcmp(file_name, READ_FILENAME) == 0)
    {
        // Now send the "file", which is just one line of text in this example
        writestring(fd, READ_FILEDATA);
        return FTPD_OK;
    }
    else
    {
        return FTPD_FAIL;
    }
}

/**
 *  FTPD_FileExists
 *
 *  This function is called by the FTP Server to determine if a file exists.
 */
int FTPD_FileExists(const char *full_directory, const char *file_name, void *pSession)
{
    // Only one file exists
    if (strcmp(file_name, READ_FILENAME) == 0) { return FTPD_OK; }

    return FTPD_FAIL;
}

/********************************************************************************************
         FILE SEND FUNCTIONS
********************************************************************************************/

/**
 *  ShowFileContents
 *
 *  This function reads the data stream from the input stream file descriptor fd
 *  and displays it to stdout, which is usually the COM1 serial port on the
 *  NetBurner board.
 */
void ShowFileContents(int fdr)
{
    iprintf("\r\n[\r\n");
    int rv = 0;
    char tmp_resultbuff[255];
    do
    {
        rv = ReadWithTimeout(fdr, tmp_resultbuff, 255, 20);
        iprintf("Bytes Read: %d", rv);
        if (rv >= 0)
        {
            tmp_resultbuff[rv] = 0;
            iprintf(", %s", tmp_resultbuff);
        }
    } while (rv > 0);
    iprintf("\r\n]\r\n");
}

/**
 *  FTPD_GetFileFromClient
 *
 *  This function gets called by the FTP Server when a FTP client
 *  sends a file. File must be named WriteFile.txt.
 */
int FTPD_GetFileFromClient(const char *full_directory, const char *file_name, void *pSession, int fd)
{
    if (strcmp(file_name, "WriteFile.txt") == 0)
    {
        ShowFileContents(fd);
        return FTPD_OK;
    }
    return FTPD_FAIL;
}

/**
 *  FTPD_AbleToCreateFile
 *
 *  This function gets called by the FTP Server to determine if it is ok to
 *  create a file on the system. In this case is will occur when a FTP
 *  client sends a file. File must be named WriteFile.txt.
 */
int FTPD_AbleToCreateFile(const char *full_directory, const char *file_name, void *pSession)
{
    if (strcmp(file_name, "WriteFile.txt") == 0) { return FTPD_OK; }
    return FTPD_FAIL;
}

/**
 *  FTPDSessionStart
 *
 *  The parameters passed to you in this function show the entered
 *  user name, password and IP address they came from. You can
 *  modify this function any way you wish for authentication.
 *
 *  Return Values:
 *      0   = Authentication failed
 *      > 0 = Authentication passed
 */
void *FTPDSessionStart(const char *user, const char *passwd, const IPADDR4 hi_ip)
{
    return (void *)1;   //  Return a non zero value
}

/****************FTPD Functions that are not supported/used in this trivial case *******************/
void FTPDSessionEnd(void *pSession)
{
    /* Do nothing */
}

int FTPD_ListSubDirectories(const char *current_directory, void *pSession, FTPDCallBackReportFunct *pFunc, int handle)
{
    /* No directories to list */
    return FTPD_OK;
}

int FTPD_DirectoryExists(const char *full_directory, void *pSession)
{
    return FTPD_FAIL;
}

int FTPD_CreateSubDirectory(const char *current_directory, const char *new_dir, void *pSession)
{
    return FTPD_FAIL;
}

int FTPD_DeleteSubDirectory(const char *current_directory, const char *sub_dir, void *pSession)
{
    return FTPD_FAIL;
}

int FTPD_DeleteFile(const char *current_directory, const char *file_name, void *pSession)
{
    return FTPD_FAIL;
}

int FTPD_Rename(const char *current_directory, const char *cur_file_name, const char *new_file_name, void *pSession)
{
    return FTPD_FAIL;
}

int FTPD_GetFileSize(const char *full_directory, const char *file_name)
{
    return FTPD_FILE_SIZE_NOSUCH_FILE;
}
