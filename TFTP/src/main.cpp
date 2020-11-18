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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utils.h>
#include <tftp.h>

/*
 * In this example we are specifying an IP address for the TFTP server. You can add a
 * configurable sever address that will be stored in application data flash memory
 * by adding a configuration parameter with a function call such as:
 *    static config_string gTftpServerName{appdata, "DNS_or_IPAdrress", "TFTP_SERVER"};
 * or
 *    static config_IPADDR4 gTftpServerAddress (appdata, "10.1.1.123", "TFTP_SERVER);
 */
#define TFTP_SERVER_IP "10.1.1.100"

const char *AppName = "TFTP Example Program";

const int       bufferSize = 1024;
unsigned char   tftpBuffer[bufferSize];


/**
 *  TFTP read test
 */
void ReadTFTP_Test()
{
    int len = bufferSize;

    switch( GetTFTP( "tftp.txt",            // file name
                     "b",                   // mode = "b" for binary, or "t" for text
                     tftpBuffer,            // input buffer to store data
                     len,                   // maximum length of data to receive
                     TICKS_PER_SECOND * 10, // Timeout in system Time Ticks
                     AsciiToIp4(TFTP_SERVER_IP)         // IP address of TFTP Server
                   ) )
    {
        case TFTP_OK:
        {
            iprintf( "Got %d bytes\n", len );
            ShowData( tftpBuffer, len );
            break;
        }

        case TFTP_TIMEOUT:
        {
            iprintf( "TFTP timeout\n" );
            break;
        }

        case TFTP_ERROR:
        {
            iprintf( "Error with %d bytes\n", len );
            ShowData( tftpBuffer, len );
            break;
        }

        default:
        {
            iprintf( "Unknown TFTP return code " );
            break;
        }
    }
    iprintf( "\n\n" );
}


/**
 *  Test TFTP send
 */
void SendTFTP_Test()
{
    char buf[ 80 ];

    tftpBuffer[ 0 ] = 0;
    for( int i = 0; i < 10; i++ )
    {
        sniprintf( buf, 80, " File line number: %d\r\n", i );
        strcat( (char *)tftpBuffer, buf );
    }

    int len = strlen( (char *)tftpBuffer );
    iprintf( "Sending %d bytes\n", len );

    switch( SendTFTP( "tftp.txt",           // name of file to send
                      "b",                  // mode = "b" for binary or "t" for text
                      tftpBuffer,           // pointer to data to send
                      len,                  // length of data to send
                      TICKS_PER_SECOND * 10,// Operation timeout value
                      TICKS_PER_SECOND * 5, // Individual packet timeout
                      AsciiToIp4(TFTP_SERVER_IP)
                     ) )
    {
        case TFTP_OK:
        {
            iprintf( "Send OK\n" );
            break;
        }

        case TFTP_TIMEOUT:
        {
            iprintf( "TFTP timeout\n" );
            break;
        }

        case TFTP_ERROR:
        {
            iprintf( "Error \n" );
            break;
        }

        default:
        {
            iprintf( "Unknown TFTP return code " );
            break;
        }
    }
    iprintf( "\n\n" );
}


/**
 *  UserMain
 *
 *  Main entry point of example
 */
void UserMain(void *pd)
{
    init();                                       // Initialize network stack
    StartHttp();                                  // Start web server, default port 80
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

    iprintf("TFTP Example Application Started\r\n");

    iprintf("Sending file to TFTP Server:\r\n");
    SendTFTP_Test();
    iprintf("\r\n");

    iprintf("Receiving file from TFTP Server:\r\n");
    ReadTFTP_Test();
    iprintf("\r\n");

    iprintf("End of program. Reset device to repeat test\r\n");

    while (1)
    {
        OSTimeDly( TICKS_PER_SECOND );
    }
}
