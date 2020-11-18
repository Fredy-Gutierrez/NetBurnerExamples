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
#include <tcp.h>

#define TIMEOUT_LIMIT (60)   // Inactivity timeout to close TCP socket, in seconds

/**
 *  If a new client TCP connection is attempted while one is active,  one of the following
 *  three actions can be taken:
 *      1. Ignore the incoming connection and leave current connection active
 *           (set override to 0xFFFFFFFF)
 *
 *      2. Replace the existing connection if it has been idle for some number of seconds
 *           (set override to the number of seconds to wait)
 *
 *      3. Always replace the existing connection
 *           (set override to 0 seconds)
 *
 *    This is done with the override timeout setting below
 */
// Number of seconds to wait before a new connection can override an existing one.
#define OVERIDE_TIMEOUT (20)

/*
 * Since this may be used for machine to machine connections we allow you
 * to easily change the messages that are sent.
 *
 * MTS messages are Message To Serial
 * MTN messages are Messages to the Network
 */
#define MTS_WHEN_NOTCONNECTED "Waiting for connection \r\n"
#define MTS_WHEN_CONNECTION_OPENED "New Connection Opened\r\n"
#define MTS_WHEN_CONNECTION_CLOSED "Connection Closed by Network \r\n"
#define MTS_WHEN_CONNECTION_TIMEDOUT "Connection Timed out and Closed\r\n"
#define MTS_WHEN_CONNECTION_OVERIDDEN "This Connection is being Overidden.\r\n"
#define MTN_WHEN_CONNECTION_OVERIDDEN "Your Connection was just Overidden\r\n"
#define MTN_WHEN_CONNECTION_OPENED "Connection Opened \r\n"
#define MTN_WHEN_CONNECTION_TIMEDOUT "Your Connection Timed out and will be Closed\r\n"

#define SERIAL_DATA_PORT (1)
#define SERIAL_BAUDRATE (115200)
#define STOP_BITS (1)
#define DATA_BITS (8)

#define TCP_LISTEN_PORT (23)   // The Telnet port

const char *AppName = "TcpToSerial Example";

/**
 *  UserMain
 */
void UserMain(void *pd)
{
    init();
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);

    // Close the serial port in case it is already open.
    SerialClose(SERIAL_DATA_PORT);

    // Open the serial port
    int fdserial = OpenSerial(SERIAL_DATA_PORT, SERIAL_BAUDRATE, STOP_BITS, DATA_BITS, eParityNone);

    // Set up the Listening TCP socket
    int fdListen = listen(INADDR_ANY, TCP_LISTEN_PORT, 5);

    if (fdListen > 0)
    {
        IPADDR client_addr;
        uint16_t port;
        writestring(fdserial, MTS_WHEN_NOTCONNECTED);

        while (1)
        {
            int fdnet = accept(fdListen, &client_addr, &port, 0);

            writestring(fdserial, MTS_WHEN_CONNECTION_OPENED);
            writestring(fdnet, MTN_WHEN_CONNECTION_OPENED);
            int tcp_timeout = 0;

            while (fdnet > 0)
            {
                fd_set read_fds;
                fd_set error_fds;

                FD_ZERO(&read_fds);
                FD_SET(fdnet, &read_fds);
                FD_SET(fdserial, &read_fds);

                // We only check for a waiting socket on the listener when we could do something about it.
                bool allowOveride = (OVERIDE_TIMEOUT != 0xFFFFFFFF);
                bool overideExpired = (OVERIDE_TIMEOUT == 0) || (tcp_timeout >= (OVERIDE_TIMEOUT * TICKS_PER_SECOND));
                if (allowOveride && overideExpired) { FD_SET(fdListen, &read_fds); }

                FD_ZERO(&error_fds);
                FD_SET(fdnet, &error_fds);

                if (select(FD_SETSIZE, &read_fds, (fd_set *)nullptr, &error_fds, TCP_WRITE_TIMEOUT))
                {
                    if (FD_ISSET(fdnet, &read_fds))
                    {
                        char buffer[40];
                        int n = read(fdnet, buffer, 40);
                        write(fdserial, buffer, n);
                        tcp_timeout = 0;
                    }

                    if (FD_ISSET(fdserial, &read_fds))
                    {
                        char buffer[40];
                        int n = read(fdserial, buffer, 40);
                        write(fdnet, buffer, n);
                    }

                    if (FD_ISSET(fdnet, &error_fds))
                    {
                        writestring(fdserial, MTS_WHEN_CONNECTION_CLOSED);
                        close(fdnet);
                        fdnet = 0;
                        writestring(fdserial, MTS_WHEN_NOTCONNECTED);
                    }

                    if (FD_ISSET(fdListen, &read_fds))
                    {
                        // We have a new suitor waiting on the listening socket.
                        writestring(fdserial, MTS_WHEN_CONNECTION_OVERIDDEN);
                        writestring(fdnet, MTN_WHEN_CONNECTION_OVERIDDEN);
                        close(fdnet);
                        fdnet = 0;
                    }
                }
                else if (TIMEOUT_LIMIT > 0)
                {
                    // Select Timed out
                    tcp_timeout += TCP_WRITE_TIMEOUT;   // Make sure we increment by the correct value
                    if (tcp_timeout >= (TIMEOUT_LIMIT * TICKS_PER_SECOND))
                    {
                        writestring(fdserial, MTS_WHEN_CONNECTION_TIMEDOUT);
                        writestring(fdnet, MTN_WHEN_CONNECTION_TIMEDOUT);
                        close(fdnet);
                        writestring(fdserial, MTS_WHEN_NOTCONNECTED);
                        fdnet = 0;
                    }
                }
            }   // Select
        }       // While net is valid
    }           // while 1
}
