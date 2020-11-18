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
#include <string.h>

const char *AppName = "SerialHttpSerialGetReq";

const char hostName[] = "myip.dnsdynamic.com";

#define SERIAL_INPUT_BUFFER_SIZE 65536
char serialInputBuffer[SERIAL_INPUT_BUFFER_SIZE];
int fdSerial1 = 0;

/**
 *  SendHttpGetRequest
 *
 *  Send HTTP GET request
 *  Note that most HTTP servers require a "Host:<name>" identifier
 *  as part of the GET request, which is why we do not simply pass
 *  one string as a URL.
 */
void SendHttpGetRequest(int fd, const char *hostName)
{
    static char szBuffer[1024];

    sniprintf(szBuffer, 1024, "GET / HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n\r\n", hostName);
    iprintf("Sending GET request to %s: \"%s\"\r\n", hostName, szBuffer);

    writestring(fd, szBuffer);
}

/**
 * ParseWebPage
 *
 *  Parse dnsdynamic web page
 *  This function will need to change depending on the exact web page
 *  you intend to parse. In this case the IP address is the last thing
 *  on the web page, so we go to the end of the page and back up to the
 *  beginning of the IP address.
 */
void ParseWebPage(char *buf, char *parseResult, int parseResultLen)
{
    char *pBuf = buf + strlen(buf) - 1;   // point to end of string
    int i = 0;

    while ((pBuf > buf) && (i < parseResultLen) && (isdigit(*pBuf) || (*pBuf == '.')))
    {
        pBuf--;
    }

    pBuf++;
    strncpy(parseResult, pBuf, parseResultLen);
}

/**
 *  UserMain
 *
 *  Main entry point of program
 */
void UserMain(void *pd)
{
    init();
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);
    StartHttp();

    close(1);
    fdSerial1 = SimpleOpenSerial(1, 115200);

    while (1)
    {
        iprintf("Hit any key to send GET request out UART1\r\n");
        getchar();

        iprintf("==>Sending GET request...\r\n");
        SendHttpGetRequest(fdSerial1, hostName);

        iprintf("==>Waiting 3 seconds for reply...\r\n");
        OSTimeDly(TICKS_PER_SECOND * 3);

        iprintf("==>Reading data...\r\n");
        for (int i = 0; i < SERIAL_INPUT_BUFFER_SIZE; i++)
        {
            serialInputBuffer[i] = '\0';
        }
        int bytesRead = read(fdSerial1, serialInputBuffer, SERIAL_INPUT_BUFFER_SIZE);
        serialInputBuffer[bytesRead] = '\0';

        iprintf("==============================================================\r\n");
        iprintf("Data read: %d bytes, %s\r\n", bytesRead, serialInputBuffer);
        iprintf("==============================================================\r\n");

        iprintf("==>Parsing data...\r\n");
        char ipAddress[64];
        ParseWebPage(serialInputBuffer, ipAddress, 64);
        iprintf("My IP Address: %s\r\n", ipAddress);
        iprintf("\r\n\r\n");
    }
}
