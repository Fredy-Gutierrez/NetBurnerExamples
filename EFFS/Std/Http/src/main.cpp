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
#include <netinterface.h>
#include <startnet.h>
#include <tcp.h>

#include "FileSystemUtils.h"
#include "effs_time.h"
#include "http_f.h"

#define FTP_PRIO (MAIN_PRIO - 2)

extern void fs_main();
extern void OnChipFlash_GetFlashName(char *pName);

const char *AppName = "EFFS-STD-HTTP";

/**
 *  DoAddDirRefLink
 */
void DoAddDirRefLink(int sock, const char *url)
{
    char wb[100];
    siprintf(wb, "http:\\\\%I\\DIR", GetSocketLocalAddr(sock));
    writestring(sock, "<A HREF=\"");
    writestring(sock, wb);
    writestring(sock, "\">");
    writesafestring(sock, wb);
    writestring(sock, "</A>");
}

/**
 * @brief Display command menu.
 */
void DisplayMenu(void)
{
    iprintf("\r\n\n--- Main Menu ---\r\n");
    iprintf("[1] - Read/Write Test\r\n");
    iprintf("[2] - Display Directory\r\n");
    iprintf("[3] - Display TestFile.txt\r\n");
    iprintf("[4] - Format File System (Warning: All Data Will Be Lost)\r\n");
    iprintf("[5] - Display Space Usage\r\n");
    iprintf("[6] - Display System Time\r\n");
    iprintf("[?] - Display Menu\r\n");
    iprintf("> ");
}

/**
 * @brief Handle commands with  a trivially simple command dispatcher.
 */
void ProcessCommand(char c)
{
    switch (c)
    {
        case '1':
        {
            ReadWriteTest();
            break;
        }
        case '2':
        {
            iprintf("Directory Contents:\r\n");
            DumpDir();
            iprintf("End of listing\r\n\n");
            break;
        }
        case '3':
        {
            iprintf("Displaying TestFile.txt:\r\n");
            DisplayTextFile((char *)"TestFile.txt");
            break;
        }
        case '4':
        {
            iprintf("Proceed with format? (Y/N) ");
            char c = getchar();
            if ((c == 'Y') || (c == 'y'))
            {
                iprintf("\r\n");
                FormatEffsStdFlash();
                iprintf("Format complete\r\n");
            }
            else
            {
                iprintf("\r\nFormat command aborted\r\n");
            }
            iprintf("\r\n");
            break;
        }
        case '5':
        {
            DisplayEffsSpaceStats();   // Display file space usage
            iprintf("\r\n");
            break;
        }
        case '6':
        {
            DisplaySystemTime();   // Display current system date and time
            iprintf("\r\n");
            break;
        }
        default:   // ?
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
    init();                                       // Initialize network stack
    StartHttp();                                  // Start web server, default port 80
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

    iprintf("\r\n===== Starting %s Program =====\r\n", AppName);

    char FlashChipName[80];
    OnChipFlash_GetFlashName(FlashChipName);

    iprintf("Application started\r\n");
    fs_main();   // Initialize the flash file system

    /*
     * Try setting the time via a NTP server.  If that fails, then set it
     * manually.
     */
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
        SetTimeManual(5, 14, 3, 2006, 11, 35, 0);
    }

    DisplaySystemTime();
    iprintf("\r\n");

    /*
     * Start FTP server with task priority higher than than the main task.
     */
    InterfaceBlock *pifb = GetInterfaceBlock();
    int status = FTPDStart(21, FTP_PRIO);
    if (status == FTPD_OK)
    {
        iprintf("Started FTP server\r\n");
        if (pifb != nullptr) { iprintf("URL \"ftp://%hI\r\n", pifb->ip4.cur_addr.i4); }
    }
    else
    {
        iprintf("*** Error:  %d. Could not start FTP server\r\n", status);
    }

    DisplayMenu();

    while (1)
    {
        char c = getchar();
        ProcessCommand(c);
    }
}
