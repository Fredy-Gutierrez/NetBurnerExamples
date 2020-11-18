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
#include <utils.h>
#include <system.h>
#include <iosys.h>
#include <stdio.h>
#include <ctype.h>
#include <tcp.h>
#include <init.h>
#include <ipshow.h>

const char* AppName = "TCP Multiple Sockets Example";

#define listenPort      (23)         // TCP port number to listen on
#define maxConnections  (10)         // Max number of file descriptors/connections
#define readBufferSize  (1024)       // Connection read buffer size

int32_t fdArray[maxConnections];     // Array of TCP file descriptors

/*-----------------------------------------------------------------------------
 * User Main
 *------------------------------------------------------------------------------*/
void UserMain( void* pd )
{
    init();
    WaitForActiveNetwork(TICKS_PER_SECOND * 10);
    showIpAddresses();

     // Listen for incoming TCP connections. You only need to call listen() one time.
     // Any incoming IP address is allowed, queue up to 5 connection requests at one time
    int32_t fdListen = listen( INADDR_ANY, listenPort, 5 );
    iprintf( "Listening for incoming connections on port %d\r\n", listenPort );

    while ( 1 )
    {
        // Declare file descriptor sets for select()
        fd_set readFds;
        fd_set errorFds;

        // Init the fd sets
        FD_ZERO( &readFds );
        FD_ZERO( &errorFds );

        // Configure the fd sets so select() knows what to process. In this case any fd data to be read, or an error
        for ( int32_t i = 0; i < maxConnections; i++ )
        {
            if ( fdArray[i] )   // The fd in the array will be > 0 if open and valid, so reset the file descriptor sets
            {
                FD_SET( fdArray[i], &readFds );
                FD_SET( fdArray[i], &errorFds );
            }
        }

        // select() should also process the listen fd
        FD_SET( fdListen, &readFds );
        FD_SET( fdListen, &errorFds );

        /* select() will block until any fd has data to be read, or has an error. When select() returns,
         * readFds and/or errorFds variables will have been modified to reflect the events.
         * */
        select( FD_SETSIZE, &readFds, ( fd_set * )0, &errorFds, 0 );

        // If the listen fd has a connection request, accept it.
        if ( FD_ISSET( fdListen, &readFds ) )
        {
            IPADDR      clientIp;
            uint16_t    clientPort;

            int fdAccept = accept( fdListen, &clientIp, &clientPort, 0 );

            // If accept() returned, find an open fd array slot
            if ( fdAccept > 0 )
            {
                for ( int32_t i = 0; i < maxConnections; i++ )
                {
                    if ( fdArray[i] == 0 )
                    {
                        fdArray[i] = fdAccept;
                        writestring( fdAccept, "Welcome to the NetBurner Multi-Socket TCP Server! 'Q' to quit." );
                        iprintf( "Added connection on fd[%d] = %d, Client IP: %I:%d\r\n",
                                i, fdAccept, GetSocketRemoteAddr(fdAccept), GetSocketRemotePort(fdAccept) );
                        fdAccept = 0;
                        break;
                    }
                }
            }

            // If no array positions are open, close the connection
            if ( fdAccept )
            {
                writestring( fdAccept, "I am sorry, but the server is full\r\n" );
                iprintf("Server Full\r\n");
                close( fdAccept );
            }
        }

        // If the listen fd has an error, close it and reopen
        if ( FD_ISSET( fdListen, &errorFds ) )
        {
            close( fdListen );
            fdListen = listen( INADDR_ANY, listenPort, 5 );
        }

        // Process each fd array element and check it against readFds and errorFds.
        for ( int32_t i = 0; i < maxConnections; i++ )
        {
            if ( fdArray[i] )
            {
                // Check for data to be read
                if ( FD_ISSET( fdArray[i], &readFds ) )
                {
                    char buffer[readBufferSize];
                    int rv = read( fdArray[i], buffer, readBufferSize );
                    if ( rv > 0 )
                    {
                        buffer[rv] = 0;
                        if ( buffer[0] == 'Q' )
                        {
                            iprintf( "Closing connection fd[%d]\r\n", i );
                            writestring( fdArray[i], "Bye\r\n" );
                            close( fdArray[i] );
                            fdArray[i] = 0;
                        }
                        else
                        {
                            iprintf( "Read \"%s\" from fd[%d]\r\n", buffer, i );
//                            sniprintf( buffer, readBufferSize, "Server read %d byte(s)\r\n", rv );
//                            writestring( fdArray[i], buffer );
                        }
                    }
                    else
                    {
                        iprintf( "Read Error on fd[%d]\r\n", fdArray[i] );
                        FD_SET( fdArray[i], &errorFds );
                    }
                } // data available to read

                // Check for errors
                if ( FD_ISSET( fdArray[i], &errorFds ) )
                {
                    iprintf( "Error on fd[%d], closing connection\r\n", i );
                    close( fdArray[i] );
                    fdArray[i] = 0;
                }
            }   // if fd is valid
        }   // process each connection in the array
    } // while (1)
}  // UserMain
