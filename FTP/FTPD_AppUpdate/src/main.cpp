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

#include <hal.h>   // access for ForceReboot() function
#include <hal.h>
#include <ftpd.h>
#include <init.h>
#include <nbrtos.h>
#include <nbupdate.h>
#include <StreamUpdate.h>
#include <string.h>

BOOL bNeedToReset = false;
const char *AppName = "FTPD App Update";
extern const char LinkTime[];

void UserMain(void *pd)
{
    init();

    iprintf("Link Time: %s\n", LinkTime);

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

int FTPD_ListFile(const char *current_directory, void *pSession, FTPDCallBackReportFunct *pFunc, int handle)
{
    /*  The 2 UserFlash files exist for reading by a FTP client.
     *  UserFlash.s19 = User Flash Parameter storage area in ASCII S19 format
     *  UserFlash.bin = User Flash Parameter storage area in binary format
     *  The Application_APP.s19 file name is there as a reminder of what a FTP client
     *  needs to name the image file sent as a flash application update.
     */
    pFunc(handle, "UserFlash.s19");
    pFunc(handle, "UserFlash.bin");
    pFunc(handle, "Application_APP.s19");
    pFunc(handle, "Application.bin");
    return FTPD_OK;
}

int FTPD_SendFileToClient(const char *full_directory, const char *file_name, void *pSession, int fd)
{
    /* Only 2 files exist */
    if (strcmp(file_name, "UserFlash.s19") == 0)
    {
        if (SendUserFlashToStreamAsS19(fd) == STREAM_UP_OK) { return FTPD_OK; }
    }
    else if (strcmp(file_name, "UserFlash.bin") == 0)
    {
        if (SendUserFlashToStreamAsBinary(fd) == STREAM_UP_OK) { return FTPD_OK; }
    }
    else if (strcmp(file_name, "Application_APP.s19") == 0)
    {
        if (SendApplicationCodeAsS19(fd) == STREAM_UP_OK) { return FTPD_OK; }
    }

    return FTPD_FAIL;
}

int FTPD_FileExists(const char *full_directory, const char *file_name, void *pSession)
{
    /* Only 2 files exist */
    if (strcmp(file_name, "UserFlash.s19") == 0) { return FTPD_OK; }
    if (strcmp(file_name, "UserFlash.bin") == 0) { return FTPD_OK; }
    if (strcmp(file_name, "Application_APP.s19") == 0) { return FTPD_OK; }

    return FTPD_FAIL;
}

/********************************************************************************************
         FILE SEND FUNCTIONS
********************************************************************************************/

AppUpdateRecord *newAppImage;

int FTPD_GetFileFromClient(const char *full_directory, const char *file_name, void *pSession, int fd)
{
    if (strcmp(file_name, "UserFlash.s19") == 0)
    {
        if (ReadS19UserFlashFromStream(fd) == STREAM_UP_OK) { return FTPD_OK; }
    }
    else if (strcmp(file_name, "UserFlash.bin") == 0)
    {
        if (ReadBinaryUserFlashFromStream(fd) == STREAM_UP_OK) { return FTPD_OK; }
    }
    else if ((strcmp(file_name, "Application_APP.s19") == 0) || (strcmp(file_name, "Application.bin") == 0))
    {
        if (!newAppImage)
        {
            newAppImage = new AppUpdateRecord;
            if (!newAppImage) { return FTPD_FAIL; }
            if (UpdateFromStream(fd, newAppImage, 0) == NBUP_ERR_NO_ERR)
            {
                bNeedToReset = true;
                return FTPD_OK;
            }
            delete newAppImage;
        }
    }

    return FTPD_FAIL;
}

int FTPD_AbleToCreateFile(const char *full_directory, const char *file_name, void *pSession)
{
    if (strcmp(file_name, "UserFlash.s19") == 0) { return FTPD_OK; }
    else if (strcmp(file_name, "UserFlash.bin") == 0)
    {
        return FTPD_OK;
    }
    else if ((strcmp(file_name, "Application_APP.s19") == 0) || (strcmp(file_name, "Application.bin") == 0))
    {
        return newAppImage ? FTPD_FAIL : FTPD_OK;
    }
    return FTPD_FAIL;
}

/**
 *  FTPDSessionStart
 *
 *  The parameters passed to you in this function show the entered
 *  username, password and IP address they came from. You can
 *  modify this function any way you wish for authentication.
 *      Return 0 = Auth failed
 *      Return > 0 = Auth passed
 */
void *FTPDSessionStart(const char *user, const char *passwd, const IPADDR4 hi_ip)
{
    return (void *)1; /* Return a non zero value */
}

/**
 *  FTPDSessionEnd
 *
 *  If the flash memory was programmed with a new application program, then call
 *  the ForceReboot() function to reboot the board when the FTP Client ends the
 *  session
 */
void FTPDSessionEnd(void *pSession)
{
    if (bNeedToReset)
    {
        bNeedToReset = false;
        ForceReboot();
        if (newAppImage) { delete newAppImage; }   // Just in case...
    }
}

/**
 *  FTPD_ListSubDirectories
 */
int FTPD_ListSubDirectories(const char *current_directory, void *pSession, FTPDCallBackReportFunct *pFunc, int handle)
{
    /* No directories to list */
    return FTPD_OK;
}

/****************************************************************************************
 The following FTPD functions are not used in this example, but need to be declared.
 *****************************************************************************************/

int FTPD_DirectoryExists(const char *full_directory, void *pSession)
{
    return FTPD_FAIL;
}

int FTPD_CreateSubDirectory(const char *current_directory, const char *new_dir, void *pSession)
{
    // Always fails
    return FTPD_FAIL;
}

int FTPD_DeleteSubDirectory(const char *current_directory, const char *sub_dir, void *pSession)
{
    // Always fails
    return FTPD_FAIL;
}

int FTPD_DeleteFile(const char *current_directory, const char *file_name, void *pSession)
{
    // Always fails
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
