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
#include <stdio.h>
#include <tcp.h>
#include <nbrtos.h>
#include <iosys.h>
#include <init.h>
#include <fdprintf.h>

const char *AppName = "Simple TCP Server Example";

#define TCP_LISTEN_PORT 23   // Telent port number
#define RX_BUFSIZE (4096)

char RXBuffer[RX_BUFSIZE];

// Allocate task stack for TCP listen task
uint32_t   TcpServerTaskStack[USER_TASK_STK_SIZE];

/*-------------------------------------------------------------------
 * TCP Server Task
 *------------------------------------------------------------------*/
void TcpServerTask(void * pd)
{
    int listenPort = (int) pd;

    // Set up the listening TCP socket
    int fdListen = listen(INADDR_ANY, listenPort, 5);

    if (fdListen > 0)
    {
        IPADDR      clientAddress;
        uint16_t    clientPort;

        while(1)
        {
            /* The accept() function will block until a TCP client requests a connection. Once a client
             * connection is accepting, the file descriptor fdAccept is used to read/write to it.
             */
            iprintf( "Waiting for connection on port %d...\n", listenPort );
            int32_t fdAccept = accept(fdListen, &clientAddress, &clientPort, 0);
            iprintf("Connected to: %I\r\n", GetSocketRemoteAddr(fdAccept));

            writestring(fdAccept, "Welcome to the NetBurner TCP Server\r\n");
            fdprintf(fdAccept, "You are connected to IP Address %I:%d\r\n", GetSocketRemoteAddr(fdAccept), GetSocketRemotePort(fdAccept) );

            while (fdAccept > 0)
            {
                /* Loop while connection is valid. The read() function will return 0 or a negative number if the
                 * client closes the connection, so we test the return value in the loop. Note: you can also use
                 * ReadWithTimout() in place of read to enable the connection to terminate after a period of inactivity.
                */
                int n = 0;
                do
                {
                    n = read( fdAccept, RXBuffer, RX_BUFSIZE );
                    RXBuffer[n] = '\0';
                    iprintf( "Read %d bytes: %s\n", n, RXBuffer );
                } while ( n > 0 );

                iprintf("Closing client connection: %I\r\n", GetSocketRemoteAddr(fdAccept) );
                close(fdAccept);
                fdAccept = 0;
            }
        } // while(1)
    } // while listen
}


/*-------------------------------------------------------------------
 User Main
 ------------------------------------------------------------------*/
void UserMain(void * pd)
{
    init();

    // Create TCP Server task
    OSTaskCreatewName( TcpServerTask,
                       (void  *)TCP_LISTEN_PORT,
                       &TcpServerTaskStack[USER_TASK_STK_SIZE] ,
                       TcpServerTaskStack,
                       MAIN_PRIO - 1,   // higher priority than UserMain
                       "TCP Server" );

    while (1)
    {
        OSTimeDly( TICKS_PER_SECOND * 5 );
    }
}
