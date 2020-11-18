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

#include <stdio.h>
#include <startnet.h>
#include <tcp.h>
#include <nbrtos.h>
#include <init.h>
#include <fdprintf.h>

const char *AppName = "TCP Keep Alive Example";

#define TCP_LISTEN_PORT 23   // Telnet port number
#define RX_BUFSIZE (4096)
char RXBuffer[RX_BUFSIZE];

/*-------------------------------------------------------------------
 * TCP Server Task. Listens for incoming data.
 *-------------------------------------------------------------------*/
void TcpServerTask(void * pd)
{
    int listenPort = (int) pd;          // Listen port is passed to task at startup

    int fdListen = listen(INADDR_ANY, listenPort, 5);       // Set up the listening TCP socket
    if (fdListen > 0)
    {
        IPADDR      remoteAddress;
        uint16_t    remotePort;

        while(1)
        {
            // The accept() function will block until a TCP client requests a connection.
            // Once a client connection is accepted, the file descriptor fdNet is used to read/write to it.
            iprintf( "Wainting for connection on port %d...\r\n", listenPort );
            int fdNet = accept(fdListen, &remoteAddress, &remotePort, 0);

            iprintf("Connected to: %I:%d\r\n", GetSocketRemoteAddr(fdNet), GetSocketRemotePort(fdNet));

            writestring(fdNet, "Welcome to the NetBurner TCP Server\r\n");          // Welcome message seen by client
            fdprintf(fdNet, "You are connected to %I:%d\r\n", GetSocketLocalAddr(fdNet), GetSocketLocalPort(fdNet));

            while (fdNet > 0)
            {
                /* Loop while connection is valid. The ReadWithTimeout() function will return
                   a negative number if the client closes the connection, and 0 if not data
                   is received during the designated timeout period, so we test the return
                   value in the loop.
                */
                int bytesRead = 0;
                BOOL keepAliveSent = FALSE;
                uint32_t LastRxTime = TcpGetLastRxTime(fdNet);   // Get time stamp of last rx packet
                do
                {
                    bytesRead = ReadWithTimeout( fdNet, RXBuffer, RX_BUFSIZE, 1 * TICKS_PER_SECOND );

                    if(bytesRead > 0)           // received data
                    {
                        RXBuffer[bytesRead] = '\0';
                        iprintf( "Read %d bytes: %s\r\n", bytesRead, RXBuffer );
                    }
                    else if( bytesRead == 0 )   // received no data and timed out
                    {
                        iprintf("read timed out\r\n");

                        if(!keepAliveSent)  // if not data has been sent, send a keep alive
                        {
                            LastRxTime = TcpGetLastRxTime(fdNet);   // Get time stamp of rx packet
                            TcpSendKeepAlive(fdNet);
                            keepAliveSent = TRUE;
                            iprintf("keepalive packet sent\r\n");
                        }
                        else   // keep alive was previously sent, so check for response
                        {
                            if(LastRxTime == TcpGetLastRxTime(fdNet))
                            {
                                iprintf("Connection did not respond to keep alive\r\n");
                                close(fdNet);
                            }
                            else
                            {
                                iprintf("Client responded to keep alive\r\n");
                                keepAliveSent = false;
                            }
                        }
                    }
                } while ( bytesRead >= 0 );

                iprintf("Closing client connection: %I:%d", remoteAddress, remotePort);
                close(fdNet);
                fdNet = 0;
            }
        } // while(1), keep waiting for incoming connections
    } // if listen socket open was successful
}


uint32_t   TcpServerTaskStack[USER_TASK_STK_SIZE];

/*------------------------------------------------------------------
 * User Main
 *------------------------------------------------------------------*/
void UserMain(void * pd)
{
    init();
    WaitForActiveNetwork();

    iprintf("Creating TCP Server Task...");
    int32_t status = OSTaskCreatewName( TcpServerTask,
                                        (void  *)TCP_LISTEN_PORT,
                                        &TcpServerTaskStack[USER_TASK_STK_SIZE] ,
                                        TcpServerTaskStack,
                                        MAIN_PRIO - 1,
                                        "Tcp Server");
    if ( status > 0 )
        iprintf("success\r\n");
    else
        iprintf("failed\r\n");

    while (1)
    {
        OSTimeDly( TICKS_PER_SECOND * 5 );
    }
}


