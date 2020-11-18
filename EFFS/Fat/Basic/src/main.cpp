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
#include <init.h>

#include "FileSystemUtils.h"

#if (defined(USE_MMC) && defined(MOD5441X))
#define MULTI_MMC TRUE   // For modules with onboard flash sockets, even if you are using external flash cards
#include <effs_fat/multi_drive_mmc_mcf.h>
#elif (defined(USE_MMC))
#include <effs_fat/mmc_mcf.h>
#elif (defined(USE_CFC))
#include <effs_fat/cfc_mcf.h>
#endif

#if (defined(USE_MMC))
const char *AppName = "EFFS BASIC MMC/SD";
#elif (defined(USE_CFC))
const char *AppName = "EFFS BASIC CFC";
#endif

/*-------------------------------------------------------------------
  UserMain()
 -------------------------------------------------------------------*/
void UserMain(void *pd)
{
    init();                                       // Initialize network stack
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address
    iprintf("\r\n===== Starting %s Program =====\r\n", AppName);

    /* The following call to f_enterFS() must be called in every task that accesses
       the file system.  This must only be called once in each task and must be done before
       any other file system functions are used.  Up to 10 tasks can be assigned to use
       the file system. Any task may also be removed from accessing the file system with a
       call to the function f_releaseFS(). */
    f_enterFS();

    InitExtFlash();   // Initialize the CFC or SD/MMC external flash drive

    DisplayEffsSpaceStats();   // Display file space usage
    DumpDir();                 // Display flash card files and directories

    // This is where the read/write file system calls are made
    ReadWriteTest();

    /* Unmount file system
       This function is used to delete an existing volume. The link between the
       file system and the driver will be broken i.e. an xxx_delfunc call will
       be made to the driver and afterwards the user_ptr will be cleared. Any
       open files on the media will be marked as closed so that subsequent API
       accesses to a previously opened file handle will return with an error.
    */
    UnmountExtFlash();

    iprintf("Program complete. Reset the device to repeat\r\n");

    // Since main will no longer be accessing the file system we can release it
    f_releaseFS();
    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND);
    }
}
