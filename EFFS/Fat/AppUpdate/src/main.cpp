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

#include <hal.h>
#include <effs_fat/fat.h>
#include <init.h>
#include <startnet.h>

#include "FileSystemUtils.h"
#include "fileup.h"

#if (defined(USE_MMC) && defined(MOD5441X))
#define MULTI_MMC true   // For modules with onboard flash sockets, even if you are using external flash cards
#include <effs_fat/multi_drive_mmc_mcf.h>
#elif (defined(USE_MMC))
#include <effs_fat/mmc_mcf.h>
#elif (defined(USE_CFC))
#include <effs_fat/cfc_mcf.h>
#endif

// Application name displayed in IPSetup
const char *AppName = "EFFS Application Update";

// Specifies the file name to look for on the flash card. Note that
// by default the name must be in 8.3 format, otherwise you will
// get an file open error.
#define APPFILENAME "App_APP.S19"

char UserInput[256];

/**
 *  UserMain
 *
 *  Main entry point of the example
 */
void UserMain(void *pd)
{
    init();                                       // Initialize network stack;
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

    iprintf("\r\n===== Starting %s Program =====\r\n", AppName);

    /* The following call to f_enterFS() must be called in every task that accesses
       the file system.  This must only be called once in each task and must be done before
       any other file system functions are used.  Up to 10 tasks can be assigned to use
       the file system. Any task may also be removed from accessing the file system with a
       call to the function f_releaseFS().
     */
    f_enterFS();

    InitExtFlash();   // Initialize the CFC or SD/MMC external flash drive

    while (1)
    {
        iprintf("Long file names are not supported by default - only 8.3 format\r\n");
        iprintf("Enter file name to update from (Press enter for default file): ");
        fgets(UserInput, 256, stdin);
        F_FILE *fp = nullptr;
        if (strlen(UserInput) > 0) { fp = f_open(UserInput, "r"); }
        else
        {
            fp = f_open(APPFILENAME, "r");
        }

        if (fp)
        {
            iprintf("\r\nStarting\r\n");
            int rv = UpdateFromFat(fp, false);
            switch (rv)
            {
                case FAT_UPDATE_OK:
                {
                    iprintf("Update complete\r\n rebooting in 2 seconds\r\n");
                    f_delvolume(CFC_DRV_NUM);
                    OSTimeDly(2 * TICKS_PER_SECOND);
                    ForceReboot();
                    break;
                }
                case FAT_UPDATE_SAMEVERSION:
                {
                    iprintf("Update not needed already running latest version\r\n");
                    break;
                }
                case FAT_UPDATE_WRONG_PLATFORM:
                {
                    iprintf("Update not done, wrong Platform\r\n");
                    break;
                }
                case FAT_UPDATE_BAD_FORMAT:
                {
                    iprintf("Update failed file is in wrong format or corrupt\r\n");
                    break;
                }
                case FAT_UPDATE_NO_MEM:
                {
                    iprintf("Update failed, unable to malloc memory buffer\r\n");
                    break;
                }
                default:
                {
                    iprintf("Update had a file error \r\n");
                    DisplayEffsErrorCode(rv);
                    break;
                }
            }
        }
        else
        {
            iprintf("Failed to open file \r\n");
        }

        while (charavail())
        {
            getchar();
        }
        iprintf("\r\n");
    }
}
