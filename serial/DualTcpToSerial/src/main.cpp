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
#include <iosys.h>
#include <serial.h>
#include <system.h>
#include <tcp.h>

const char *AppName = "DualTCP2Serial Example";

#define BAUDRATE_TO_USE (115200)
#define STOP_BITS (1)
#define DATA_BITS (8)
#define TCP_PORT0_TO_USE (2000)
#define TCP_PORT1_TO_USE (2001)
#define OVERRIDE_TIMEOUT (TICKS_PER_SECOND * 10)

/**
 *  sShowIP
 *
 *  Optional function to use if you want to create string of
 *  and IP address.
 */
void sShowIP(char *buf, IPADDR ia)
{
    puint8_t ipb = (puint8_t)&ia;
    siprintf(buf, "%d.%d.%d.%d", (int)ipb[0], (int)ipb[1], (int)ipb[2], (int)ipb[3]);
}

/**
 *  SerialToEthernetFunction
 *
 *  This function connects one serial port to a specific Ethernet
 *  socket. It will continually transfer data to/from serial to
 *  Ethernet. It does not return.
 *
 *  The idle_timeout is used to handle the case in which a client
 *  crashes or terminates without closing the socket normally,
 *  creating a half open socket condition on the server side. In
 *  this case the old socket will be left open until a new
 *  connection request is received. When the new connection
 *  request is detected, the old socket will be closed.
 *
 *  Parameters:
 *     fdserial     - The file descriptor (fd) of the serial port to use.
 *                    This is the value returned from the OpenSerial()
 *                    function.
 *
 *     listen_port  - The network port number to listen on for incoming
 *                    connections.
 *
 *     idle_timeout - The number of time ticks to wait before allowing
 *                    a new incoming network connection to bump the
 *                    old connection.
 */
void SerialToEthernetFunction(int fdserial, uint16_t listen_port, uint32_t idle_timeout)
{
    int fdListen = 0;

    while (1)
    {
        /*
         * If there is no active listen socket, create one. This
         * call will normally be true only on the first pass in
         * this function call. We set the listen queue size to 1,
         * since we will be closing this listen socket as soon
         * as we have a connection (with accept()).
         */
        if (fdListen == 0)
        {
            fdListen = listen(INADDR_ANY, listen_port, 1);
            iprintf("Listening on port %d, fdListen = %d\r\n", listen_port, fdListen);
        }

        // wait for an incoming network connection
        IPADDR client_ip;
        uint16_t client_port;
        int fdnet = accept(fdListen, &client_ip, &client_port, 0);
        iprintf("Connection accepted from %I:%d\r\n", client_ip, client_port);

        /*    If you want debug connection data to be sent to a serial port
         *    other than the debug port, you can uncomment the following code
         */
        /*
        writestring( fdserial, "writestring: Connection accepted from ");
        char buf[40];
        sShowIP( buf, client_ip );
        writestring( fdserial, buf );
        sniprintf( buf, 40, ":%d\r\n", client_port );
        writestring( fdserial, buf );
        */

        uint32_t timeout_value = TimeTick;

        /* If the idle_timeout is not infinite, close
         * the listen socket so no new connections can
         * be made. We will create a new listen socket
         * once the idle_timeout has expired to enable
         * the override feature.
         */
        if ((fdListen != 0) && (idle_timeout > 0))
        {
            close(fdListen);
            fdListen = 0;
            timeout_value = TimeTick;
            iprintf("Closed listen port %d\r\n", listen_port);
        }

        // Process data while the fdnet is valid
        while (fdnet > 0)
        {
            fd_set read_fds;
            fd_set error_fds;

            FD_ZERO(&read_fds);
            FD_ZERO(&error_fds);

            FD_SET(fdserial, &read_fds);
            FD_SET(fdserial, &error_fds);

            FD_SET(fdnet, &read_fds);
            FD_SET(fdnet, &error_fds);

            /*
             * If the idle_timeout has expired, open a new listen
             * socket so an override can occur.
             */
            if ((timeout_value + idle_timeout) < TimeTick)
            {
                if (fdListen == 0)
                {
                    fdListen = listen(INADDR_ANY, listen_port, 1);
                    iprintf("Override timeout expired, listening on port %d, fdListen = %d\r\n", listen_port, fdListen);
                }
                FD_SET(fdListen, &read_fds);
            }

            /* Pend on the file descriptors and wait for data. The
             * select() will return when data is detected on the
             * current fdnet and serial port, or on a new listen
             * socket if the idle_timeout has expired and an
             * override is enabled.
             */
            if (select(FD_SETSIZE, &read_fds, (fd_set *)0, &error_fds, TICKS_PER_SECOND * 2))
            {
                /* Network data was received, so send it out the serial port */
                if (FD_ISSET(fdnet, &read_fds))
                {
                    char buffer[80];
                    int n = read(fdnet, buffer, 80);
                    if (n > 0)
                    {
                        write(fdserial, buffer, n);
                        timeout_value = TimeTick;
                    }
                }

                /* Serial data was received, so send it out the network socket */
                if (FD_ISSET(fdserial, &read_fds))
                {
                    char buffer[80];
                    int n = read(fdserial, buffer, 80);
                    if (n > 0)
                    {
                        int w = write(fdnet, buffer, n);
                        if (w == n) { timeout_value = TimeTick; }
                    }
                }

                /* Network error */
                if (FD_ISSET(fdnet, &error_fds))
                {
                    close(fdnet);
                    fdnet = 0;
                    break;
                }

                /* If the idle_timeout has expired, and there is a
                 * new connection request from a client, then close
                 * the old socket connection.
                 */
                if (FD_ISSET(fdListen, &read_fds))
                {
                    iprintf("Override connection detected, closing connection to %I:%u\r\n", GetSocketRemoteAddr(fdnet),
                            GetSocketRemotePort(fdnet));
                    close(fdnet);
                    fdnet = 0;
                    break;
                }
            }   // end of select
        }       // while FDnet is valid
    }           // while forever
}   // end of function

/**
 *  SerialTask2
 *
 *  This is the task for the second serial port. It will process
 *  serial to Ethernet connections for the second serial port, which
 *  is 1 (number 0 is the first serial port). Once the SerialtoEthernetFunction
 *  is called, it never returns.
 */
void SerialTask2(void *pd)
{
    int fdserial1 = OpenSerial(1, BAUDRATE_TO_USE, STOP_BITS, DATA_BITS, eParityNone);
    SerialToEthernetFunction(fdserial1, TCP_PORT1_TO_USE, OVERRIDE_TIMEOUT);
}

/**
 *  UserMain
 *
 *  Main entry point for the example
 */
void UserMain(void *pd)
{
    init();
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);

    iprintf("Application: %s\r\nNNDK Revision: %s\r\n", AppName, GetReleaseTag());

    // Give any pending I/O time to print
    OSTimeDly(TICKS_PER_SECOND);

    // Close the serial ports
    SerialClose(0);
    SerialClose(1);

    /* One serial task will be run in UserMain, at the UserMain task
     * priority level. A task will be created to handle the
     * second serial port, called SerialTask2. We will create
     * SerialTask2 first. It will run at a priority level lower
     * than UserMain, which means it will process data only when
     * the UserMain priority level is blocking.
     */
    int fdserial0 = OpenSerial(0, BAUDRATE_TO_USE, STOP_BITS, DATA_BITS, eParityNone);

    // Make stdio work on fdserail0 for printf and iprintf
    ReplaceStdio(0, fdserial0);
    ReplaceStdio(1, fdserial0);
    ReplaceStdio(2, fdserial0);

    // Create task to handle fdserial1 (the seconds serial port)
    OSSimpleTaskCreatewName(SerialTask2, MAIN_PRIO + 1, "SerialTask2");

    // Enable processing for the first serial port. This function never returns
    SerialToEthernetFunction(fdserial0, TCP_PORT0_TO_USE, OVERRIDE_TIMEOUT);
}
