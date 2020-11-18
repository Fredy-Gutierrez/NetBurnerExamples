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

#include <basictypes.h>
#include <constants.h>
#include <init.h>
#include <nbrtos.h>
#include <stdio.h>

#define NUM_POSTS (3)   // Number of queue posts

const char *AppName = "OSQueue Example";

uint32_t QueueTaskStack[USER_TASK_STK_SIZE];

#define MY_QUEUE_SIZE (20)          // number of void pointers in the queue
OS_Q myQ;                           // the queue
void *myQueueData[MY_QUEUE_SIZE];   // pointer to an array of void pointers

/*-------------------------------------------------------------------
 Q Post Task
 ------------------------------------------------------------------*/
void QPostTask(void *pdata)
{
    static uint32_t count;

    while (1)
    {
        iprintf("QPostTask posting %d entries to queue\r\n", NUM_POSTS);
        for (int i = 0; i < NUM_POSTS; i++)
            myQ.Post((void *)count++);

        iprintf("Post task delaying 10 seconds\r\n");
        OSTimeDly(TICKS_PER_SECOND * 10);
    }
}

/*-------------------------------------------------------------------
 UserMain
 ------------------------------------------------------------------*/
void UserMain(void *pd)
{
    init();
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address
    myQ.Init(myQueueData, MY_QUEUE_SIZE);         // Initialize queue

    // Create task to post to queue
    if (OSTaskCreatewName(QPostTask, NULL, (void *)&QueueTaskStack[USER_TASK_STK_SIZE], (void *)QueueTaskStack, MAIN_PRIO - 1,
                          "OSQueueTask") != OS_NO_ERR)
    { iprintf("*** Error creating QUEUE task\r\n"); }

    OSTimeDly(TICKS_PER_SECOND);
    while (1)
    {
        uint8_t err;

        iprintf("\r\n>>> UserMain Calling OSQPend()...\r\n");
        void *p = myQ.Pend(0, err);   // timeout of 0 waits forever
        iprintf("Got Q = %p with err = 0x%02X\r\n", p, err);
    }
}
