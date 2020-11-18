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

#include <predef.h>
#include <constants.h>
#include <nbrtos.h>
#include <init.h>
#include <stdio.h>
#include <string.h>
#include <iosys.h>
#include <nbstring.h>
#include <IntervalTimer.h>

const char *AppName = "Interval Timer Example";

#define FLAG_BIT_0      (0x1)
#define ALL_FLAG_BITS   (0xFFFFFFFF)

OS_SEM timerSem;
OS_FLAGS timerFlag;
uint32_t interruptCounter;

/*-----------------------------------------------------------------------------
 * This callback function is called by the timer's interrupt handler so treat
 * this callback like an ISR. Keep it short.
 *----------------------------------------------------------------------------*/
void timerInterruptCallbackFunc()
{
    interruptCounter++;
}

/*-----------------------------------------------------------------------------
 * Process menu command
 *----------------------------------------------------------------------------*/
void processCommand( char command)
{
    char buf[16];

    iprintf("\r\n");
    switch( command )
    {
        case '1':   // Using the IntervalTimer to post to a semaphore
        {
            uint32_t semPostCount = 0;
            timerSem.Init();

            iprintf("Enter the number of semaphore posts per second: ");
            fgets( buf , sizeof(buf), stdin);
            NBString frequencyBuf(buf);
            int freq = frequencyBuf.stoi();
            iprintf("\r\nUsing %d posts per second\r\n", freq);

            iprintf("Press any key to stop\r\n");

            int timerNumer = IntervalOSSem(&timerSem, freq);
            if(timerNumer < 0)
            {
                iprintf("Error %d\r\n", timerNumer);
                break;
            }

            while(!charavail())
            {
                timerSem.Pend();
                semPostCount++;
            }
            getchar();
            IntervalStop(timerNumer);

            iprintf("%d semaphore posts occurred\r\n", semPostCount);
            break;
        }


        case '2':   // Using the IntervalTimer to post to a flag
        {
            iprintf("Enter the number of flag posts per second: ");
            fgets( buf , sizeof(buf), stdin);
            NBString frequencyBuf(buf);
            int freq = frequencyBuf.stoi();
            iprintf("\r\nUsing %d posts per second\r\n", freq);

            iprintf("Press any key to stop\r\n");
            uint32_t flagPostCount = 0;
            timerFlag.Init();
            int timerNumer = IntervalOSFlag(&timerFlag, FLAG_BIT_0, freq);
            if(timerNumer < 0)
            {
                iprintf("Error %d\r\n", timerNumer);
                break;
            }

            while(!charavail())
            {
                timerFlag.PendAny(ALL_FLAG_BITS, 0);   // pend forever for any flag to be set
                timerFlag.Clear( ALL_FLAG_BITS );      // clear all flag bits
                flagPostCount++;
            }
            getchar();
            IntervalStop(timerNumer);

            iprintf("%d flag posts occurred\r\n", flagPostCount);
            break;
        }

        case '3':   // Using the IntervalTimer to trigger an interrupt
        {
            iprintf("Enter the number of interrupts per second: ");
            fgets( buf , sizeof(buf), stdin);
            NBString frequencyBuf(buf);
            int freq = frequencyBuf.stoi();
            iprintf("\r\nUsing %d interrupts per second\r\n", freq);

            interruptCounter = 0;
            int timerNumer = IntervalInterruptCallback(&timerInterruptCallbackFunc, freq);
            if(timerNumer < 0)
            {
                iprintf("Error %d\r\n", timerNumer);
                break;
            }

            iprintf("Press any key to stop\r\n");
            getchar();
            IntervalStop(timerNumer);

            iprintf("%d interrupts occurred\r\n", interruptCounter);
            break;
        }

        default:
            iprintf("Invalid command\r\n");
            break;
    }
}

/*-----------------------------------------------------------------------------
 * Display user menu
 *----------------------------------------------------------------------------*/
void displayMenu()
{
    iprintf("\r\nUse Interval Timer to:\r\n");
    iprintf( "1 Post to a semaphore\r\n" );
    iprintf( "2 Post to a flag\r\n" );
    iprintf( "3 Trigger an interrupt routine\r\n" );
}

void UserMain( void *pd )
{
    init();
    WaitForActiveNetwork();

    iprintf("Application Started\r\n");

    while ( 1 )
    {
        displayMenu();
        char c = getchar();
        processCommand(c);
    }
}
