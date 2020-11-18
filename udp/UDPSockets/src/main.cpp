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
#include <string.h>
#include <system.h>
#include <udp.h>
#include <utils.h>
#include <stdio.h>

const char *AppName = "UDP Sockets Example";

// Allocate stack space for the listen task
uint32_t   UdpReceiveTaskStack[USER_TASK_STK_SIZE];


/**
 *  This task will wait for incoming UDP packets and process them.
 **/
void UdpReceiveTask(void *pd)
{
    int listenPort = (int)pd;

    iprintf("UdpReceiveTask monitoring port %d\r\n", listenPort);

    // Create a UDP socket for receiving
    int udpFd = CreateRxUdpSocket(listenPort);
    if (udpFd <= 0)
    {
        iprintf("Error Creating UDP Listen Socket: %d\r\n", udpFd);
        while (1)
        {
            OSTimeDly(TICKS_PER_SECOND);
        }
    }
    else
    {
        iprintf("Listening for UDP packets on port %d\r\n", listenPort);
    }

    while (1)
    {
        IPADDR sourceIpAddress;     // UDP packet source IP address
        uint16_t localPort;         // Port number UDP packet was sent to
        uint16_t sourcePort;        // UDP packet source port number
        char buffer[80];

        int len = recvfrom(udpFd, (uint8_t *)buffer, 80, &sourceIpAddress, &localPort, &sourcePort);
        buffer[len] = '\0';

        iprintf("\r\nReceived a UDP packet with %d bytes from : %I\r\n%s\r\n", len, sourceIpAddress, buffer);
    }
}

/**
 *  UserMain
 **/
void UserMain(void *pd)
{
    int     portNumber;
    IPADDR  destIpAddress;
    char    buffer[80];

    init();                 						// Initialize network stack
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);		// Wait for DHCP address

    iprintf("Application: %s\r\nNNDK Revision: %s\r\n", AppName, GetReleaseTag());

    // Get destination IP address
    iprintf("Enter the UDP Server destination IP address: ");
    fgets(buffer,80,stdin);
    destIpAddress = AsciiToIp(buffer);
    iprintf("\r\n");

    // Get the port number. This application uses the same port number for send and receive
    iprintf("Enter the source/destination port number: ");
    fgets(buffer,80,stdin);
    portNumber = atoi(buffer);
    iprintf("\r\n");

    // Create a UDP socket for sending
    int udpFd = CreateTxUdpSocket(destIpAddress, portNumber, portNumber);
    if (udpFd <= 0)
    {
        iprintf("Error Creating UDP Socket: %d\r\n", udpFd);
        while (1)
        {
            OSTimeDly(TICKS_PER_SECOND);
        }
    }
    else
    {
        iprintf("Sending/Receiving with host %I: %d\r\n", destIpAddress, portNumber);
    }

    // Create UDP receive task. The priority is higher than UserMain() so packets get processed as they are received
    OSTaskCreatewName( UdpReceiveTask,
                       (void  *)portNumber,
                       &UdpReceiveTaskStack[USER_TASK_STK_SIZE] ,
                       UdpReceiveTaskStack,
                       MAIN_PRIO - 1,   // higher priority than UserMain
                       "UDP Receive" );

    iprintf("Enter data and hit enter to send.\r\n");

    while (1)           // Loop forever displaying UDP data
    {
        fgets(buffer,80,stdin);
        iprintf("\r\n");
        iprintf("Sending \"%s\" using UDP to %I : %d\r\n", buffer, destIpAddress, portNumber);

        sendto(udpFd, (uint8_t *)buffer, strlen(buffer), destIpAddress, portNumber);
        iprintf("\r\n");
    }
}
