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
#include <iointernal.h>
#include <iosys.h>
#include <utils.h>
#include <tcp.h>
#include <nbrtos.h>
#include <serial.h>
#include <serinternal.h>
#include <stdlib.h>
#include <string.h>

#include <sim.h>
#include <ethervars.h>
#include <dhcpd.h>
#include <base64.h>


#define UART_MAP_COUNT 10

const char *AppName = "WebSocketConsole";
extern http_wshandler *TheWSHandler;
int ws_fd = -1;
OS_SEM waitingForWS;
fd_set read_fds;
fd_set error_fds;
int stdioList[3];

extern DHCP::Server MyServer;


struct UartSockMapping {
    int uart;
    int sock;
};

UartSockMapping UartMap[UART_MAP_COUNT];

struct UartConfig {
    int num;
    int32_t baud;
    int data_bits;
    int parity;
    int stop_bits;
};

void BadRequestResponse( int sock, PCSTR url, PCSTR data );
void NotAvailableResponse( int sock, PCSTR url );

using namespace NB;

static inline const char * MovePastQMark(const char *url)
{
    const char *backup = url;
    while (*url && (*url != '?')) { url++; }
    if (*url == '?') { return url+1; }

    return backup;
}

int httpstricmp( PCSTR s1, PCSTR sisupper2 )
{
   while ( (*s1) && (*sisupper2) && (toupper( *s1 ) == toupper( *sisupper2 )) )
   {
      s1++;
      sisupper2++;
   }
   if ( (*s1) && (*sisupper2) )
   {
      return 0;
   }
   return 1;
}

int ParseQueryString( const char *query, const char *url, char *buf, int maxlen )
{
     url = MovePastQMark(url);

     bool foundAmp = true;
     while (*url) {
         if (foundAmp && (*query == *url) && httpstricmp(url, query)) {
             while (*url && (*url != '=') && (*url!='&')) { url++; }

             if (*url == '=') {
                 ++url;
                 const char *start = url;
                 while (*url && (*url != '=') && (*url!='&')) { url++; }
                 int cpylen = ((url - start) > (maxlen - 1))
                     ? (maxlen - 1)
                     : (url - start);
                 memcpy( buf, start, cpylen );
                 return cpylen;
             }
             else if (*url == '&') {
                 return 0;
             }
             else {
                 return -1;
             }
         }
         url++;
         if (*url == '=') {
             foundAmp = false;
         }
         else if (*url == '&') {
             foundAmp = true;
         }
     }
     return -1;
}

bool ParseUartConfig( HTTP_Request *req,    int sock,
                        PSTR url,           PSTR rxb,
                        UartConfig &uart
                    )
{
    char buf[80];
    int rv = ParseQueryString("uart", url, buf, 80);
    if (rv != 1) { return false; }
    uart.num = buf[0] - '0';
    if ((uart.num < 0) || (uart.num > (UART_MAP_COUNT-1))) {
        return false;
    }
    rv = ParseQueryString("baud", url, buf, 80);
    if (rv < 1) { return false; }
    uart.baud = atol(buf);
    if ((uart.baud < 0) || (uart.baud > 230400)) { return false; }

    rv = ParseQueryString("data_bits", url, buf, 80);
    if (rv != 1) { return false; }
    uart.data_bits = buf[0] - '0';
    if ((uart.data_bits < 5) || (uart.data_bits > 8)) { return false; }

    rv = ParseQueryString("parity", url, buf, 80);
    if (rv != 1) { return false; }
    uart.parity = buf[0] - '0';
    if ((uart.parity < 0) || (uart.parity > 2)) { return false; }

    rv = ParseQueryString("stop_bits", url, buf, 80);
    if (rv != 1) { return false; }
    uart.stop_bits = buf[0] - '0';
    if ((uart.stop_bits < 1) || (uart.stop_bits > 3)) { return false; }

    return true;
}

int MyDoWSUpgrade( HTTP_Request *req, int sock, PSTR url, PSTR rxb )
{
    if (httpstricmp(url, "/STDIO")) {
        if (ws_fd < 0) {
            int rv = WSUpgrade( req, sock );
            if (rv >= 0) {
                ws_fd = rv;
                NB::WebSocket::ws_setoption(ws_fd, WS_SO_TEXT);
                waitingForWS.Post();
                stdioList[0] = ReplaceStdio( 0, ws_fd );
                stdioList[1] = ReplaceStdio( 1, ws_fd );
                stdioList[2] = ReplaceStdio( 2, ws_fd );
                return 2;
            }
            else {
                return 0;
            }
        }
    }
    else if (httpstricmp(url, "/UART")) {
        UartConfig uart;
        if(!ParseUartConfig(req, sock, url, rxb, uart)) {
            BadRequestResponse( sock, url, req->pData );
            return 0;
        }
        if ((UartMap[uart.num].uart > -1) || (UartMap[uart.num].sock > -1)){
            NotAvailableResponse( sock, url );
            return 0;
        }
        SerialClose( uart.num );
        UartMap[uart.num].uart = OpenSerial( uart.num, uart.baud, uart.stop_bits,
                                        uart.data_bits, (parity_mode)uart.parity );
        if (UartMap[uart.num].uart < 0) {
            NotAvailableResponse( sock, url );
            return 0;
        }
        UartMap[uart.num].sock = WSUpgrade( req, sock );
        if (UartMap[uart.num].sock < 0) {
            SerialClose(uart.num);
            UartMap[uart.num].uart = -1;
            return 0;
        }
        NB::WebSocket::ws_setoption(UartMap[uart.num].sock, WS_SO_TEXT);
        return 2;
    }
    NotFoundResponse( sock, url );
    return 0;
}

// while (readAvail(uartfd) && writeAvail(wsfd) {
//  char c;
//  read(uartfd, &c, 1);
//  write(wsfd, &c, 1);
// }

int32_t spliceToWS(int num, int32_t len)
{
    if ((num < 0) || (num > 9)) {
        return -1;
    }
    SMPoolPtr pp;
    int32_t totalRead = 0;
    int n;
    WebSocket *ws = WebSocket::GetWebSocketRecord( UartMap[num].sock );
    if (ws == NULL) { return -1; }

    int maxWrite = ws->GetWriteSpace();
    if ((!dataavail(UartMap[num].uart)) || (maxWrite == 0)) {
        return 0;
    }

    len = (len > maxWrite) ? maxWrite : len;

    while ( (totalRead + ETHER_BUFFER_SIZE) < len) {
        n = read( UartMap[num].uart, (char *)pp->pData, ETHER_BUFFER_SIZE );
        totalRead += n;
        write( UartMap[num].sock, (char *)pp->pData, n );

        if (n < ETHER_BUFFER_SIZE) {
            return totalRead;
        }
    }
    n = read( UartMap[num].uart, (char *)pp->pData, len - totalRead );
    totalRead += n;
    write( UartMap[num].sock, (char *)pp->pData, n );
    return totalRead;
}

int32_t spliceToUart(int num, int32_t len)
{
    if ((num < 0) || (num > 9)) {
        return -1;
    }
    SMPoolPtr pp;
    int32_t totalRead = 0;
    int n;
    WebSocket *ws = WebSocket::GetWebSocketRecord( UartMap[num].sock );
    if (ws == NULL) { return -1; }

    int maxWrite = UartData[num].m_FifoWrite.SpaceAvail();
    if ((!dataavail(UartMap[num].sock)) || (maxWrite == 0)) {
        return 0;
    }

    len = (len > maxWrite) ? maxWrite : len;

    while ( (totalRead + ETHER_BUFFER_SIZE) < len) {
        n = read( UartMap[num].sock, (char *)pp->pData, ETHER_BUFFER_SIZE );
        totalRead += n;
        write( UartMap[num].uart, (char *)pp->pData, n );

        if (n < ETHER_BUFFER_SIZE) {
            return totalRead;
        }
    }
    n = read( UartMap[num].sock, (char *)pp->pData, len - totalRead );
    totalRead += n;
    write( UartMap[num].uart, (char *)pp->pData, n );
    return totalRead;
}

void TaskStatus( void *pd )
{
    while (1) {
        OSTimeDly(10*TICKS_PER_SECOND);
#ifdef NBRTOS_STACKCHECK
        OSDumpTasks();
#endif
        iprintf("Buffers: %d\n", GetFreeCount());
    }

}

#define TRNFR_BUF_SIZ 80
void UserMain( void * pd )
{
    for (int i = 0; i < UART_MAP_COUNT; i++) {
        UartMap[i].uart = -1;
        UartMap[i].sock = -1;
    }

    init();                                       // Initialize network stack
    StartHttp();                                  // Start web server, default port 80
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

    TheWSHandler = MyDoWSUpgrade;

    OSSimpleTaskCreatewName(TaskStatus, MAIN_PRIO-1,"WebSocket Console");

    iprintf("Application: %s\r\nNNDK Revision: %s\r\n",AppName,GetReleaseTag());
    int fd_count;
    while(1) {
        fd_count = 0;
        FD_ZERO( &error_fds );
        FD_ZERO( &read_fds );
        if (ws_fd > 0 ) {
            FD_SET( ws_fd, &error_fds );
            fd_count++;
        }

        for (int i = 0; i < UART_MAP_COUNT; i++) {
            if ((UartMap[i].uart != -1) && (UartMap[i].sock != -1)) {
                FD_SET( UartMap[i].uart, &read_fds );
                FD_SET( UartMap[i].uart, &error_fds );
                FD_SET( UartMap[i].sock, &read_fds );
                FD_SET( UartMap[i].sock, &error_fds );
                fd_count += 2;
            }
        }

        if ( select( fd_count, &read_fds, NULL, &error_fds, TICKS_PER_SECOND ) ) {
            if (FD_ISSET( ws_fd, &error_fds )) {
                ReplaceStdio( 0, stdioList[0] );
                ReplaceStdio( 1, stdioList[1] );
                ReplaceStdio( 2, stdioList[2] );
                close(ws_fd);
                ws_fd = -1;
                iprintf("WS error\r\n");
            }
            for (int i = 0; i < UART_MAP_COUNT; i++) {
                if (FD_ISSET( UartMap[i].uart, &error_fds)
                    || FD_ISSET( UartMap[i].sock, &error_fds)) {
                    close(UartMap[i].uart);
                    close(UartMap[i].sock);
                    UartMap[i].uart = -1;
                    UartMap[i].sock = -1;
                }
                else {
                    if (FD_ISSET( UartMap[i].uart, &read_fds)) {
                        while( spliceToWS(i, 0xFFFF) == 0xFFFF);
                    }
                    if (FD_ISSET( UartMap[i].sock, &read_fds)) {
                        while (spliceToUart( i, 0xFFFF) == 0xFFFF);
                    }
                }
            }

        }
        iprintf("Socket States:\n");
        iprintf("ws_fd: %d, stdio: %d\n",  ws_fd, CurrentStdioFD( 0 ));
        for (int i = 0; i < UART_MAP_COUNT; i++) {
            iprintf("Port: %d, uart_fd: %d, ws_fd: %d\n", i, UartMap[i].uart, UartMap[i].sock);
        }
        iprintf("\n");
    }
}
