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
#include <startnet.h>
#include <stdlib.h>
#include <udp.h>

const char *AppName = "UDP C++ Packet Class Example";

// Send/Receive port number
int gPortNum = 0;

/**
 *  This task will wait for incoming UDP packets and process them.
 **/
void UdpReaderTask( void *pd )
{
    iprintf( "UdpReaderTask monitoring port %d\r\n", gPortNum );

    // Create FIFO to store incoming packets and initialize it
    OS_FIFO fifo;

    // Register to listen for UDP packets on port number 'port'
    RegisterUDPFifo( gPortNum, &fifo );

    while( 1 )
    {
        // We construct a UDP packet object using the FIFO.
        // This constructor will only return when we have received a packet.
        UDPPacket upkt( &fifo, 0 ); // Replace this 0 with a tick count to have a time out delay

        // Did we get a valid packet or just time out?
        if( upkt.Validate() )
        {
            uint16_t len = upkt.GetDataSize();
            iprintf( "\r\nReceived a UDP packet with %d bytes from: %I\r\n", (int)len, upkt.GetSourceAddress() );
            ShowData( upkt.GetDataBuffer(), len );
            iprintf( "\r\n" );
        }
    }
}

/**
 *  UserMain Task
 *  This is the first task to be executed and will create the UDP
 *  Reader Task.
 **/
void UserMain( void *pd )
{
    init();                 						// Initialize network stack
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);		// Wait for DHCP address

    iprintf( "Application: %s\r\nNNDK Revision: %s\r\n", AppName, GetReleaseTag() );

    IPADDR ipaddr;
    char buffer[ 80 ];

    // Get destination IP address
    iprintf( "Enter the destination IP address: " );
    fgets( buffer, 80, stdin );
    ipaddr = AsciiToIp( buffer );

    // Get port number. This application uses the same port number for TX and RX
    iprintf( "\r\n" );
    iprintf( "Enter the source/destination port number: " );
    fgets( buffer, 80, stdin );
    gPortNum = atoi( buffer );

    // Display informational message
    iprintf( "\r\n" );
    iprintf( "Sending/Receiving with host %I : %d\r\n", ipaddr, gPortNum );

    // Create task to receive UDP packets. We will set the
    // priority to 1 less than the UserMain priority so packets get
    // processed as they are received.
    OSSimpleTaskCreatewName( UdpReaderTask, MAIN_PRIO - 1, "UdpReader" );

    // Loop forever displaying UDP data
    iprintf( "Enter data and hit return to send.\r\n" );
    while( 1 )
    {
        fgets( buffer, 80, stdin );
        iprintf( "\r\n" );
        iprintf( "Sending \"%s\" using UDP to %I : %d\r\n", buffer, ipaddr, gPortNum );

        UDPPacket pkt;
        pkt.SetSourcePort( gPortNum );
        pkt.SetDestinationPort( gPortNum );
        pkt.AddData( buffer );
        pkt.AddDataByte( 0 );
        pkt.Send( ipaddr );

        iprintf( "\r\n" );
    };
}
