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
TcpSressTest
Note that this test measures DATA PAYLOAD, not total number of bits transferred,
which is really the number that matters. Total mega bits per second (Mbps)
will be much higher than the numbers reported in this test for just the data
payload.

Build notes:
- Built with Visual Studio 2015
- Turn Unicode off in project properties->configuration properties->general->character set
- Add wsock32.lib to linker input field (under properties)

Executing the program:
- Need a NetBurner device running the stress test code on the same LAN
- Open a command prompt, type "TcpStrssTest <ip address>" where "ip address" is the address of the NetBurner device.
- The "-r" command line option will repeat forever or until you use cntl-c to exit.

***************************************************************************/
#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <errno.h>
#include <winsock.h>

#define PRINTERROR(s) fprintf(stderr,"\n%s: %d\n", s, WSAGetLastError())
#define LISTEN_PORT_NUMBER (10000)


const char *targetIpAddr;


/*-----------------------------------------------------------------------------
Connect and close the specified number of times to an open listen socket on the 
target.
-------------------------------------------------------------------------------*/
int ListeningTest(LPHOSTENT lpHostEntry, int ntimes)
{
	int nRet;
	SOCKET mySocket;
	SOCKADDR_IN saServer;
	char Buffer[4096];

	printf("Starting listen test, connecting to target %d times...\n", ntimes);

	// Fill in the address structure
	saServer.sin_family = AF_INET;
	saServer.sin_addr = *((LPIN_ADDR)* lpHostEntry->h_addr_list); //  Server's address
	saServer.sin_port = htons(LISTEN_PORT_NUMBER);   // Port number from command line

	// Connect and close the specified number of times
	for (int i = 0; i < (ntimes - 1); i++)
	{
		// Create a TCP/IP stream socket
		mySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (mySocket == INVALID_SOCKET)
		{
			PRINTERROR("B: socket()"); 
			return -1;
		}

		nRet = connect(mySocket, (LPSOCKADDR)& saServer, sizeof(struct sockaddr)); 

		if (nRet == SOCKET_ERROR)
		{
			PRINTERROR("C: socket()"); 
			closesocket(mySocket); 
			return -1;
		}

		send(mySocket, "O", 1, 0);

		nRet = recv(mySocket, Buffer, 1, 0);
		if ((nRet != 1) || (Buffer[0] != 'G'))
		{
			printf("Error nRet=%d s Char = %c\n", nRet, Buffer[0]);
            closesocket(mySocket);
            return -1;
		}
		closesocket(mySocket);
	}

	// Get stats from the target device
	mySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (mySocket == INVALID_SOCKET)
	{
		PRINTERROR("B: socket()"); return -1;
	}

	nRet = connect(mySocket, (LPSOCKADDR)& saServer, sizeof(struct sockaddr));  
	if (nRet == SOCKET_ERROR)
	{
		PRINTERROR("C: socket()"); closesocket(mySocket); return -1;
	}

	send(mySocket, "X", 1, 0);
	Buffer[0] = 0;
	do
	{
		nRet = recv(mySocket, Buffer, 256, 0);
		if (nRet > 1)
		{
			printf("Results from target: %s\n", Buffer);
		}
	} while (nRet < 10);
	closesocket(mySocket);
	return 0;
}



/*-----------------------------------------------------------------------------
Connect and close the specified number of times to an open listen socket on the
target.
-------------------------------------------------------------------------------*/
int ConnectingTest(LPHOSTENT lpHostEntry, int ntimes)
{
	int nRet;
	SOCKET mySocket;
	SOCKET lSocket;
	SOCKADDR_IN saServer;
	SOCKADDR_IN saListen;
	char Buffer[4096];

	printf("Starting client connect test, listening on port %d...\n", LISTEN_PORT_NUMBER);

	// Fill in the address structure
	saServer.sin_family = AF_INET;
	saServer.sin_addr = *((LPIN_ADDR)* lpHostEntry->h_addr_list); //  Server's address
	saServer.sin_port = htons(LISTEN_PORT_NUMBER);   // Port number from command line

	mySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (mySocket == INVALID_SOCKET)
	{
		PRINTERROR("B: socket()");
		closesocket(mySocket);
		return -1;
	}

	nRet = connect(mySocket, (LPSOCKADDR)& saServer, sizeof(struct sockaddr));   // Length of server address structure
	if (nRet == SOCKET_ERROR)
	{
		PRINTERROR("C: socket()");
		closesocket(mySocket);
		return -1;
	}
	send(mySocket, "X", 1, 0);
	nRet = recv(mySocket, Buffer, 256, 0);
	closesocket(mySocket);

	saListen.sin_family = AF_INET;
	saListen.sin_addr.s_addr = INADDR_ANY;
	saListen.sin_port = htons(LISTEN_PORT_NUMBER);  // Port number from command line

	lSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (bind(lSocket, (LPSOCKADDR)& saListen, sizeof(saListen)) == SOCKET_ERROR)
	{
		PRINTERROR("A: Bind()");
        closesocket(lSocket);
		return -1;
	}

	if (listen(lSocket, 5) < 0)
	{
		PRINTERROR("Listen");
        closesocket(lSocket);
		return -1;
	}

	int adrlen;
	for (int i = 0; i < (ntimes - 1); i++)
	{
		// Create a TCP/IP stream socket
		adrlen = sizeof(struct sockaddr);
		mySocket = accept(lSocket, (LPSOCKADDR)& saServer, &adrlen); // Length of server address structure
		if (mySocket == INVALID_SOCKET)
		{
			PRINTERROR("B: accept()"); return -1;
		}

		send(mySocket, "O", 1, 0);

		nRet = recv(mySocket, Buffer, 1, 0);
		if ((nRet != 1) || (Buffer[0] != 'G'))
		{
			printf("Error nRet=%d s Char = %c\n", nRet, Buffer[0]); return -1;
		}
		closesocket(mySocket);
	}

	adrlen = sizeof(struct sockaddr);
	mySocket = accept(lSocket, (LPSOCKADDR)& saServer, &adrlen); // Length of server address structure
	if (mySocket == INVALID_SOCKET)
	{
		PRINTERROR("B: accept()"); return -1;
	}

	send(mySocket, "X", 1, 0);
	Buffer[0] = 0;
	do
	{
		nRet = recv(mySocket, Buffer, 256, 0);
		if (nRet > 1)
		{
			printf("Results from target: %s\n", Buffer);
		}
	} while (nRet < 10);
	closesocket(mySocket);
	closesocket(lSocket);
	return 0;
}


/*-----------------------------------------------------------------------------
Send a byte and receive a byte back, measuring the time. Note that sending one 
byte at a time is the worse case performance number for TCP since it requires 
all the overhead for a single byte.
------------------------------------------------------------------------------*/
int ResponseTest(LPHOSTENT lpHostEntry, int ntimes)
{
	int nRet;
	SOCKET mySocket;
	SOCKADDR_IN saServer;
	char Buffer[4096];

	printf("Starting client response test for a single byte...\n");

	// Fill in the address structure
	saServer.sin_family = AF_INET;
	saServer.sin_addr = *((LPIN_ADDR)* lpHostEntry->h_addr_list); //  Server's address
	saServer.sin_port = htons(LISTEN_PORT_NUMBER);   // Port number from command line

	mySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	// Create a TCP/IP stream socket
	if (mySocket == INVALID_SOCKET)
	{
		PRINTERROR("B: socket()");
		closesocket(mySocket);
		return -1;
	}

	nRet = connect(mySocket, (LPSOCKADDR)& saServer, sizeof(struct sockaddr));   // Length of server address structure
	if (nRet == SOCKET_ERROR)
	{
		PRINTERROR("C: socket()"); closesocket(mySocket); return -1;
	}

	for (int i = 0; i < (ntimes); i++)
	{
		char sc;

		Buffer[0] = (i & 0xFF);
		if (Buffer[0] == 'X')
		{
			Buffer[0] = 'x';
		}

		sc = Buffer[0];		// send increment count or x
		send(mySocket, Buffer, 1, 0);
		nRet = recv(mySocket, Buffer, 1, 0);
		if ((nRet != 1) || (Buffer[0] != sc))
		{
			printf("Error nRet = %d s Char = %c\n", nRet, Buffer[0]); 
			return -1;
		}
	}

	send(mySocket, "X", 1, 0);
	Buffer[0] = 0;
	do
	{
		nRet = recv(mySocket, Buffer, 256, 0);
		if (nRet > 1)
		{
			printf("Results from target: %s\n", Buffer);
		}
	} while (nRet < 10);
	closesocket(mySocket);
	return 0;
}


char retbuffer[100010];

int BulkSpeed(LPHOSTENT lpHostEntry, int ntimes)
{
	int nRet;
	SOCKET mySocket;
	SOCKADDR_IN saServer;
	//char Buffer[4096];
	//int FieldSize;
	//int BytesWritten;
	//unsigned long nWritten;
	float fMax = 0;
	float fMin = 9999999.0;
	float nTotal = 0;

	printf("Starting bulk speed test...\n");

	// Create a TCP/IP stream socket
	mySocket = socket(AF_INET, // Address family
		SOCK_STREAM,           // Socket type
		IPPROTO_TCP);          // Protocol
	if (mySocket == INVALID_SOCKET)
	{
		PRINTERROR("B: socket()");
		closesocket(mySocket);
		return -1;
	}

	// Fill in the address structure
	saServer.sin_family = AF_INET;
	saServer.sin_addr = *((LPIN_ADDR)* lpHostEntry->h_addr_list); //  Server's address
	saServer.sin_port = htons(LISTEN_PORT_NUMBER);   // Port number from command line

	nRet = connect(mySocket,        // Socket
		(LPSOCKADDR)& saServer,     // Server address
		sizeof(struct sockaddr));   // Length of server address structure

	if (nRet == SOCKET_ERROR)
	{
		PRINTERROR("C: socket()");
		closesocket(mySocket);
		return -1;
	}

	{
		long nread = 0;
		int sv = 200000;

		nRet = setsockopt(mySocket,	SOL_SOCKET,	SO_RCVBUF,	(const char *)&sv,	sizeof(sv));
		send(mySocket, "10000000\0", 9, 0);
		uint32_t ds = GetTickCount();		// time in milliseconds since system start

		while (nread < 10000000)
		{
			nRet = recv(mySocket, retbuffer, 100000, 0);
			if (nRet > 0)
			{
				nread += nRet;
			}
			else
			{
				printf("Failed \r\n"); exit(-1);
				return 0;
			}
		}

		uint32_t de = GetTickCount();	// stop time ticks
		float f = (float) (de - ds);
		f /= 1000;
		f = (float)nread / f;

		if (f > fMax)
		{
			fMax = f;
		}

		if (f < fMin)
		{
			fMin = f;
		}

		nTotal += nread;
		printf("Time: %u ms, Bytes: %ld. %0.2f MBytes per second\n", de - ds, nread, f/1000000.0);

		closesocket(mySocket);
	}
	return 0;
}





int main(int argc, char **argv)
{
	uint16_t wVersionRequested = MAKEuint16_t(1, 1);
	WSADATA wsaData;
	LPHOSTENT lpHostEntry;
	int nRet;
	const char ipDefault[] = "localhost";
	bool bRepeat = FALSE;
	int nTries = 20;

	// Initialize WinSock and check the version
	nRet = WSAStartup(wVersionRequested, &wsaData);
	if (wsaData.wVersion != wVersionRequested)
	{
		fprintf(stderr, "\n Wrong version\n");
		return -1;
	}


	targetIpAddr = ipDefault;	// init to someting to make comiler error go away

	for (int i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			if ((argv[i][1] == 'r') || (argv[i][1] == 'R'))
			{
				bRepeat = TRUE;
			}
			if ((argv[i][1] == 'n') || (argv[i][1] == 'N'))
			{
				nTries = atoi(argv[i] + 2);
			}
		}
		else
		{
			targetIpAddr = argv[i];
		}
	}

	lpHostEntry = gethostbyname(targetIpAddr);

	if (lpHostEntry == NULL)
	{
		PRINTERROR("A: gethostbyname()");
		return -1;
	}

	printf("--------------------------------------------------\n");
	printf("Getting IP Address for target: %s\r\n", targetIpAddr);
	int ncycles = 0;
	do
	{
		ListeningTest(lpHostEntry, nTries);	printf("\n");
		ConnectingTest(lpHostEntry, nTries); printf("\n");
		ResponseTest(lpHostEntry, nTries); printf("\n");
		ResponseTest(lpHostEntry, nTries);	printf("\n");
		BulkSpeed(lpHostEntry, nTries);	printf("\n");
		if ((ncycles))
		{
			printf("%d Cycles complete\r\n", ncycles);
		}
		ncycles++;
	} while (bRepeat);
	printf("--------------------------------------------------\n");

	return 0;
}
