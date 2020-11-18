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


/***************************************************************************
TcpSpeedTest
Note that this test measures DATA PAYLOAD, not total number of bits transferred,
which is really the number that matters. Total mega bits per second (Mbps)
will be much higher than the numbers reported in this test for just the data
payload.

Build notes:
- Built with Visual Studio 2015
- Turn Unicode off in project properties->configuration properties->general->character set
- Add wsock32.lib to linker input field (under properties)

Executing the program:
- Need a NetBurner device running it's speed test code on the same LAN
- Open a command prompt, type "TcpSpeedTest <ip address>" where "ip address"
  is the address of the NetBurner device.
- The "-r" command line option will repeat forever or until you use cntl-c
  to exit.

***************************************************************************/
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include "stdafx.h"
#include <windows.h>
#include <conio.h>
#include <winsock.h>
#define PRINTERROR(s) fprintf(stderr,"\n%s: %d\n", s, WSAGetLastError())
#define sprintf(...) sprintf_s(...)
#else
#ifdef __APPLE__
#define SOCKET int
#include <netinet/in.h>
#endif
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define SOCKADDR_IN sockaddr_in
#define LPHOSTENT struct hostent *
#define PRINTERROR(s) fprintf(stderr,"\n%s: %d - %d\n", s, h_errno, errno)
#define closesocket(x) close(x)
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#endif


#define LISTEN_PORT_NUMBER (1234)

#define RX_BUFSIZE 100000               // Receive buffer size for recv() call

// Command line parameters
//#define TotalBytesToReceive 10000000      // Number of bytes to receive from remote host
uint32_t TotalBytesToReceive = 10000000;
bool        bRepeat = false;

char rxBuffer[RX_BUFSIZE + 10];     // Receive buffer

//int _tmain(int argc, _TCHAR* argv[])
int main(int argc, char* argv[])
{
    int         nRet;
    SOCKET      mySocket;
    SOCKADDR_IN saServer;
    LPHOSTENT   lpHostEntry;
    char        Buffer[4096];
    char        *ipAddr = 0;
    float       bytesPerSecondMax = 0;
    float       bytesPerSecondMin = 99999999.0;
    uint32_t    totalBytesRead = 0;
    float       totalTestTime = 0;

#ifdef _WIN32
    // Initialize WinSock and check the version
    uint16_t    wVersionRequested = MAKEuint16_t(1,1);
    WSADATA     wsaData;
    nRet = WSAStartup(wVersionRequested, &wsaData);
    if (wsaData.wVersion != wVersionRequested)
    {
        fprintf(stderr,"\n Incorrect wsock version\n");
        return -1;
    }
#endif

    if ( argc < 2 )
    {
        printf("Usage: TcpSpeedTest <ip address or name>\n");
        printf(" The -r  = repeat test in loop\n");
        printf(" The -s  = specify total bytes to send (default is 10,000,000\n");
        printf(" The destination port number must be 1234\n");
        exit(1);
    }

    // Loop until all command line arguments are parsed
    for (int i = 1; i < argc; i++)
    {
        printf("arg %%1 = %s\n", argv[i]);

        if (argv[i][0]=='-')
        {
            if ((argv[i][1]=='r') || (argv[i][1]=='R'))
            {
                bRepeat=true;
            }

            if ((argv[i][1] == 's') || (argv[i][1] == 'S'))
            {
                TotalBytesToReceive = atol((const char *)&argv[i][2]);
            }
        }
        else
            ipAddr = (char *)argv[i];
    }

    printf("Total bytes to receive set to %u\n", TotalBytesToReceive);


    printf("Getting IP For %s\r\n", ipAddr);
    do
    {
        lpHostEntry = gethostbyname(ipAddr);
        if (lpHostEntry == NULL)
        {
            PRINTERROR("A: gethostbyname()");
            return -1;
        }

        // Create a TCP/IP stream socket
        mySocket = socket(AF_INET,          // Address family
                            SOCK_STREAM,    // Socket type
                            IPPROTO_TCP);   // Protocol
        if (mySocket == INVALID_SOCKET)
        {
            PRINTERROR("B: socket()");
            return -1;
        }

        // Fill in the address structure
        saServer.sin_family = AF_INET;
        saServer.sin_port = htons(LISTEN_PORT_NUMBER);  // Port number from command line
#ifdef _WIN32
        saServer.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list); //  Server's address
        nRet = connect(mySocket,        // Socket
            (LPSOCKADDR)&saServer,      // Server address
            sizeof(struct sockaddr));   // Length of server address structure
#else
        saServer.sin_addr = *((struct in_addr*)*lpHostEntry->h_addr_list); //  Server's address
        nRet = connect(mySocket,        // Socket
            (sockaddr *)&saServer,      // Server address
            sizeof(struct sockaddr));   // Length of server address structure
#endif


        if (nRet == SOCKET_ERROR)
        {
            PRINTERROR("C: socket()");
            closesocket(mySocket);
            return -1;
        }

        {
            long bytesRead = 0;

            // Set the per-socket receive buffer space size
            int optValue = 200000;  // setsocketopt value for SO_RCVBUF
            nRet = setsockopt( mySocket, SOL_SOCKET, SO_RCVBUF, (const char *)&optValue, sizeof(optValue) );
            if ( nRet != 0 )
            {
                printf("*** Error in setsockopt(), returned: %d, error code = %d", nRet, errno);
                closesocket(mySocket);
                exit(1);
            }

            char tempBuf[80];
            int n = sprintf(tempBuf, "%u", TotalBytesToReceive);
            tempBuf[n+1] = '\0';
            send( mySocket, tempBuf, strlen( tempBuf ) + 1, 0 );

#ifdef _WIN32
            uint32_t startTime = GetTickCount();           // Start time in milliseconds
#else
            timeval time;
            gettimeofday(&time, NULL);
            time_t startTime = (time.tv_sec * 1000) + (time.tv_usec / 1000);
#endif
            printf("Start time: %lu ms\n", startTime );

            while (bytesRead < TotalBytesToReceive)
            {
                nRet = recv( mySocket, rxBuffer, RX_BUFSIZE, 0 );
                if (nRet > 0)
                {
                    bytesRead += nRet;
                }
                else
                {
                    printf( "*** Error: recv() failed, return value: %d\n", nRet );
                    closesocket(mySocket);
                    exit(-1);
                }
            }


#ifdef _WIN32
            uint32_t stopTime = GetTickCount();
            float timeInSeconds = (float)(stopTime - startTime) / 1000;
#else
            gettimeofday(&time, NULL);
            time_t stopTime =  (time.tv_sec * 1000) + (time.tv_usec / 1000);
            float timeInSeconds = ((float)(stopTime - startTime)/1000);
#endif
            printf("Stop time:  %lu ms\n", stopTime );
            printf("Test time: %ld ms (%0.3f s)\n", stopTime - startTime, timeInSeconds );

            float bytesPerSecond = (float)bytesRead / timeInSeconds;

            if ( bytesPerSecond > bytesPerSecondMax)
                bytesPerSecondMax = bytesPerSecond;

            if ( bytesPerSecond < bytesPerSecondMin )
                bytesPerSecondMin = bytesPerSecond;

            if (bRepeat)
            {
                totalBytesRead += bytesRead / 1000000;
                totalTestTime += timeInSeconds;

                printf("Total Bytes Read: %u MB, Bits/Sec: %0.3f MBps, Max:%0.3f MBps, Min:%0.3f\n",
                    totalBytesRead, totalBytesRead / totalTestTime * 8, bytesPerSecondMax / 1000000.0 * 8.0, bytesPerSecondMin / 1000000.0 * 8.0 );
                    //totalBytesRead, bytesPerSecond / 1000000 * 8, bytesPerSecondMax / 1000000.0 * 8.0, bytesPerSecondMin / 1000000.0 * 8.0 );
                printf("\n");
            }
            else
            {
                printf("Results are for TCP DATA PAYLOAD. Does not include headers, checksums, etc\n");
                printf("Time: %0.3f s, Bytes Read: %ld (%ld MB)\n", timeInSeconds, bytesRead, bytesRead/1000000);
                printf("Bytes/Sec: %0.1f (%0.1f MBps), Bits/Sec: %0.1f Mbps\n", bytesPerSecond, bytesPerSecond/1000000, bytesPerSecond / 1000000*8 );
                printf("\n\n");
            }

            closesocket(mySocket);
        }
    } while (bRepeat == true);

    return 0;
}


