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

/*-------------------------------------------------------------------
  This program illustrates basic file system operations for Flash
  memory cards:
   - Mounting a drive
   - Determining amount of used and free file space
   - Creating files
   - Writing data
   - Reading data
   - Unmounting a drive

 When the program executes it will display program status information
 through the debug serial port.
 -------------------------------------------------------------------*/
#include <init.h>
#include <nbrtos.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <effs_fat/fat.h>
#include <effs_fat/multi_drive_mmc_mcf.h>
#include "FileSystemUtils.h"

const char* AppName = "MOD5441x EFFS Flash Card Test";

/*-------------------------------------------------------------------
  UserMain()
 -------------------------------------------------------------------*/
void UserMain(void* pd)
{
    init();
    WaitForActiveNetwork();

    iprintf("\r\n===== Starting %s Program =====\r\n", AppName);

    /* The following call to f_enterFS() must be called in every task that accesses
     the file system.  This must only be called once in each task and must be done before
     any other file system functions are used.  Up to 10 tasks can be assigned to use
     the file system. Any task may also be removed from accessing the file system with a
     call to the function f_releaseFS(). */
    f_enterFS();

    iprintf("Testing the on-board Flash card\r\n");
    int drv = OpenOnBoardFlash();
    int rv = f_chdrive(drv);

    if (rv == F_NO_ERROR)
    {
        iprintf("drive change successful\r\n");
        F_FILE* fp = f_open("FileOnBoardSD.txt", "w+");
        if (fp)
        {
            const char * cp = "This file written to SD card on the Module";
            int n = f_write(cp, 1, strlen(cp), fp);
            iprintf("Wrote %d bytes: [%s]", n, cp);
            f_close(fp);
        }

        DumpDir();
    }
    else
    {
        iprintf("drive change failed: ");
        DisplayEffsErrorCode(rv);
    }

    UnmountFlash(drv);

    iprintf("Testing the external Flash card\r\n");
    drv = OpenOffBoardFlash();
    rv = f_chdrive(drv);

    if (rv == F_NO_ERROR)
    {
        F_FILE* fp = f_open("FileOffBoardSD.txt", "w+");
        if (fp)
        {
            const char * cp = "This file written to SD card off of the Module";
            int n = f_write(cp, 1, strlen(cp), fp);
            iprintf("Wrote %d bytes: [%s]", n, cp);
            f_close(fp);
        }

        iprintf("drive change successful\r\n");
        DumpDir();
    }
    else
    {
        iprintf("drive change failed: ");
        DisplayEffsErrorCode(rv);
    }

    UnmountFlash(drv);

    iprintf("Program complete. Reset to repeat\r\n");

    f_releaseFS();
    while (1)
    {
        OSTimeDly( TICKS_PER_SECOND);
    }
}

