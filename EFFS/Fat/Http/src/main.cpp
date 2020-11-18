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

#include <effs_fat/fat.h>
#include <ftpd.h>
#include <init.h>
#include <nbtime.h>
#include <startnet.h>

#include "FileSystemUtils.h"
#include "effs_time.h"
#include "http_f.h"

#if (defined(USE_MMC) && defined(MOD5441X))
#define MULTI_MMC TRUE   // For modules with onboard flash sockets, even if you are using external flash cards
#include <effs_fat/multi_drive_mmc_mcf.h>
#elif (defined(USE_MMC))
#include <effs_fat/mmc_mcf.h>
#elif (defined(USE_CFC))
#include <effs_fat/cfc_mcf.h>
#endif

#if (defined(USE_MMC))
const char *AppName = "EFFS HTTP MMC/SD";
#elif (defined(USE_CFC))
const char *AppName = "EFFS HTTP CFC";
#endif

#define FTP_PRIO (MAIN_PRIO - 2)

/**
 * @brief Display command menu.
 */
void DisplayMenu()
{
    iprintf("\r\n\n--- Main Menu ---\r\n");
    iprintf("D - Display Directory\r\n");
    iprintf("E - Display TestFile.txt\r\n");
    iprintf("F - Format SD Flash card (warning: data will be lost!)\r\n");
    iprintf("G - fgets_test()\r\n");
    iprintf("H - fputs_test()\r\n");
    iprintf("P - fprintf_test()\r\n");
    iprintf("S - Display Space usage\r\n");
    iprintf("T - Display system Time\r\n");
    iprintf("? - Display Menu\r\n");
    iprintf("> ");
}

/**
 * @brief Handle commands with  a trivially simple command dispatcher.
 */
void ProcessCommand(char *buffer)
{
    switch (toupper(buffer[0]))
    {
        case 'D':
        {
            iprintf("Directory Contents:\r\n");
            DumpDir();   // Display flash card files and directories
            iprintf("End of listing\r\n\n");
            break;
        }
        case 'E':
        {
            iprintf("Displaying TestFile.txt:\r\n");
            DisplayTextFile((char *)"TestFile.txt");
            break;
        }
        case 'F':
        {
            iprintf("Proceed with format? ('Y' to execute)");
            char c = getchar();
            if ((c == 'y') || (c == 'Y'))
            {
                iprintf("\r\n");
                FormatExtFlash();
                iprintf("Format complete\r\n");
            }
            else
            {
                iprintf("\r\nFormat command aborted\r\n");
            }
            iprintf("\r\n");
            break;
        }
        case 'G':
        {
            iprintf("Running fgets_test()\r\n");
            fgets_test((char *)"TestFile.txt");
            break;
        }
        case 'H':
        {
            iprintf("Running fputs_test()\r\n");
            fputs_test((char *)"TestFile.txt");
            break;
        }
        case 'P':
        {
            iprintf("Running fprinf_test()\r\n");
            fprintf_test();
            break;
        }
        case 'S':
        {
            DisplayEffsSpaceStats();   // Display file space usage
            iprintf("\r\n");
            break;
        }
        case 'T':
        {
            DisplaySystemTime();   // Display file space usage
            iprintf("\r\n");
            break;
        }
        default:
        {
            DisplayMenu();
            break;
        }
    }
}

/**
 * @brief Main entry point of the example.
 */
void UserMain(void *pd)
{
    // Note that you almost never want to put anything before you call init()
    // but in this case, we need to call f_enterFS() before the HTTP task starts, or we won't
    // be able to do it.
    OSChangePrio(HTTP_PRIO);
    f_enterFS();
    OSChangePrio(MAIN_PRIO);   // change back to this task's priority

    init();                                       // Initialize network stack
    StartHttp();                                  // Start web server, default port 80
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

    iprintf("\r\n===== Starting %s Program =====\r\n", AppName);

    /* The following call to f_enterFS() must be called in every task that accesses
       the file system.  This must only be called once in each task and must be done before
       any other file system functions are used.  Up to 10 tasks can be assigned to use
       the file system. Any task may also be removed from accessing the file system with a
       call to the function f_releaseFS(). */
    f_enterFS();

    // We now must also enter the file system for the FTP and HTTP tasks
    OSChangePrio(FTP_PRIO);
    f_enterFS();
    OSChangePrio(MAIN_PRIO);   // change back to this task's priority

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

    // This is where the read/write file system calls are made
    ReadWriteTest();

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

    DisplayMenu();
    while (1)
    {
        char buffer[255];
        buffer[0] = '\0';

        if (charavail())
        {
            gets(buffer);
            iprintf("\r\n");
            ProcessCommand(buffer);
            buffer[0] = '\0';
        }
    }
}
