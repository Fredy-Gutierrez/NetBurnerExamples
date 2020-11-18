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

#include <basictypes.h>
#include <constants.h>
#include <init.h>
#include <multicast.h>
#include <nbrtos.h>
#include <nettypes.h>
#include <string.h>
#include <udp.h>
#include <utils.h>

const char *AppName = "Multicast Example";

// Make sure they're 4 byte aligned to keep the ColdFire happy
uint32_t MultiTestStk[USER_TASK_STK_SIZE] __attribute__((aligned(4)));

IPADDR4 MulticastIpAddr;
BOOL shutdown;

/*-------------------------------------------------------------------
 * Task to process incoming multicast packets
 *------------------------------------------------------------------*/
void MultiReaderMain(void *pd)
{
    int port = (int)pd;
    iprintf("Reading from port %d\r\n", port);

    OS_FIFO fifo;

    // Register to listen for Multi packets on port number 'port'
    RegisterMulticastFifo(MulticastIpAddr, port, &fifo);

    while (!shutdown)
    {
        // We construct a UDP packet object using the FIFO.
        // This constructor will only return when we have received a packet
        UDPPacket upkt(&fifo, TICKS_PER_SECOND);

        // Did we get a valid packet? or just time out?
        if (upkt.Validate())
        {
            uint16_t len = upkt.GetDataSize();
            iprintf("Recd UDP packet with %d Bytes From :", (int)len);
            ShowIP(upkt.GetSourceAddress());
            iprintf("\r\n");
            ShowData(upkt.GetDataBuffer(), len);
            iprintf("\r\n");
        }
    }
    iprintf("Unregistering from group\r\n");
    UnregisterMulticastFifo(MulticastIpAddr, port);
    iprintf("Done unregistering\r\n");
}

/*-------------------------------------------------------------------
 * UserMain
 *-----------------------------------------------------------------*/
void UserMain(void *pd)
{
    init();

    int portnum;
    char buffer[80];

    iprintf("Input the Multicast group port number: ");
    scanf("%d", &portnum);

    iprintf(
        "\r\nEnter the Multicast IP Address (valid administratively-scoped, local IPv4 addresses range from 239.0.0.0 to "
        "239.255.255.255): ");
    buffer[0] = 0;
    while (buffer[0] == 0)
    {
        fgets(buffer, 80, stdin);
    }
    MulticastIpAddr = AsciiToIp4(buffer);

    iprintf("\r\nListening/Sending on Port %d of Group:", portnum);
    ShowIP(MulticastIpAddr);
    iprintf("\r\n");

    // Create task to listen for incoming packets
    OSTaskCreatewName(MultiReaderMain, (void *)portnum, &MultiTestStk[USER_TASK_STK_SIZE], MultiTestStk, MAIN_PRIO - 1, "MultiReaderTask");

    while (1)
    {
        iprintf("Enter the text to send in the packet. (Empty string to unregister listener)\r\n");
        fgets(buffer, 80, stdin);
        if (strlen(buffer) == 0)
        {
            iprintf("Unregistering the listener\r\n");
            iprintf("You must reset the board to continue\r\n");
            shutdown = TRUE;
            while (1)
            {
                OSTimeDly(TICKS_PER_SECOND);
            }
        }
        else
        {
            iprintf("Sending %s on UDP port %d to IP Address ", buffer, portnum);
            ShowIP(MulticastIpAddr);
            {
                UDPPacket pkt;
                pkt.SetSourcePort(portnum);
                pkt.SetDestinationPort(portnum);
                pkt.AddData(buffer);
                pkt.AddDataByte(0);
                pkt.Send(MulticastIpAddr);
            }
            iprintf("\r\n");
        }
    }
}
