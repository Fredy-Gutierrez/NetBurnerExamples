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

#define MAIN_DELAY (2)
#define TASK1_DELAY (4)
#define TASK2_DELAY (6)

#define TASK1_PRIO (MAIN_PRIO - 1)
#define TASK2_PRIO (MAIN_PRIO - 2)

#if (defined(USE_MMC))
const char *AppName = "EFFS MultiTask MMC/SD";
#elif (defined(USE_CFC))
const char *AppName = "EFFS MultiTask CFC";
#endif

/**
 *  Task1
 *
 *  Loops forever, running a file test at a specified interval.
 */
void Task1(void *pdata)
{
    f_enterFS();                    // Create working dir for active task, must be done before any other fs calls
    f_chdrive(EXT_FLASH_DRV_NUM);   // Select drive

    while (1)
    {
        ReadWriteTest("Task1.txt");
        OSTimeDly(TICKS_PER_SECOND * TASK1_DELAY);
        DumpDir();   // Display flash card files and directories
    }
}

/**
 *  Task2
 *
 *  Loops forever, running a file test at a specified interval.
 */
void Task2(void *pdata)
{
    f_enterFS();                    // Create working dir for active task, must be done before any other fs calls
    f_chdrive(EXT_FLASH_DRV_NUM);   // Select drive
    f_chdir("/");                   // optional

    while (1)
    {
        ReadWriteTest("Task2.txt");
        OSTimeDly(TICKS_PER_SECOND * TASK2_DELAY);
        DumpDir();   // Display flash card files and directories
    }
}

/**
 *  UserMain
 *
 *  Main entry point for the example
 */
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

    OSSimpleTaskCreatewName(Task1, TASK1_PRIO, "Task 1");
    OSSimpleTaskCreatewName(Task2, TASK2_PRIO, "Task 2");

    f_chdir("/");   // Optional

    while (1)
    {
        ReadWriteTest("Main.txt");
        OSTimeDly(TICKS_PER_SECOND * MAIN_DELAY);
        DumpDir();   // Display flash card files and directories
    }
}
