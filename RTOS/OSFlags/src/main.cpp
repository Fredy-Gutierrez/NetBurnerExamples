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

// NB Constants
#include <constants.h>

// NB Libs
#include <basictypes.h>
#include <init.h>
#include <nbrtos.h>
#include <utils.h>

// OSFlag ( gOsFlag ) assigned bits
#define TASK_FLAG_1 ((uint32_t)0x00000001)
#define TASK_FLAG_2 ((uint32_t)0x00000002)
#define TASK_FLAG_3 ((uint32_t)0x00000004)

const char *AppName = "OSFlags Example";

OS_FLAGS gOsFlag;   // global OS Flag object

void OsFlagTask1(void *notUsed)
{
    while (TRUE)
    {
        OSTimeDly((2 * TICKS_PER_SECOND));
        gOsFlag.Set(TASK_FLAG_1);
    }
}

void OsFlagTask2(void *notUsed)
{
    while (TRUE)
    {
        OSTimeDly((4 * TICKS_PER_SECOND));
        gOsFlag.Set(TASK_FLAG_2);
    }
}

void OsFlagTask3(void *notUsed)
{
    while (TRUE)
    {
        OSTimeDly((6 * TICKS_PER_SECOND));
        gOsFlag.Set(TASK_FLAG_3);
    }
}

/*-------------------------------------------------------------------
 * UserMain
 * ----------------------------------------------------------------*/
void UserMain(void *notUsed)
{
    uint32_t gOsFlagState = 0;

    init();
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

    gOsFlag.Init();

    // Create tasks
    OSSimpleTaskCreatewName(OsFlagTask1, (MAIN_PRIO + 1), "OsFlagTask1");
    OSSimpleTaskCreatewName(OsFlagTask2, (MAIN_PRIO + 2), "OsFlagTask2");
    OSSimpleTaskCreatewName(OsFlagTask3, (MAIN_PRIO + 3), "OsFlagTask3");

    while (TRUE)
    {
        /* There are two pending functions we can use:
         *   OSFlagPendAny(), returns if one or more of the flags are set
         *   OSFlagPendAll(), returns only if all flags are set
         *
         * This example will use the Any version, which is useful for a task that is reporting
         * on multiple events. It will return if any of the flags in TASK1, TASK2 or TASK3 are set
         */

        gOsFlag.PendAny((TASK_FLAG_1 | TASK_FLAG_2 | TASK_FLAG_3), 0);   // Pend on any flag being set

        // Only one of the OSFlagPend functions can be used at a time
        // OSFlagPendAll( &gOsFlag, ( TASK_FLAG_1 | TASK_FLAG_2 | TASK_FLAG_3 ), 0 );    // Pend on all flags being set
        iprintf("OSFlagPendAny() detected the following flag(s) are set: ");

        // Get the state of the flags so we can determine which ones are set
        gOsFlagState = gOsFlag.State();

        // Test each flag (bit position)
        if ((gOsFlagState & TASK_FLAG_1) == TASK_FLAG_1) iprintf("TASK1 ");

        if ((gOsFlagState & TASK_FLAG_2) == TASK_FLAG_2) iprintf("TASK2 ");

        if ((gOsFlagState & TASK_FLAG_3) == TASK_FLAG_3) iprintf("TASK3 ");

        iprintf("\r\n");

        // Clear the Flag mask. You might need to signal the task that set the flag in some applications
        gOsFlag.Clear(gOsFlagState);
    }
}
