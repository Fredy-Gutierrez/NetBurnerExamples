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
#include <system.h>
#include <iosys.h>
#include <fdprintf.h>
#include <serial.h>
#include <pins.h>

const char *AppName = "Serial Empty Callback";

#define DEMO_PIN P2[26]

// This function is a standin for whatever action you want to trigger off of the
//  completion of transmission of data on the serial port. Most likely, you are
//  wanting to turn off a transciever for RS485, so we're turning off a GPIO in
//  this example
void DoActionTransmissionCompleteAction()
{
    DEMO_PIN = 0;
}

// While this should not be an issue for small writes, for long writes that cannot
//  be fully buffered or take place as multiple smaller writes (i.e. printf),
//  we need to track whether all bytes have been written to the serial driver,
//  and only then perform our "definitely done transmitting" functions,
//  because there may be task switches or other interrupts that cause the driver
//  to have sent all bytes it was given, but not have sent all bytes we wish to
//  wait on.
OS_SEM TxDoneSem;
bool allDataQueued;
uint32_t bytesSent;

// Cycling over iterations of the loop, we will delay before configuring the
//  callback to post to our semaphore, simulating a preemption during a
//  multiwrite transaction to the serial port that was split and caused the
//  callback to fire before we were actually ready for it to do so.
enum Preemption_t {
    PREEMPT_NONE = 0,
    PREEMPT_MIDWRITE, // Preempt only between the writes
    PREEMPT_ALL_QUEUED, // Preempt between the writes *and* before telling the callback that all data is queued
    PREEMPT_count
};

Preemption_t SimulateTaskPreemption;

void SerialEmptyCB(int portnum, uint32_t bytesSinceLast)
{
    bytesSent += bytesSinceLast;
    if (allDataQueued)
    {
        TxDoneSem.Post();
    }
}

/**
 *  UserMain
 *
 *  Main entry point for the example
 */

void UserMain(void *pd)
{
    DEMO_PIN.function(PinIO::PIN_FN_OUT);
    init();                                       // Initialize network stack

    int fd_uart2 = SimpleOpenSerial(4, 115200);
    iprintf("Application: %s\r\nNNDK Revision: %s\r\n", AppName, GetReleaseTag());
    SimulateTaskPreemption = PREEMPT_NONE;
    while (1)
    {
        RegisterTxEmptyCallback(fd_uart2, NULL); // flushes the driver's bytesSinceLast count

        // Reset sending state variables
        allDataQueued = false;
        TxDoneSem.Init();
        bytesSent = 0;

        uint32_t bytesStarted = 0;
        RegisterTxEmptyCallback(fd_uart2, SerialEmptyCB); // Register our callback
        iprintf("Sending on Serial\n");

        // Set the demo pin High before starting the write to the serial port
        //  driver
        DEMO_PIN = 1;

        // The following writes are the moral equivalent to:
        //  bytesStarted = fdiprintf(fd_uart2, "This is a %s.\n", "test");
        //
        // We are splitting the printf into two separate writes to allow us to
        //  trigger a preemption between the internal multiple writes of printf,
        //  and demonstrate a robust method for graceful recovery
        bytesStarted = writestring(fd_uart2, "This is a ");
        if (SimulateTaskPreemption > PREEMPT_NONE)
        {
            OSTimeDly(2);
        }
        if (bytesStarted > 0)
        {
            uint32_t ret = writestring(fd_uart2, "test.\n");
            if (ret > 0)
            {
                bytesStarted += ret;
            }
            else
            {
                bytesStarted = ret;
            }
        }

        if (SimulateTaskPreemption >= PREEMPT_ALL_QUEUED)
        {
            OSTimeDly(2);
        }
        USER_ENTER_CRITICAL();
        if ((bytesStarted > 0) && (bytesStarted > bytesSent))
        {
            allDataQueued = true;
            USER_EXIT_CRITICAL();
            iprintf("bytesStarted: %lu - bytesSent: %lu\n", bytesStarted, bytesSent);
            TxDoneSem.Pend();
        }
        else
        {
            // We took longer to get here than it did to send the data,
            //  so no need to wait on the completion semaphore
            USER_EXIT_CRITICAL();
        }
        // Our action in this example will drive the demo pin low,
        //  simulating disabling a transceiver
        DoActionTransmissionCompleteAction();

        iprintf("Action Complete\n");

        OSTimeDly(TICKS_PER_SECOND);

        // Configure what type of preemption the next round will be
        SimulateTaskPreemption = (Preemption_t)((SimulateTaskPreemption+1) % PREEMPT_count);
    }
}

