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
#include <netinterface.h>
#include <system.h>

const char *AppName = "UDP Notify Example";

#define UDP_LISTEN_PORT 2121

/**
 *  UDP Notify Callback Function
 *  The callback function must be of type:
 *    typedef void  ( udp_data_notify )(OS_FIFO * pfifo, uint16_t port);
 *
 *  Whenever data is received on the FIFO specified in the
 *  RegisterUDPFifoWithNotify() this function will be called.
 **/
void UdpListenCallback( OS_FIFO * pfifo, uint16_t port )
{
    iprintf( "UdpListenCallback() \r\n" );

    // We construct a UDP packet object using the FIFO.
    // This constructor will only return when we have received a packet
    UDPPacket upkt( pfifo, 0 /* Replace this 0 with a tick count to have a time out delat */ );

    // Did we get a valid packet or just time out?
    if( upkt.Validate() )
    {
        uint16_t len = upkt.GetDataSize();
        iprintf( "\r\nReceived a UDP packet with %d bytes from : %I\r\n", (int)len, upkt.GetSourceAddress() );
        ShowData( upkt.GetDataBuffer(), len );
        iprintf( "\r\n" );
    }
    else
    {
        iprintf( "Invalid packet received.\r\n" );
    }
}


/**
 *  UserMain Task
 **/
void UserMain( void *pd )
{
    init();                 						// Initialize network stack
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);		// Wait for DHCP address

    iprintf( "Application: %s\r\nNNDK Revision: %s\r\n", AppName, GetReleaseTag() );

    // Delay here until we have officially received an address from the DHCP server
    InterfaceBlock* pifb = GetInterfaceBlock(); // Defaults to first interface
    if( pifb != nullptr )
    {
        while( pifb->dhcpClient.GetDHCPState() != SDHCP_CMPL )
        {
            OSTimeDly( 1 );
        }
    }

    // Display informational message
    iprintf( "Listening for UDP packets on %hI: %d\r\n", IPADDR4( pifb->ip4.cur_addr ), UDP_LISTEN_PORT );

    // Create FIFO to store incoming packets and initialize it
    OS_FIFO UdpListenFifo;

    // Register to listen for UDP packets on the specified port number, and
    // specify the FIFO and callback function to use when data is received.
    RegisterUDPFifoWithNotify( UDP_LISTEN_PORT, &UdpListenFifo, UdpListenCallback );

    while( 1 )
    {
        OSTimeDly( TICKS_PER_SECOND );
    }
}
