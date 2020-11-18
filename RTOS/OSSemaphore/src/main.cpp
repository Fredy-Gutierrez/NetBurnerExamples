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

// NB Definition
#include <predef.h>

// NB Constants
#include <constants.h>

// NB Libs
#include <init.h>
#include <nbrtos.h>

const char *AppName = "Semaphore Example";

OS_SEM MySemaphore;   // Create semaphore

/*-------------------------------------------------------------------
 MyTask
 This task will post to a semaphore every 3 seconds
 -------------------------------------------------------------------*/
void MyTask(void *pdata)
{
    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND * 3);
        iprintf("      MyTask: Posted to Semaphore\r\n");
        MySemaphore.Post();
    }
}

/*-------------------------------------------------------------------
 UserMain
 -------------------------------------------------------------------*/
void UserMain(void *pd)
{
    init();
    MySemaphore.Init();
    OSSimpleTaskCreatewName(MyTask, MAIN_PRIO + 1, "Semaphore");

    while (1)
    {
        iprintf(">>> UserMain: Pending on Semaphore from MyTask\r\n");
        MySemaphore.Pend();
        iprintf("<<< UserMain: Semaphore was posted from MyTask!\r\n\r\n");
    }
}
