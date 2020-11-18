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
#include <nbtime.h>

#include "FileSystemUtils.h"
#include "effs_time.h"

#define FTP_PRIO (MAIN_PRIO - 1)

const char *AppName = "Send Mail Attachment with EFFS";

/**
 * @brief Initialize EFFS Flash file system
 */
void InitFileSystem()
{
    /* The following call to f_enterFS() must be called in every task that accesses
     *  the file system.  This must only be called once in each task and must be done before
     *  any other file system functions are used.  Up to 10 tasks can be assigned to use
     *  the file system. Any task may also be removed from accessing the file system with a
     *  call to the function f_releaseFS().
     */
    OSChangePrio(MAIN_PRIO);   // enable for UserMain
    f_enterFS();

    InitExtFlash();   // Initialize the CFC or SD/MMC external flash drive

    // Try setting the time via a NTP Server, if that fails, set it
    // manually.
    if (SetTimeNTP())
    {
        // tzsetchar((char*)"EST5EDT4,M3.2.0/01:00:00,M11.1.0/02:00:00");
        // tzsetchar((char*)"CST6CDT5,M3.2.0/01:00:00,M11.1.0/02:00:00");
        // tzsetchar((char*)"MST7MDT6,M3.2.0/01:00:00,M11.1.0/02:00:00");
        tzsetchar((char *)"PST8PDT7,M3.2.0/01:00:00,M11.1.0/02:00:00");
    }
    else
    {
        iprintf("NTP failed, setting time manually\r\n");
        // month, day, dow, year, hour, min, sec
        SetTimeManual(5, 14, 3, 2006, 11, 35, 0);
    }
    DisplaySystemTime();
    iprintf("\r\n");

    DisplayEffsSpaceStats();   // Display file space usage
    DumpDir();                 // Display flash card files and directories
}

/**
 * @brief Initialize FTP server to access SD Flash card
 */
void InitFtpServer()
{
    // We now must also enter the file system for the FTP task. This
    // must be called before the OSTaskCreate() function.
    iprintf("Enabled EFFS access for FTP task\r\n");
    OSChangePrio(FTP_PRIO);
    f_enterFS();
    OSChangePrio(MAIN_PRIO);

    // Start FTP server with task priority higher than UserMain()
    int status = FTPDStart(21, FTP_PRIO);
    if (status == FTPD_OK)
    {
        iprintf("Started FTP Server\r\n");
        if (F_LONGFILENAME == 1) { iprintf("Long file names are supported\r\n"); }
        else
        {
            iprintf("Long file names are not supported- only 8.3 format\r\n");
        }
    }
    else
    {
        iprintf("** Error: %d. Could not start FTP Server\r\n", status);
    }
}

/**
 * @brief Initialize Web server to access SD Flash card
 */
void InitWebServer()
{
    // Enable EFFS access for the HTTP task. This
    // must be called before the StartHttp() function.
    iprintf("Enabled EFFS access for HTTP task\r\n");
    OSChangePrio(HTTP_PRIO);
    f_enterFS();
    int rv = f_chdrive(EXT_FLASH_DRV_NUM);

    if (rv == F_NO_ERROR) { iprintf("Drive change successful\r\n"); }
    else
    {
        iprintf("Drive change failed: ");
        DisplayEffsErrorCode(rv);
    }
    OSChangePrio(MAIN_PRIO);
    StartHttps();
}

/**
 * @brief Main entry point for the example
 */
void UserMain(void *pd)
{
    init();                                       // Initialize network stack
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

    InitFileSystem();

    InitWebServer();

    InitFtpServer();

    // Since main will no longer be accessing the file system we can release it
    f_releaseFS();

    // Loop forever - all the action takes place from the web page
    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND);
    }
}
