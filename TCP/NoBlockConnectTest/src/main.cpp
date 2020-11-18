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
#include <ctype.h>
#include <init.h>
#include <tcp.h>
#include <utils.h>
#include <iosys.h>
#include <stdlib.h>

const char * AppName = "NoBlockConnect";

IPADDR GetIpAddr()
{
    static char buf[65];
    iprintf("Address to connect to?");
    fgets(buf,65,stdin);
    IPADDR ip = AsciiToIp(buf);
    iprintf("\r\nConnecting to ");
    ShowIP(ip);
    iprintf("\r\n");
    return ip;
}

uint16_t GetPort()
{
    static char buf[65];
    iprintf("Port to connect to?");
    fgets(buf,65,stdin);
    uint16_t port = atoi(buf);
    iprintf("Conencitng to port %d\r\n", port);
    return port;
}

void DisplayMenu()
{
    iprintf("MAIN MENU\r\n");
    iprintf("C = Make a connection using connect()\r\n");
    iprintf("N = Make a connection using NoBlockConnect()\r\n");
    iprintf("X = Close the current connection\r\n");
    iprintf("I = Specify the destination IP address and port number\r\n");
    iprintf("Any other characters typed will be sent if a TCP connection exists\r\n");
}

/*-----------------------------------------------------------------------------
 * UserMain
 *----------------------------------------------------------------------------*/
void UserMain(void * pd)
{
    init();

    IPADDR destIpAddress = GetIpAddr();
    uint16_t destIpPort  = GetPort();

    int fd = -1;
    bool bValid = false;
    uint32_t TimeMark = Secs;

    DisplayMenu();
    while (1)
    {
        if (charavail())
        {
            char c = getchar();
            switch (toupper(c))
            {
                case 'C':
                {
                    iprintf("Connecting...");
                    if (fd > 0) close(fd);

                    fd = connect(destIpAddress,destIpPort, 20);
                    if (fd > 0)
                    {
                        iprintf("Connected\r\n");
                        bValid = true;
                    }
                    else
                    {
                        iprintf("Connect failed\r\n");
                    }
                }
                    break;

                case 'I':
                    if (fd <= 0)
                    {
                        destIpAddress = GetIpAddr();
                        destIpPort = GetPort();
                    }
                    else
                        iprintf("Cannot change IP or port while connected\r\n Close first (command X)\r\n");
                    break;

                case 'N':
                {
                    iprintf("No Block Connecting...");
                    if (fd > 0) close(fd);

                    fd = NoBlockConnect(destIpAddress,destIpPort);
                    if (fd > 0)
                    {
                        iprintf("Connect started\r\n");
                        bValid = false;
                    }
                    else
                    {
                        iprintf("Connect failed\r\n");
                    }
                }
                break;

                case 'X':
                {
                    if (fd > 0)
                    {
                        iprintf("Closing..");
                        close(fd);
                        fd = -1;
                    }
                    else
                    {
                        iprintf("Not connected, cant close\r\n");
                    }
                }
                break;

                default:
                    if ((fd > 0) && (bValid)) write(fd, &c, 1);
            }
        }

        if ((fd > 0) && (bValid) && ((TimeMark + 5) <= Secs))
        {
            static char buf[255];
            sniprintf(buf, 255, "Time XTick at :%ld\n", Secs);
            writestring(fd, buf);
            TimeMark = Secs;
        }

        if ((fd > 0) && (!bValid))
        {
            if (TcpGetSocketState(fd) == TCP_STATE_ESTABLISHED)
            {
                bValid = true;
                iprintf("NoBlock connection is done\r\n");
            }
            else
            {
                iprintf("State = %d\r\n", TcpGetSocketState(fd));
            }
        }
    }
}
