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
#include <stdlib.h>
#include <system.h>
#include <udp.h>
#include <utils.h>

const char *AppName = "UDP Send/Receive Example";

// Allocate stack space for the listen task
uint32_t   UdpReceiveTaskStack[USER_TASK_STK_SIZE];

/**
 *  This UDP task will wait for incoming packets on the specified port number,
 *  which is passed as a OSTaskCreate() void * parameter.
 **/
void UdpReceiveTask(void *pd)
{
    OS_FIFO fifo;   // Create a FIFO for the UDP packet and initialize it
    int listenPort = (int)pd;

    iprintf("Listening on UDP port: %d\r\n", listenPort);

    // Register the OS_FIFO. Received packets will be stored in the OS_FIFO.
    RegisterUDPFifo(listenPort, &fifo);

    while (1)
    {
        // Construct a UDP packet object using the previously declared FIFO.
        // The UDP constructor will block until a packet has been received.
        // The second parameter is a timeout value (time in ticks). A value of 0 will
        // wait forever.
        UDPPacket upkt(&fifo, 0 * TICKS_PER_SECOND);

        // Did we get a valid packet, or just time out?
        if (upkt.Validate())
        {
            uint16_t len = upkt.GetDataSize();
            iprintf("Received UDP packet with %d bytes from: %I\r\n", (int)len, upkt.GetSourceAddress());
            ShowData(upkt.GetDataBuffer(), len);   // hex dump function
            iprintf("\r\n");
        }
    }
}

/**
 *  UserMain
 */
void UserMain(void *pd)
{
    int     portNumber;
    IPADDR  destIpAddress;
    char    buffer[80];

    init();                 						// Initialize network stack
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);		// Wait for DHCP address

    iprintf("Application: %s\r\nNNDK Revision: %s\r\n", AppName, GetReleaseTag());
    iprintf("Enter the UDP port number (will be used for send & receive): ");
    fgets(buffer,80,stdin);
    portNumber = atoi(buffer);

    iprintf("\r\nEnter the destination IP Address: ");
    fgets(buffer,80,stdin);
    destIpAddress = AsciiToIp(buffer);

    iprintf("Listening/Sending on UDP Port %d, Sending to IP address: %I\r\n", portNumber, destIpAddress);

    // Create UDP listen task
    //OSSimpleTaskCreatewName(UdpReaderMain, MAIN_PRIO - 1, "UdpReader");
    OSTaskCreatewName( UdpReceiveTask,
                       (void  *)portNumber,
                       &UdpReceiveTaskStack[USER_TASK_STK_SIZE] ,
                       UdpReceiveTaskStack,
                       MAIN_PRIO - 1,   // higher priority than UserMain
                       "UDP Receive" );

    // while loop to process user input and send as a UDP packet
    iprintf("Enter data and hit return to send.\r\n");
    while (1)
    {
        fgets(buffer,80,stdin);
        iprintf("Sent \"%s\" to %I:%d\r\n", buffer, destIpAddress, portNumber);

        UDPPacket pkt;
        pkt.SetSourcePort(portNumber);
        pkt.SetDestinationPort(portNumber);
        pkt.AddData(buffer);
        pkt.AddDataByte(0);
        pkt.Send(destIpAddress);
        iprintf("\r\n");
    }
}
