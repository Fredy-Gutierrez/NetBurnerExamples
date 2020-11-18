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

// NB Definitions
#include <predef.h>

// NB Libs
#include <crypto/ssl.h>
#include <fdprintf.h>
#include <init.h>
#include <netinterface.h>
#include <nettypes.h>
#include <startnet.h>
#include <stdio.h>
#include <tcp.h>

const char *AppName = "Simple SSL/TLS Server Example";

#define TCP_LISTEN_PORT 8883   // MQTT secure port number
#define RX_BUFSIZE (4096)

//----- Global Vars -----
char RXBuffer[RX_BUFSIZE];

// Allocate task stack for UDP listen task
uint32_t TcpServerTaskStack[USER_TASK_STK_SIZE];

/*-------------------------------------------------------------------
 TCP Server Task
 -------------------------------------------------------------------*/
void TcpServerTask(void *pd)
{
    int ListenPort = (int)pd;

    // Set up the listening TCP socket
    int fdListen = listen(INADDR_ANY, ListenPort, 5);

    if (fdListen > 0)
    {
        IPADDR client_addr;
        uint16_t port;

        IPADDR ip = GetNullIP();
        InterfaceBlock *pifb = GetInterfaceBlock(0);
        if (pifb != nullptr) { ip = pifb->ip4.cur_addr; }

        while (1)
        {
            // The accept() function will block until a client requests
            // a connection. Once a client connection is accepted, the
            // file descriptor fdnet is used to read/write to it.
            iprintf("Waiting for connection on port %d...\n", ListenPort);
            int fdnet = SSL_accept(fdListen, &client_addr, &port, 0);

            iprintf("Connected to: ");
            ShowIP(client_addr);
            iprintf(":%d\n", port);

            // Send messages to client
            writestring(fdnet, "Welcome to the NetBurner SSL/TLS Server Example\r\n");
            fdprintf(fdnet, "You are connected to IP Address %I, port %d\r\n", SSL_GetSocketLocalAddr(fdnet), TCP_LISTEN_PORT);

            while (fdnet > 0)
            {
                /* Loop while connection is valid. The read() function will return
                   0 or a negative number if the client closes the connection, so we
                   test the return value in the loop. Note: you can also use
                   ReadWithTimout() in place of read to enable the connection to
                   terminate after a period of inactivity.
                */
                int n = 0;
                do
                {
                    n = read(fdnet, RXBuffer, RX_BUFSIZE);
                    RXBuffer[n] = '\0';
                    iprintf("Read %d bytes: %s\n", n, RXBuffer);
                } while (n > 0);

                // Don't foreget to close !
                iprintf("Closing client connection: ");
                ShowIP(client_addr);
                iprintf(":%d\n", port);
                close(fdnet);
                fdnet = 0;
            }
        }   // while(1)
    }       // while listen
}

/*-------------------------------------------------------------------
 User Main
 ------------------------------------------------------------------*/
void UserMain(void *pd)
{
    init();
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

    // Create TCP Server task
    OSTaskCreatewName(TcpServerTask, (void *)TCP_LISTEN_PORT, &TcpServerTaskStack[USER_TASK_STK_SIZE], TcpServerTaskStack, MAIN_PRIO - 1,
                      "TcpServerTask");   // higher priority than UserMain

    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND * 5);
    }
}
