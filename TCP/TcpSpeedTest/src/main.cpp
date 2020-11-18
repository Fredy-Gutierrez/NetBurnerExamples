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

#include <predef.h>
#include <stdio.h>
#include <startnet.h>
#include <iosys.h>
#include <tcp.h>
#include <init.h>
#include <ipshow.h>

// #define ADD_WIFITEST      // uncomment to add wifi interface

#ifdef ADD_WIFITEST
#include <wifi/wifi.h>
#include <wifi/wifiDriver.h>
#endif

#define LISTEN_PORT_NUMBER (1234)
#define BUFFER_SIZE 20000

const char * AppName="TCP Speed Test";

char   DataBuffer[BUFFER_SIZE]__attribute__(( aligned( 16 )));
bool   bVerbose = FALSE;

extern uint16_t tx_buf_frame_cnt;

#ifdef ADD_WIFITEST
int initWifiInterface()
{
#ifndef FAST_WIFI_STACK
      iprintf("Note: for improved WiFi performance, enable FAST_WIFI_STACK in \\nburn\\include\\constants.h\r\n");
#endif

    // Scan for available networks and print the results on stdout
    WifiInitScanAndShow_SPI();

    iprintf("Connecting......");
    int ifnumWifi = InitWifi_SPI();
    // NB is the NetBurner  namespace, and Wifi is the name of the class within the namespace.
    // We need to obtain a pointer to the already created Wifi object so we can call the object's
    // member functions.
    NB::Wifi *pNBWifiObject;
    pNBWifiObject = NB::Wifi::GetDriverByInterfaceNumber( ifnumWifi );

    bool connected = pNBWifiObject->Connected();

    if ( connected )
        iprintf("Success\r\n");
    else
        iprintf("Failed\r\n");

    return (connected == TRUE);
}
#endif



/*-------------------------------------------------------------------
 * UserMain
 * ----------------------------------------------------------------*/
void UserMain(void * pd)
{
    init();
    WaitForActiveNetwork(TICKS_PER_SECOND * 10);
    showIpAddresses();


#ifdef ADD_WIFITEST
    initWifiInterface();
#endif

    // Init data buffer array
    for (int32_t i = 0; i < BUFFER_SIZE; i++)
        DataBuffer[i] = ('A' + (i % 64));

    iprintf("Listening on port %d ... ", LISTEN_PORT_NUMBER);
    int32_t fdListen = listen( INADDR_ANY, LISTEN_PORT_NUMBER, 5 );
    if (fdListen > 0)
        iprintf("complete, fd = %d\r\n", fdListen);
    else
        iprintf("FAILED\r\n");

    if (fdListen > 0)
    {
        while (1)
        {
            iprintf("Waiting for incoming connection ... ");
            IPADDR clientIp;
            uint16_t clientPort;
            int32_t fda = accept(fdListen, &clientIp, &clientPort, 0);
            if (fda > 0)
            {
                iprintf("Connected to: ");
                ShowIP(clientIp);
                iprintf(":%d\n", clientPort);

                iprintf("Setting TCP socket options to NOPUSH and ack buffers to 40\r\n");
                setsockoption(fda, SO_NOPUSH);
                setsocketackbuffers(fda, 20);
                SetSocketTxBuffers(fda, 20);

                // Block to read first byte from client, indicates start of test
                iprintf("Waiting for Client to signal start of test\r\n");
                //read( fda, DataBuffer, 1 );
                int32_t bytesRead = ReadWithTimeout(fda, DataBuffer, 1, TICKS_PER_SECOND * 10);
                if (bytesRead <= 0)
                {
                    iprintf("No start signal received within 10 seconds, aborting test\r\n");
                    break;
                }
                else
                {
                    iprintf("Received start signal\r\n");
                }

                uint32_t timeTickStart, timeTickEnd;
                uint32_t BytesSent = 0;

                timeTickStart = TimeTick;
                while (1)
                {
                    int32_t rv = write(fda, DataBuffer, 1460);
                    if (rv < 0)
                    {
                        timeTickEnd = TimeTick;
                        close(fda);
                        uint32_t numTimeTicks = timeTickEnd - timeTickStart;
                        printf("Bytes Sent = %lu, TimeTick count: %lu (approx %0.2f seconds)\r\n", BytesSent, numTimeTicks,
                                (float) numTimeTicks / (float) TICKS_PER_SECOND);
                        break;
                    }
                    else
                    {
                        BytesSent += rv;
                        if (bVerbose)
                            iprintf("wrote: %d, total: %lu\r\n", rv, BytesSent);
                    }
                }
            }
            else
                iprintf("accept() failed\r\n");
        }
    }

    iprintf("Program terminated. Reset device to repeat\r\n");
    while (1)
        ;
}

