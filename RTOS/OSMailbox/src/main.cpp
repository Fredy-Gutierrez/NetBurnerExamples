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

// NB Definitions
#include <predef.h>

// NB Constants
#include <constants.h>

// NB Libs
#include <init.h>
#include <nbrtos.h>

const char *AppName = "Mailbox Example";

//----- Global Variables
uint32_t MyTaskStack[USER_TASK_STK_SIZE];
OS_MBOX MyMailbox;

/*-------------------------------------------------------------------
 MyTask
 -------------------------------------------------------------------*/
void MyTask(void *pdata)
{
    uint8_t myMsg = 1;

    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND * 3);
        iprintf("      MyTask: Posted to Mailbox\r\n");

        MyMailbox.Post((void *)&myMsg);
    }
}

/*-------------------------------------------------------------------
 UserMain
 -------------------------------------------------------------------*/
extern "C" void UserMain(void *pd)
{
    init();

    MyMailbox.Init();

    OSSimpleTaskCreatewName(MyTask, MAIN_PRIO + 1, "Mailbox Task");

    uint8_t *pMessage;
    uint8_t error = 0;

    while (1)
    {
        iprintf(">>> UserMain: Pending on Mailbox from MyTask\r\n");

        pMessage = (uint8_t *)MyMailbox.Pend(0, error);   // Pend forever

        if (pMessage != NULL)   // Check if pointer is valid before dereferencing
        {
            iprintf("     Message: %d \r\n", *pMessage);
            (*pMessage)++;   // increment the value of myMsg variable in MyTask
        }
        else
        {
            iprintf("       Error: %d \r\n", error);
        }

        iprintf("<<< UserMain: Mailbox message was posted from MyTask!\r\n\r\n");
    }
}
