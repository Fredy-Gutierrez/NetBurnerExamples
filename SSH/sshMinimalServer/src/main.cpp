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

/*-------------------------------------------------------------------
 * SSH Minimal Server Example
 * A SSH Server task is created that will listen for incoming
 * SSH connections on TCP port 22. This simple example uses the
 * SSH key stored in the NetBurner library.
 *
 * To use the example:
 * - Compile and load the application into your NetBurner device.
 * - Run MTTTY and connect to USB or Serial port to view status
 *   messages and send data to the SSH Client.
 * - Run a SSH Client like Putty, and connect to the NetBurner device.
 *
 *-----------------------------------------------------------------*/
#include <predef.h>
#include <ctype.h>
#include <stdio.h>
#include <init.h>
#include <netinterface.h>
#include <startnet.h>
#include <string.h>
#include <serial.h>
#include <tcp.h>
#include <nbssh/nbssh.h>

#define SSH_LISTEN_PORT 22       // server listen port number
#define CONNECTION_TIMEOUT 120   // connection timeout in seconds

#define BUFSIZE (4096)
char gRxBuffer[BUFSIZE];
char gTxBuffer[BUFSIZE];

int fdnet;

extern "C"
{
    void UserMain(void *pd);
}

const char *AppName = "SshMin";

/*-------------------------------------------------------------------
 * Display TCP Err state
 *-----------------------------------------------------------------*/
void DisplayTcpErrState(int code)
{
    switch (code)
    {
        case TCP_ERR_NORMAL: iprintf("TCP Closed\r\n"); break;
        case TCP_ERR_TIMEOUT: iprintf("TCP Timeout\r\n"); break;
        case TCP_ERR_NOCON: iprintf("TCP No Connection\r\n"); break;
        case TCP_ERR_CLOSING: iprintf("TCP Closing\r\n"); break;
        case TCP_ERR_NOSUCH_SOCKET: iprintf("TCP No such socket\r\n"); break;
        case TCP_ERR_NONE_AVAIL: iprintf("TCP No aviable free scoketd\r\n"); break;
        case TCP_ERR_CON_RESET: iprintf("TCP Reset\r\n"); break;
        case TCP_ERR_CON_ABORT: iprintf("TCP Abort\r\n"); break;
        case SSH_ERROR_FAILED_NEGOTIATION: iprintf("SSH FAILED NEGOTIATION\r\n"); break;
        case SSH_ERROR_FAILED_SESSION_FAILED: iprintf("SSH SESSION FAILURE\r\n"); break;
        default: iprintf("Unknown code: %d\r\n", code);
    }
}

/*-------------------------------------------------------------------
 Convert IP address to a string
 -------------------------------------------------------------------*/
void IPtoString(IPADDR ia, char *s)
{
    puint8_t ipb = (puint8_t)&ia;
    siprintf(s, "%d.%d.%d.%d", (int)ipb[0], (int)ipb[1], (int)ipb[2], (int)ipb[3]);
}

/*-------------------------------------------------------------------
 * SSH Server Task
 * Listen for incoming SSH Client connections.
 * ------------------------------------------------------------------*/
// Allocate task stack for UDP listen task
#define SSHSERVER_TASK_STACK_SIZE (USER_TASK_STK_SIZE * 4)
uint32_t SshServerTaskStack[SSHSERVER_TASK_STACK_SIZE];

void SshServerTask(void *pd)
{
    // The listen port is passed to the task when it is created.
    int ListenPort = (int)pd;

    // Set up the listening TCP socket
    int fdListen = listen(INADDR_ANY, ListenPort, 5);

    if (fdListen > 0)
    {
        IPADDR client_addr;
        uint16_t port;

        while (1)
        {
            // The SSL_accept() function will block until a TCP client requests
            // a connection. Once a client connection is accepted, the
            // file descriptor fdnet is used to read/write to it.
            iprintf("\r\nWaiting for connection on port %d...\n", ListenPort);
            fdnet = SshAccept(fdListen, &client_addr, &port, TICKS_PER_SECOND * 300);

            if (fdnet < 0)
            {
                iprintf("SshAccept() timeout: ");
                DisplayTcpErrState(fdnet);
                iprintf("\r\n");
            }
            else
            {
                iprintf("Connected to: ");
                ShowIP(client_addr);
                iprintf(":%d\n", port);

                writestring(fdnet, "Welcome to the NetBurner SSH Server\r\n");

                char s[20];
                IPtoString(InterfaceIP(GetFirstInterface()), s);
                sniprintf(gTxBuffer, BUFSIZE, "You are connected to IP Address %s, port %d\r\n", s, SSH_LISTEN_PORT);
                writestring(fdnet, gTxBuffer);

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
                        n = ReadWithTimeout(fdnet, gRxBuffer, BUFSIZE - 1, TICKS_PER_SECOND * CONNECTION_TIMEOUT);
                        if (n > 0)
                        {
                            gRxBuffer[n] = '\0';
                            iprintf("%s", gRxBuffer);
                        }
                        else if (n == TCP_ERR_TIMEOUT)   // If the client closes the connection, ReadWithTimeout() will time out
                        {
                            iprintf("TCP ReadWithTimout() Error %d: ", n);
                            DisplayTcpErrState(n);
                        }
                    } while (n > 0);

                    iprintf("Closing client connection: ");
                    ShowIP(client_addr);
                    iprintf(":%d\n", port);
                    close(fdnet);
                    fdnet = 0;
                }
            }
        }   // while(1)
    }       // if fdListen > 0
    else
    {
        iprintf("Error: could not open listen port in SslSocketServerTask\r\n");
    }
}

/*-------------------------------------------------------------------
 * UserMain
 *------------------------------------------------------------------*/
void UserMain(void *pd)
{
    init();
    SerialClose(0);
    int fd = OpenSerial(0, 115200, 1, 8, eParityNone);
    ReplaceStdio(0, fd);
    ReplaceStdio(1, fd);
    ReplaceStdio(2, fd);

    /*
     *   SSH task priority
     *
     *      The SSH server will create by default a task at priority SSH_TASK_PRIORITY
     *      This can be changed by calling SshSetTaskPriority with another priority
     *      greater than MAIN_PRIO and less than 63.
     *
     *      Priorities used the library are defined in constants.h
     *
     */
    if (SshSetTaskPriority(SSH_TASK_PRIORITY) == FALSE) { iprintf("SSH task priority not changed\r\n"); }
    // Create TCP Server task
    iprintf("Starting SSH Server Task, Listening for SSH connection on port %d\r\n", SSH_LISTEN_PORT);
    int status = OSTaskCreatewName(SshServerTask, (void *)SSH_LISTEN_PORT, &SshServerTaskStack[SSHSERVER_TASK_STACK_SIZE],
                                   SshServerTaskStack, MAIN_PRIO - 1, "SshServer");   // higher priority than UserMain

    if (status != OS_NO_ERR) iprintf("*** Error creating SSH Server Task: %d\r\n", status);

    iprintf("Type an ascii string followed by a <CR> to send\r\n");
    while (1)
    {
        char buf[90];

        fgets(buf, 80, stdin);   // will block until it gets user input from keyboard
        if (fdnet > 0)
        {
            writestring(fdnet, buf);
            iprintf("wrote: \"%s\"\r\n", buf);
        }
    }
}
