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
#include <system.h>
#include <udp.h>
#include <utils.h>

const char *AppName = "UDP Send/Receive Example";

// Allocate task stack for UDP listen task
static uint8_t UdpDataBuffer[MAX_UDPDATA];
static uint32_t PktCounter;

// Port number to send/receive on
int gPortNum = 1000;

void SendNext(IPADDR toip, uint16_t fPort, uint16_t tPort)
{
    // Fill the buffer with junk data
    if (PktCounter == 0)
    {
        for (int i = 0; i < MAX_UDPDATA; i++)
        {
            UdpDataBuffer[i] = (' ') + (i % 64);
        }
    }

    // Add the current counter to the beginning and end of the packet
    puint32_t pdw = (puint32_t)UdpDataBuffer;
    *pdw = PktCounter;
    pdw = (puint32_t)(UdpDataBuffer + (MAX_UDPDATA - 4));
    *pdw = PktCounter++;

    // Send the packet
    UDPPacket up;
    up.AddData(UdpDataBuffer, MAX_UDPDATA);
    up.SetSourcePort(fPort);
    up.SetDestinationPort(tPort);
    up.Send(toip);
}

/**
 *  UDP Server task will wait for incoming packets on the
 *  designated port number
 **/
void UdpReaderMain(void *pd)
{
    iprintf("Listening on UDP port: %d\r\n", gPortNum);

    // Create a FIFO for the UDP packet and initialize it
    OS_FIFO fifo;
    OSFifoInit(&fifo);

    // Register to listen for UDP packets on port number 'port'
    RegisterUDPFifo(gPortNum, &fifo);

    while (1)
    {
        // Construct a UDP packet object using the previously
        // declared FIFO. The UDP constructor will only return
        // when a packet has been received. The second parameter
        // is a timeout value (time in ticks). A value of 0 will
        // wait forever. The TICKS_PER_SECOND definition can
        // be used for code readability.
        UDPPacket upkt(&fifo, 0 * TICKS_PER_SECOND);

        // Did we get a valid packet, or just time out?
        if (upkt.Validate())
        {
            iprintf("Received packet from %I:%d...  Sending response.\r\n", upkt.GetSourceAddress(), upkt.GetSourcePort());
            SendNext(upkt.GetSourceAddress(), gPortNum, upkt.GetSourcePort());
        }
    }
}

/**
 *  UserMain
 *  This is the first task to be executed and will create the UDP
 *  Reader Task.
 **/
void UserMain(void *pd)
{
    init();                 						// Initialize network stack
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);		// Wait for DHCP address

    iprintf("Application: %s\r\nNNDK Revision: %s\r\n", AppName, GetReleaseTag());
    iprintf("Listening/Sending on UDP Port %d\r\n", gPortNum);

    // Create UDP listen task
    OSSimpleTaskCreatewName(UdpReaderMain, MAIN_PRIO - 1, "UdpReader");

    // Main while loop will take any user input and send it as a
    // UDP datagram.
    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND);
    }
}
