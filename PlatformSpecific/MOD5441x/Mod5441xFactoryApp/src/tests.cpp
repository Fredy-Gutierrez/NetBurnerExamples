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

/*------------------------------------------------------------------------------
 * MOD5441x Test Functions
 *----------------------------------------------------------------------------*/
#include <predef.h>
#include <stdio.h>
#include <stdlib.h>
#include <iosys.h>
#include <ctype.h>
#include <string.h>
#include <ethernet.h>
#include <tcp.h>
#include <udp.h>
#include <serial.h>
#include <effs_fat/multi_drive_mmc_mcf.h>
#include <sim5441x.h>

#include <effs_fat/fat.h>
#include <effs_fat/effs_utils.h>
#include "FileSystemUtils.h"

#define MMC_OFF_BOARD        (  0 )   // SD Card reader on MOD-DEV-70 and -100
#define MMC_ON_BOARD         (  1 )   // MicroSD Card reader on MOD54415


/**
 * Performs a simple test of either the on-board uSD or off-board SD card reader
 */
void doSingleMemCardTest(int mmc_drive)
{
    const char *driveLabel;

    if (mmc_drive == MMC_OFF_BOARD)
        driveLabel = "Off-board Card Reader";
    else if (mmc_drive == MMC_ON_BOARD)
        driveLabel = "On-board Card Reader";
    else
    {
        driveLabel = "Unidentified Card Reader";
        iprintf("ERROR  -- %s used\r\n", driveLabel);
        return;
    }

    // Card detection check
    if (get_cd(mmc_drive) != 0)
    {
        // Write protection check
        if (get_wp(mmc_drive) == 0)
        {
            int rv;

            if (mmc_drive == MMC_OFF_BOARD)
                rv = f_mountfat(mmc_drive, mmc_initfunc, F_MMC_DRIVE0);
            else // (mmc_drive == MMC_ON_BOARD)
                rv = f_mountfat(mmc_drive, mmc_initfunc, F_MMC_DRIVE1);

            if (rv == F_NO_ERROR)
            {
                rv = f_chdrive(mmc_drive);

                if (rv == F_NO_ERROR)
                {
                    F_FILE *fp = f_open( "TestFile.txt", "w+");

                    if (fp)
                    {
                        const char *cp = "Hello Memory Card!";
                        f_write(cp, 1, strlen(cp), fp);
                        f_close(fp);
                    }

                    rv = f_delvolume(mmc_drive);

                    if (rv == F_NO_ERROR)
                    {
                        iprintf("PASS   ++ %s test successful\r\n", driveLabel);
                    }
                    else
                    {
                        iprintf("ERROR  -- Failed to delete volume:  ");
                        DisplayEffsErrorCode(rv);
                    }
                }
                else
                {
                    iprintf("ERROR  -- Failed to change drive:  ");
                    DisplayEffsErrorCode(rv);
                }
            }
            else
            {
                iprintf("ERROR  -- Failed to mount drive:  ");
                DisplayEffsErrorCode(rv);
            }
        }
        else
        {
            iprintf("ERROR  -- %s has write-protection enabled\r\n", driveLabel);
        }
    }
    else
    {
        iprintf("ERROR  -- %s is empty\r\n", driveLabel);
    }
}


/**
 * Tests with the off-board and on-board card readers
 */
void doDualMemCardTest(void)
{
    iprintf("\r\nSTATUS -- Testing on-board uSD Card reader...\r\n");
    doSingleMemCardTest(MMC_ON_BOARD);

    iprintf("STATUS -- Testing off-board SD Card reader...\r\n");
    doSingleMemCardTest(MMC_OFF_BOARD);

    iprintf("STATUS -- End of dual memory card reader test\r\n\r\n");
}


/**
 * Used as part of a bring-up test in manufacturing.
 */
void doManfTest(void)
{
    char buffer[10];

    int fd1 = OpenSerial(1, 115200, 2, 8, eParityNone);
    buffer[0] = ' ';

    do
    {
        read(fd1, buffer, 1);
        write(fd1, buffer, 1);
    } while (buffer[0] != 'x');

    close(fd1);
}



