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
#include <string.h>
#include <utils.h>

const char *AppName = "MultiTask Input example";

//----- global vars -----
OS_MBOX UserInputMbox;
BOOL bExitUserInputTask;

/*-------------------------------------------------------------------
 UserInputTask
 Once created, this task will loop unitl bExitUserInputTask = FALSE.
 The fgets will block until the user enters a string in mttty. This
 task will have a higher priority than UserMain.
 ------------------------------------------------------------------*/
void UserInputTask(void *pdata)
{
    static char strInput[1024];

    while (!bExitUserInputTask)
    {
        fgets(strInput, 80, stdin);   // block until user enters a string

        int len = strlen(strInput);
        iprintf("Received %d bytes\r\n", len);

        // Remove any trailing \r or \n
        if (len >= 2)
        {
            for (int i = 0; i < 2; i++)
            {
                if ((strInput[len - i] == '\r') || (strInput[len - i] == '\n')) strInput[len - i] = '\0';
            }
        }

        // Post pointer to input string in mailbox
        UserInputMbox.Post((void *)strInput);
    }

    // Not much to do in this example once the task exits
    iprintf("*** UserInputTask() terminated. Please reset program ***\r\n");
}

/*-------------------------------------------------------------------
 UserMain
 ------------------------------------------------------------------*/
void UserMain(void *pd)
{
    init();

    putleds(0);

    // Initialize the mailbox
    UserInputMbox.Init();

    bExitUserInputTask = FALSE;

    OSSimpleTaskCreatewName(UserInputTask, MAIN_PRIO - 1, "User Input");

    uint8_t i = 0;
    iprintf("Enter String: ");
    while (1)
    {
        uint8_t err;

        /* We can use the Wait or No Wait version of OSMboxPend.
         * In this example we will use NoWait so that we can
         * show how multiple tasks can operate and count on the
         * LEDs while we are waiting for user input. The Wait
         * version of the function is:
         *   void *pmsg = OSMboxPend( &UserInputMbox, 0, &err );
         */
        void *pmsg = UserInputMbox.PendNoWait(err);
        if (pmsg != NULL)
        {
            iprintf("\r\nUserMain Received: \"%s\"\r\n\r\n", (const char *)pmsg);
            iprintf("Enter String: ");

            // If user input was "exit", then send exit signal to UserInputTask()
            if (strcmp((const char *)pmsg, "exit") == 0) bExitUserInputTask = TRUE;
        }
        else
        {
            // Count on 8 LEDs on dev board while wainting for input
            putleds(i++);
            OSTimeDly(1);
        }
    }
}
