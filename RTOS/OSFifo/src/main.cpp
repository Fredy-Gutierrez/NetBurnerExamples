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
#include <stddef.h>

#define NUM_POSTS (5)   // Number of structures to post

const char *AppName = "OSFifo Example";

typedef struct   // Pointers to this structure will be passed in the FIFO
{
    void *pUsedByFifo;   // Do not modify this value, it must be first in the structure.
                         // It is used for the linked list
    int x;               // Just a variable
    BOOL free;           // free = TRUE if structure is free to be used.
} MyStructure;

uint32_t FifoPostTaskStack[USER_TASK_STK_SIZE];   // Allocate task stack space
OS_FIFO MyFIFO;                                   // Declare a FIFO object
MyStructure StructArray[NUM_POSTS];               // Declare a small array of our structure

/*-------------------------------------------------------------------
 Init FIFO members
 ------------------------------------------------------------------*/
void InitFifoMembers()
{
    for (int i = 0; i < NUM_POSTS; i++)
    {
        StructArray[i].x = 0;
        StructArray[i].free = TRUE;
    }
}

/*-------------------------------------------------------------------
 Find a free FIFO structure in the array.
 Rather than use dynamic memory allocation, this example uses a
 declared array of structures with a structure element that indicates
 if a particular structure is being used. This function will return
 the index of the first free structure array member, or -1 if all
 members are being used.
 ------------------------------------------------------------------*/
int FindFreeFifoStruct()
{
    int index = -1;
    int i = 0;

    do
    {
        if (StructArray[i].free)
            index = i;
        else
            i++;
    } while ((i < NUM_POSTS) && (index == -1));

    return index;
}

/*-------------------------------------------------------------------
 FIFO Post Task
 ------------------------------------------------------------------*/
void FifoPostTask(void *pdata)
{
    while (1)
    {
        iprintf("\r\n     FIFOPostTask():\r\n");
        for (int count = 0; count < NUM_POSTS; count++)
        {
            int i;
            while ((i = FindFreeFifoStruct()) < 0)
            {
                iprintf("Waiting for free FIFO structure\r\n");
                OSTimeDly(TICKS_PER_SECOND / 2);
            }

            StructArray[i].x = count;
            StructArray[i].free = FALSE;
            // Put a message in the Fifo
            MyFIFO.Post((OS_FIFO_EL *)&StructArray[i]);
            iprintf("     Posted FIFO StructArray[%d].x = %d\r\n", i, count);
        }
        // This delay simulates a resource taking time to pend.
        iprintf("     Delaying 9 seconds before next FIFO post\r\n\r\n");
        OSTimeDly(TICKS_PER_SECOND * 9);
    }
}

/*-------------------------------------------------------------------
 UserMain
 ------------------------------------------------------------------*/
void UserMain(void *pd)
{
    uint32_t FifoCnt = 0;

    init();
    MyFIFO.Init();
    InitFifoMembers();

    // Create a task to post to the FIFO
    if (OSTaskCreatewName(FifoPostTask, NULL, (void *)&FifoPostTaskStack[USER_TASK_STK_SIZE], (void *)FifoPostTaskStack, MAIN_PRIO - 1,
                          "FifoPostTask") != OS_NO_ERR)
    { iprintf("*** Error creating FIFO task\r\n"); }

    while (1)
    {
        iprintf("\r\n>>> Calling OSFifoPend()...\r\n");
        MyStructure *pData = (MyStructure *)MyFIFO.Pend(TICKS_PER_SECOND * 5);   // Pend on FIFO

        if (pData == NULL)
        {
            // Because of the OSTimeDly() in the post task, we will timeout once per sequence on purpose
            iprintf("    Timeout in OSFifoPend(), waiting for next FIFO Post\r\n");
        }
        else
        {
            iprintf("    FIFO Read #%lu, pData->x = %d\r\n", FifoCnt++, pData->x);
            pData->free = TRUE;
        }
    }
}
