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

#include <init.h>
#include <nbrtos.h>
#include <stdlib.h>
#include <system.h>
#include <utils.h>

const char *AppName = "OSTaskCreate Example";

// Allocate stack space
uint32_t TaskAllParamsStack[USER_TASK_STK_SIZE];

/*-----------------------------------------------------------------------------
 * Task created with OSTaskCreatewName() using all function parameters.
 * The void *pd parameter can be cast to any type of data you wish to send to
 * the task, or NULL if not used.
 *----------------------------------------------------------------------------*/
void TaskAllParams(void *pd)
{
    uint32_t loopCount = 0;
    uint32_t delayTime = (uint32_t)pd;

    iprintf("TaskAllParams delay time set to: %ld seconds\r\n", delayTime);

    while (1)
    {
        iprintf("TaskAllParams, %ld\r\n", loopCount);
        loopCount++;
        OSTimeDly(TICKS_PER_SECOND * delayTime);
    }
}

/*-----------------------------------------------------------------------------
 * Task created with OSSimpleTaskCreatewName(). The primary difference from the
 * full version is that it allocates the task stack for you.
 *----------------------------------------------------------------------------*/
void TaskSimple(void *pd)
{
    uint32_t loopCount = 0;
    uint32_t delayTime = 6;

    iprintf("TaskSimple delay time set to: %ld seconds\r\n", delayTime);

    while (1)
    {
        iprintf("TaskSimple, %ld\r\n", loopCount);
        loopCount++;
        OSTimeDly(TICKS_PER_SECOND * delayTime);
    }
}

/*-----------------------------------------------------------------------------
 *  UserMain
 *----------------------------------------------------------------------------*/
void UserMain(void *pd)
{
    uint32_t delayTime = 3;
    int returnCode;

    init();                                       // Initialize network stack
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

    iprintf("Creating TaskAllParams....");
    returnCode = OSTaskCreatewName(TaskAllParams,                             // Task function name
                                   (void *)delayTime,                         // Optional parameter, NULL if not used
                                   &TaskAllParamsStack[USER_TASK_STK_SIZE],   // Top of task stack
                                   TaskAllParamsStack,                        // Botom of task stack
                                   MAIN_PRIO - 1,                             // Priority
                                   "TaskAllParams");                          // Name
    if (returnCode == OS_NO_ERR)
        iprintf("Task creation successful\r\n");
    else
        iprintf("*** Error: status = %d\r\n", returnCode);

    iprintf("Creating TaskSimple\r\n");
    OSSimpleTaskCreatewName(TaskSimple, MAIN_PRIO - 2, "TaskSimple");

    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND * 1);
    }
}
