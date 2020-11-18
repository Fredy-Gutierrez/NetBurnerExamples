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
#include <init.h>
#include <stdio.h>
#include <ctype.h>
#include <startnet.h>
#include <websockets.h>
#include <stdlib.h>
#include <nbrtos.h>

#include <sim.h>

#define SERVER_IP "192.168.1.209"



const char * AppName="Connect";
int ws_fd = -1;
OS_SEM wsSem;

void UserTask(void * pd)
{
    SMPoolPtr pp;
    fd_set read_fds;
    fd_set error_fds;

    FD_ZERO( &read_fds );
    FD_ZERO( &error_fds );

    while (ws_fd > 0) {
        FD_SET(ws_fd, &read_fds);
        FD_SET(ws_fd, &error_fds);
        select(1, &read_fds, NULL, &error_fds, 0);
        if (FD_ISSET(ws_fd, &error_fds)) {
            iprintf("Closing Socket\r\n");
            OSTimeDly(1);
            close(ws_fd);
            ws_fd = -1;
            iprintf("Socket Closed\r\n");
            break;
        }
        if (FD_ISSET(ws_fd, &read_fds)) {
            int n = read( ws_fd, (char *)pp->pData, ETHER_BUFFER_SIZE );
            n = writeall( 1, (char *)pp->pData, n );
            NB::WebSocket::ws_flush( ws_fd );
        }
        FD_ZERO( &read_fds );
        FD_ZERO( &error_fds );
    }


}

void UserMain(void * pd)
{

    init();                                       // Initialize network stack
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

    iprintf("Application started\n");

    ws_fd = NB::WebSocket::Connect(SERVER_IP, "/", 80);
    iprintf("ws_fd: %d\r\n", ws_fd);

    OSSimpleTaskCreatewName( UserTask, MAIN_PRIO+1, "SocketTask" );

    char buf[40];
    while (1)
    {
        sniprintf(buf, 40, "TimeTick: %lu\r\n", TimeTick);
        writestring(ws_fd, buf);
        OSTimeDly(TICKS_PER_SECOND * 1);
    }
}
