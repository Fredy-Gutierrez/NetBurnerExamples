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
#include <iosys.h>
#include <nbrtos.h>
#include <netinterface.h>
#include <serial.h>
#include <udp.h>
#include <utils.h>
#include <hal.h>
#include "webif.h"

#define CLIENT_WRITE_BUF_SIZE (1024)
#define DATA_SERIAL_PORT 1
#define DEBUG_SERIAL_PORT 0
#define DATA_BAUDRATE 115200
#define DEBUG_BAUDRATE 115200
#define STOP_BITS 1
#define DATA_BITS 8
#define DEBUG false

//----- function prototypes -----
extern void CheckNVSettings();
extern bool gChangedLocalPort;
extern bool gChangedDest;
extern struct NV_SettingsStruct gNvSettings;

//----- Global Vars -----
const char *AppName = "UDP to Serial Example";
char gWriteBuffer[CLIENT_WRITE_BUF_SIZE];
int gFdSerial, gFdDebug;
int gLocalPort, gDestPort;
IPADDR gIpaddr;

/**
 *  Initialize serial ports.  This occurs when the system boots, and
 *  when changed in the web page configuration.
 **/
void InitializeSerialPorts(uint32_t DataBaudRate, uint32_t DebugBaudRate)
{
    // Open the data serial port
    SerialClose(DATA_SERIAL_PORT);
    gFdSerial = OpenSerial(DATA_SERIAL_PORT, DataBaudRate, STOP_BITS, DATA_BITS, eParityNone);

    // Open the debug serial port
    if (DATA_SERIAL_PORT != DEBUG_SERIAL_PORT)
    {
        SerialClose(DEBUG_SERIAL_PORT);
        gFdDebug = OpenSerial(DEBUG_SERIAL_PORT, DebugBaudRate, STOP_BITS, DATA_BITS, eParityNone);
    }
    else
    {
        gFdDebug = gFdSerial;
    }

    iprintf("fdSerial = %d\r\n", gFdSerial);
    iprintf("fdDebug  = %d\r\n", gFdDebug);

    ReplaceStdio(0, gFdDebug);
    ReplaceStdio(1, gFdDebug);
    ReplaceStdio(2, gFdDebug);
}

/**
 *  Listen for incoming UDP packets
 */
void UdpReaderMain(void *pd)
{
    iprintf("Started listen UDP task on local port %d\r\n", gLocalPort);

    OS_FIFO fifo;

    // Register to listen for UDP packets on port number 'port'
    RegisterUDPFifo(gLocalPort, &fifo);

    while (1)
    {
        // Construct a UDP packet object using the FIFO.
        // This constructor will only return when we have received a UDP packet
        UDPPacket upkt(&fifo, TICKS_PER_SECOND);

        // Did we get a valid packet? or just time out?
        if (upkt.Validate())
        {
            uint16_t len = upkt.GetDataSize();
            iprintf("Received UDP packet with %d bytes from:", (int)len);
            ShowIP(upkt.GetSourceAddress());
            iprintf("\n");
            ShowData(upkt.GetDataBuffer(), len);
            iprintf("\n");

            iprintf("Writing UDP data to fdSerial...\r\n");
            writeall(gFdSerial, (const char *)upkt.GetDataBuffer(), len);
        }

        if (gChangedLocalPort)
        {
            UnregisterUDPFifo(gLocalPort);
            gLocalPort = gNvSettings.nLocalPort;
            RegisterUDPFifo(gLocalPort, &fifo);
            gChangedLocalPort = FALSE;
            iprintf("Changed gLocalPort to: %d", gLocalPort);
        }
    }
}

/**
 *  UserMain Task
 **/
void UserMain(void *pd)
{
    init();                 						// Initialize network stack
    StartHttp();									// Start web server, default port 80
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);		// Wait for DHCP address

    iprintf("Application: %s\r\nNNDK Revision: %s\r\n", AppName, GetReleaseTag());

    gChangedLocalPort = false;
    gChangedDest      = false;
    InitializeSerialPorts(115200, 115200);

    CheckNVSettings();

    gDestPort  = gNvSettings.nDestPort;                 // Destination UDP port
    gLocalPort = gNvSettings.nLocalPort;                // Local UDP port
    gIpaddr    = AsciiToIp(gNvSettings.szDestIpAddr);   // Destination UDP IP address

    iprintf("Listening on UDP Port %d, Sending serial data to %I port %d\r\n", gLocalPort, gIpaddr, gDestPort);

    OSSimpleTaskCreatewName(UdpReaderMain, MAIN_PRIO + 1, "UdpReader");

    while (1)
    {
        // Set up a file descriptor (FD) set so we can select() on the serial ports.
        // Once configured, the select() function will block until activity is
        // detected on a FD (a serial port in this example), or a timeout occurs
        // (set to 1 second here).  If a timeout does occur, we simply go back to the
        // select() function and wait for more activity.  The "write" and "error"
        // FD sets of the select() function are not used and set to nullptr; only
        // read operations caused by the items in read_fds will cause select()
        // to exit.  A timeout value of 0 would disable the timeout, and select()
        // would wait forever.
        //
        // Detailed description of how select() works:
        // The select() function operates on a bit mapped field of 64 bits.  In this
        // example, the bit field is read_fds.  When select() is called, read_fds
        // represents a MASK of which file descriptors to check.  AFTER select()
        // exits, read_fds contains the STATUS of which file descriptors have
        // pending events.  This means that read_fds must be reinitialized to
        // contain MASK information each time before select() is called.  FD_ZERO,
        // FD_SET, and FD_ISSET are macros that execute very fast.

        fd_set read_fds;
        FD_ZERO(&read_fds);             // zero read_fds set
        FD_SET(gFdSerial, &read_fds);   // set to look for serial activity
        FD_SET(gFdDebug, &read_fds);    // set to look for serial activity

        if (select(FD_SETSIZE, &read_fds, nullptr, nullptr, TICKS_PER_SECOND))
        {
            // Received input from serial port, send it to Ethernet port
            if (FD_ISSET(gFdSerial, &read_fds))
            {
                int n = read(gFdSerial, gWriteBuffer, CLIENT_WRITE_BUF_SIZE - 1);
                gWriteBuffer[n] = '\0';
                iprintf("Received %d bytes of serial data: %s\r\n", n, gWriteBuffer);
                iprintf("Sending serial data to: %I port %d\r\n", gIpaddr, gDestPort);

                UDPPacket pkt;
                pkt.SetSourcePort(gLocalPort);
                pkt.SetDestinationPort(gDestPort);
                pkt.AddData(gWriteBuffer);
                pkt.AddDataByte(0);
                pkt.Send(gIpaddr);
            }
        }
        else if (DEBUG)
        {
            iprintf("tick\r\n");
        }

        if (gChangedDest)   // Config changed in web page
        {
            gDestPort = gNvSettings.nDestPort;
            gIpaddr = AsciiToIp(gNvSettings.szDestIpAddr);
            gChangedDest = false;

            iprintf("Rebooting for destination changes to take effect.....\r\n");
            OSTimeDly(TICKS_PER_SECOND);
            ForceReboot();
        }
    }
}
