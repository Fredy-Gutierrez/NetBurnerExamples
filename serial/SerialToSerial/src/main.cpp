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
#include <serial.h>
#include <startnet.h>
#include <stdio.h>

const char *AppName = "Serial To Serial Example";

/**
 *  UserMain
 *
 *  Main entry point of example
 */
void UserMain(void *pd)
{
    init();
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);

    /*
     * The serial port drivers are initialized in polled mode by the monitor.
     * To enable them in interrupt driven mode we need to close and reopen
     * them with OpenSerial()
     */
    SerialClose(0);
    SerialClose(1);

    // Open the serial ports....
    int fd0 = OpenSerial(0, 115200, 1, 8, eParityNone);
    int fd1 = OpenSerial(1, 115200, 1, 8, eParityNone);

    // Now write something out both ports
    writestring(fd1, "Test1");
    writestring(fd0, "Test0");

    while (1)
    {
        // Set up a file set so we can select on the serial ports...
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(fd1, &read_fds);
        FD_SET(fd0, &read_fds);
        if (select(FD_SETSIZE, &read_fds, (fd_set *)0, (fd_set *)0, TICKS_PER_SECOND * 5))
        {
            if (FD_ISSET(fd1, &read_fds))
            {
#ifdef MOD5441X
                putleds(0x22);
#endif
                char buffer[40];
                int n = read(fd1, buffer, 40);
                write(fd0, buffer, n);
            }

            if (FD_ISSET(fd0, &read_fds))
            {
#ifdef MOD5441X
                putleds(0x22);
#endif
                char buffer[40];
                int n = read(fd0, buffer, 40);
                write(fd1, buffer, n);
            }
        }
        else
        {
            // WE timed out... nothing to send
        }
    }
}
