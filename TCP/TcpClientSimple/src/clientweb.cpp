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
 * This code implements the web page entries for the message,
 * destination IP address and destination port number. When the
 * web page first loads it will automatically fill in the IP
 * address from the source requesting the web page, because in
 * most cases it will also be the address of the TCP Server.
 * The web page is a form, and when a user presses the submit
 * button the SendMsg() function will open a TCP connection to
 * the server, send the message, and close the connection.
 * Any error messages will be sent to stdout and can be viewed
 * with MTTTY.
 *
 * A TCP server program must already be listening at the specified
 * IP address and port number for the message to be sent. A simple
 * TCP Server called TcpServerWin.exe is located in the
 * \nburn\pctools directory of your NetBurner installation.
 -------------------------------------------------------------------*/

#include <predef.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <http.h>
#include <iosys.h>
#include <utils.h>
#include <tcp.h>
#include <string.h>
#include <fdprintf.h>
#include <httppost.h>
#include "clientweb.h"

#define APP_VERSION "Version 1.2 9/1/2018"

#define MAX_MSG_SIZE 1024

int    gDestPort;
IPADDR gDestIp;
char   gMessage[MAX_MSG_SIZE];

/*-------------------------------------------------------------------
 Sends a message to the specified host.
 ------------------------------------------------------------------*/
void SendMsg( IPADDR destIp, int destPort, char *msg )
{
    iprintf( "Connecting to: %I:%d\r\n", destIp, destPort );
    int fd = connect( destIp, destPort, TICKS_PER_SECOND * 5 );
    if (fd < 0)
    {
        iprintf("Error: Connection failed, return code: %d\r\n", fd);
    }
    else
    {
        iprintf( "Attempting to write: \"%s\" \r\n", msg );
        int n = write( fd, msg, strlen(msg) );
        iprintf( "Wrote %d bytes\r\n", n );
        close( fd );
    }
}


/*-------------------------------------------------------------------
 * Show destination port number on web page
 * ----------------------------------------------------------------*/
void WebDestPort(int sock, PCSTR url)
{
    if ( gDestPort == 0 )      // If no dest port is specified, use a default
        gDestPort = 2000;
    fdprintf(sock, "VALUE=\"%d\" ", gDestPort);
}

/*-------------------------------------------------------------------
 Show destination ip address on web page
 ------------------------------------------------------------------*/
void WebDestIp(int sock, PCSTR url)
{
    if ( gDestIp.IsNull() )     // If no dest ip address has been entered, use the one that requested the web page.
        fdprintf( sock, "VALUE=\"%I\" ", GetSocketRemoteAddr(sock) );
    else
        fdprintf( sock, "VALUE=\"%I\" ", gDestIp );
}

/*-------------------------------------------------------------------
 * Show destination ip address on web page
 *------------------------------------------------------------------*/
void WebShowClientIp(int sock, PCSTR url)
{
    fdprintf( sock, "%I", GetSocketRemoteAddr(sock));
}


/*-------------------------------------------------------------------
 * Parse form post variables.
 *------------------------------------------------------------------*/
void processPostVariables( const char *pName, const char *pValue )
{
    iprintf("Processing: %s\r\n", pName);

    if( strcmp(pName, "tfDestPortNum") == 0 )
    {
        gDestPort = (uint16_t)atoi( pValue );
        iprintf( "Destination port set to: %d\r\n", gDestPort );
    }
    else if( strcmp( pName, "tfDestIpAddr") == 0 )
    {
        iprintf( "DestIpAddr set to: %s\r\n", pValue );
        gDestIp = AsciiToIp(pValue);
    }
    else if( strcmp( pName, "tfMessage") == 0 )
    {
        strncpy(gMessage, pValue, MAX_MSG_SIZE - 1);
    }
    else if( strcmp( pName, "SendMessage") == 0 )
    {
        SendMsg( gDestIp, gDestPort, (char *)pValue);
    }
    else
    {
        iprintf("Error processing %s\r\n", pName);
    }
}


/*-------------------------------------------------------------------
 * Form post callback function
 *------------------------------------------------------------------*/
void PostCallBack(int sock, PostEvents event, const char * pName, const char * pValue)
{
    // Received a call back with an event, check for event type
    switch (event)
    {
    case eStartingPost:     // Called at the beginning of the post before any data is sent
        break;

    case eVariable:     // Called once for each variable in the form
        processPostVariables(pName, pValue);
        break;

    //Called back with a file handle if the post had a file
    case eFile:
        break; //No file type here so we do nothing

    // Called back when the post is complete. You should send your response here.
    case eEndOfPost:
        {
            iprintf("\r\n\r\n");    // just some new lines for easier console reading
            RedirectResponse(sock, "INDEX.HTML");  // Our response is to redirect to the index page
        }
        break;

    } //Switch
}


// Create a global static post handeling object that responds to the specified html page.
// A separate post handler can be created for each form in your application.
HtmlPostVariableListCallback poster("index.html",PostCallBack);








