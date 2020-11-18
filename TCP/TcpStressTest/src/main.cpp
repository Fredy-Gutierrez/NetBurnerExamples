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
#include <init.h>
#include <ipshow.h>
#include <ctype.h>
#include <tcp.h>
#include <string.h>
#include <iosys.h>
#include <utils.h>

const char *AppName = "TCP Stress Test";

const uint16_t listenPort = 10000;

/*-----------------------------------------------------------------------------
 * Listen for and receive connections on a socket. Send a char and receive a char,
 * if the char is X then send status data back on the number of connections and exit
 * ------------------------------------------------------------------------------*/
void ListeningTest()
{
    int fdListen = listen( INADDR_ANY, listenPort, 5 );
    uint32_t numConnections = 0;
    uint32_t startTime = 0;
    char buffer[80];

    iprintf( "We are starting the Listen test\r\n" );

    while ( 1 )
    {
        int fd = accept( fdListen, NULL, NULL, 0 );
        if ( fd > 0 )
        {
            if ( numConnections++ == 0 )
            {
                startTime = TimeTick;
            }

            buffer[0] = 'G';
            write( fd, buffer, 1 );
            int rv = read( fd, buffer, 1 );
            if ( rv != 1 )
            {
                iprintf( "Receive error %d on socket\r\n", rv );
            }
            else
            {
                if ( buffer[0] == 'X' )
                {
                    uint32_t end_time = TimeTick;
                    sniprintf( buffer, 80, "Accepted %ld times in %ld ticks", numConnections, ( end_time - startTime ) );
                    write( fd, buffer, strlen( buffer ) + 1 );
                    close( fdListen );
                    close( fd );

                    iprintf( buffer );
                    break;
                }
                close( fd );
            }
        }
        else
        {
            iprintf( "Accept error %d\r\n", fd );
        }
    }
    iprintf( "%s\r\n", buffer );
}

/*-----------------------------------------------------------------------------
 * Listen for and receive a connections, then make connections back to that device.
 * Send a char and receive a char, if the char is X then send status data back on the
 * number of connections and exit
 * -----------------------------------------------------------------------------*/
void ConnectingTest()
{
    IPADDR ip;

    iprintf("We are starting the connect test\r\n");
    char buffer[80];
    uint32_t numConnections = 0;
    uint32_t startTime = 0;

    int fdListen = listen( INADDR_ANY, listenPort, 5 );
    int fd = accept(fdListen, &ip, NULL, 0);
    iprintf("We accepted a connection from %I\r\n", GetSocketRemoteAddr(fd));

    read(fd, buffer, 1);
    buffer[0] = 'X';
    write(fd, buffer, 1);
    close(fd);
    close(fdListen);

    iprintf("We are connecting to : %I\r\n", ip );

    while (1)
    {
        fd = connect(ip, listenPort, 20);

        if (fd > 0)
        {
            if (numConnections++ == 0)
            {
                startTime = TimeTick;
            }
            buffer[0] = 'G';
            write(fd, buffer, 1);
            int rv = read(fd, buffer, 1);
            if (rv != 1)
            {
                iprintf("Receive error %d on socket\r\n", rv);
            }
            else
            {
                if (buffer[0] == 'X')
                {
                    uint32_t end_time = TimeTick;
                    sniprintf(buffer, 80, "Connected %ld times in %ld ticks", numConnections, (end_time - startTime));
                    write(fd, buffer, strlen(buffer) + 1);
                    close(fd);
                    iprintf(buffer);
                    break;
                }
                close(fd);
            }
        }
        else
        {
            iprintf("Connect error %d\r\n", fd);
        }
    }
    iprintf("%s\r\n", buffer);
}

/*-----------------------------------------------------------------------------
 * Listen for an incoming byte and send a byte back, measuring the time. Note that
 * sending one byte at a time is the worse case performance number for TCP since it
 * requires all the overhead for a single byte.
 * ------------------------------------------------------------------------------*/
void ResponseTest()
{
    char buffer[80];
    uint32_t nchar = 0;
    uint32_t startTime = 0;

    iprintf("We are starting the response test\r\n");
    int fdListen = listen( INADDR_ANY, listenPort, 5 );
    if (fdListen < 0)
    {
        iprintf("Failed to listen %d\r\n", fdListen);
    }
    int fd = accept(fdListen, NULL, NULL, 0);
    if (fd < 0)
    {
        iprintf("Failed to accept %d\r\n", fd);
    }
    close(fdListen);

    while (1)
    {
        int rv = read(fd, buffer, 1);
        if (rv != 1)
        {
            iprintf("Receive error %d on socket\r\n", rv);
        }
        else
        {
            if (buffer[0] == 'X')
            {
                uint32_t end_time = TimeTick;
                sniprintf(buffer, 80, "Responded %ld times in %ld ticks", nchar, (end_time - startTime));
                write(fd, buffer, strlen(buffer) + 1);
                close(fd);
                iprintf(buffer);
                break;
            }

            rv = write(fd, buffer, 1);
            if (rv != 1)
            {
                iprintf("Write error\n");
            }
            if (nchar++ == 0)
            {
                startTime = TimeTick;
            }
        }
    }
    iprintf("%s\r\n", buffer);
}

/*-----------------------------------------------------------------------------
 * Select function response test
 *----------------------------------------------------------------------------*/
void SelectResponseTest()
{
    char buffer[80];
    uint32_t nchar = 0;
    uint32_t startTime = 0;

    iprintf("We are starting the Select response test\r\n");
    int fdListen = listen( INADDR_ANY, listenPort, 5 );
    if (fdListen < 0)
    {
        iprintf("Failed to listen %d\r\n", fdListen);
    }

    int fd = accept(fdListen, NULL, NULL, 0);
    close(fdListen);

    if (fd > 0)
    {
        while (1)
        {
            fd_set read_fds;
            fd_set error_fds;

            FD_ZERO(&read_fds);
            FD_ZERO(&error_fds);
            FD_SET(fd, &read_fds);
            FD_SET(fd, &error_fds);

            int sr = select( FD_SETSIZE, &read_fds, (fd_set *) 0, &error_fds, 40);

            if ((sr == 1) && (FD_ISSET(fd, &read_fds)))
            {
                int rv = read(fd, buffer, 1);
                if (rv != 1)
                {
                    iprintf("Receive error %d on socket\r\n", rv);
                }
                else
                {
                    if (buffer[0] == 'X')
                    {
                        uint32_t end_time = TimeTick;
                        sniprintf(buffer, 80, "Select Responded %ld times in %ld ticks", nchar, (end_time - startTime));
                        write(fd, buffer, strlen(buffer) + 1);
                        close(fd);
                        iprintf(buffer);
                        break;
                    }

                    rv = write(fd, buffer, 1);
                    if (rv != 1)
                    {
                        iprintf("Write error\n");
                    }

                    if (nchar++ == 0)
                    {
                        startTime = TimeTick;
                    }
                }
            }
            else
            {
                if (FD_ISSET(fd, &error_fds))
                {
                    iprintf("Select FD_ERROR error %d\r\n", sr);
                }
                else
                {
                    iprintf("Weird Select  error %d\r\n", sr);
                    iprintf("FDR= %d FDE=%d\r\n", FD_ISSET(fd, &read_fds), FD_ISSET(fd, &error_fds));
                }
            }
        }
    }
    else
    {
        iprintf("Accept failed fd = %d\r\n", fd);
    }
    iprintf("%s\r\n", buffer);
}


#define BUFFER_SIZE 20000
char DataBuffer[BUFFER_SIZE] __attribute__(( aligned( 16 )));

/*-----------------------------------------------------------------------------
 * Bulk data speed test
 *-----------------------------------------------------------------------------*/
void BulkSpeed()
{
    uint32_t nsent = 0;
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        DataBuffer[i] = ('A' + (i % 64));
    }

    int fdListen = listen( INADDR_ANY, listenPort, 5 );

    if (fdListen > 0)
    {
        int fda = accept(fdListen, NULL, NULL, 0);
        close(fdListen);

        if (fda > 0)
        {
            iprintf("Doing Bulk Speed\r\n");
            setsockoption(fda, SO_NOPUSH);
            setsocketackbuffers(fda, 40);
            read(fda, DataBuffer, 1);
            nsent = 0;
            while (1)
            {
                int rv = write(fda, DataBuffer, TCP_BUFFER_SEGMENTS * 1460);
                if (rv < 0)
                {
                    close(fda);
                    iprintf("Nsent = %ld\r\n", nsent);
                    break;
                }
                else
                {
                    nsent += rv;
                }
            }
        }
        else
        {
            iprintf("Accept failed\r\n");
        }
    }
    else
    {
        iprintf("FD listen is NOT ok\r\n");
    }
}


/*-----------------------------------------------------------------------------
 * Test task
 *-----------------------------------------------------------------------------*/
void TestTask(void *pd)
{
    while (1)
    {
        iprintf("Application started\n");
        ListeningTest();
        ConnectingTest();
        ResponseTest();
        SelectResponseTest();
        BulkSpeed();
    }
}


static uint32_t TestStk[USER_TASK_STK_SIZE] __attribute__( ( aligned( 4 ) ) );

/*-----------------------------------------------------------------------------
 * User Main
 *-----------------------------------------------------------------------------*/
void UserMain(void *pd)
{
    init();
    WaitForActiveNetwork();
    showIpAddresses();

    OSTaskCreatewName(TestTask, 0, (void *) &TestStk[USER_TASK_STK_SIZE], (void *) TestStk, MAIN_PRIO - 1, "Test Task");

    while (1)
    {
        if (charavail())
        {
            getchar();
            ShowCounters();
        }
    }
}
