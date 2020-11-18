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

/* NB Library Definitions */
#include <ftpd.h>
#include <https.h>
#include <init.h>

/* Example application support */
#include "FileSystemUtils.h"
#include "cardtype.h"
#include "ftp_f.h"

// Priority for the FTP task
#define FTP_PRIO (MAIN_PRIO - 2)

const char *AppName = "POP3 Mail Example";

/**
 * @brief The main entry point for the example.
 */
void UserMain(void *pd)
{
    // Initialize network stack
    init();

    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

    // Set up the file system. f_enterFS() must be called for each task priority level
    // that will have a task which utilizes the file system. Note that the task does not
    // have to be currently running while it is called.
    f_enterFS();
    OSChangePrio(HTTP_PRIO);
    f_enterFS();
    OSChangePrio(FTP_PRIO);
    f_enterFS();
    OSChangePrio(MAIN_PRIO);

    InitExtFlash();   // Initialize the CFC or SD/MMC external flash drive

    StartHttps();   // Start web server, default port 443

    DisplayEffsSpaceStats();   // Display file space usage
    DumpDir();                 // Display flash card files and directories

    // Start FTP server with task priority higher than UserMain()
    int status = FTPDStart(21, FTP_PRIO);
    if (status == FTPD_OK)
    {
        iprintf("Started FTP Server\r\n");
        if (F_LONGFILENAME == 1) { iprintf("Long file names are supported\r\n"); }
        else
        {
            iprintf("Long file names are not supported\r\n");
        }
    }
    else
    {
        iprintf("** Error: %d. Could not start FTP Server\r\n", status);
    }

    iprintf("\n\r\n** READY! **\n\r\n");
    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND);
    }
}
