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
#include <nbrtos.h>
#include <system.h>
#include <websockets.h>
#include <iosys.h>



const char * AppName="WebsocketEcho";
extern http_wshandler *TheWSHandler;
int ws_fd = -1;
OS_SEM waitingForWS;
fd_set read_fds;
fd_set error_fds;

int MyDoWSUpgrade( HTTP_Request *req, int sock, PSTR url, PSTR rxb )
{
    if (httpstricmp(url, "/ECHO")) {
        if (ws_fd < 0) {
            int rv = WSUpgrade( req, sock );
            if (rv >= 0) {
                ws_fd = rv;
                NB::WebSocket::ws_setoption(ws_fd, WS_SO_TEXT);
                waitingForWS.Post();
                return 2;
            }
            else {
                return 0;
            }
        }
        else {
            NotAvailableResponse(sock, url);
            return 0;
        }
    }
    NotFoundResponse( sock, url );
    return 0;
}


void UserMain(void * pd)
{

    init();                                       // Initialize network stack
    StartHttp();                                  // Start web server, default port 80
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

    FD_ZERO( &read_fds );
    FD_ZERO( &error_fds );

    iprintf("Application started\n");
    SMPoolPtr pp;
    TheWSHandler = MyDoWSUpgrade;
    while (1)
    {
        waitingForWS.Pend( 0 );

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
                n = writeall( ws_fd, (char *)pp->pData, n );
                NB::WebSocket::ws_flush( ws_fd );
            }
            FD_ZERO( &read_fds );
            FD_ZERO( &error_fds );
        }
        OSTimeDly(TICKS_PER_SECOND * 1);
    }
}
