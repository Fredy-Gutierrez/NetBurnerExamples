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

/*
 * @file       main.cpp
 * @brief      Serial and Ethernet throughput testing.
 *
 * This application will repeatedly run a serial and Ethernet throughput test between two
 * devices.  To setup this application, do the following:
 *   1) Set the SERIAL_PORT value in Constants.h to reflect the port that the serial tests will
 *      be conducted over.
 *   2) Set gClientIp to the IP address of the device that will receive the connection request.
 *   3) Build and deploy the application to both devices.
 *
 * The application operates in the following fashion:
 *    1) The device that initiates the connection will be the first to send data.
 *    2) The devices will take turns receiving and sending data, then validating that all of the
 *       data was transmitted correctly.  Timing data, and session averages are displayed after
 *       ever test.  The order of the tests is as follows:
 *          Connection Initiator                Connection Receiver
 *          --------------------                -------------------
 *          1. Send Serial                      1. Receive Serial
 *          2. Receive Serial                   2. Send Serial
 *          3. Send Ethernet                    3. Receive Ethernet
 *          4. Receive Ethernet                 4. Send Ethernet
 *
 * To add additional connection tests, add the new test states to TestStates in Constants.h, add
 * the needed state change logic to AdvanceState() on the TestStateData object, and add the new
 * states in the switch statement located in UserMain().
 */

#include <autoupdate.h>
#include <ctype.h>
#include <dhcpclient.h>
#include <HiResTimer.h>
#include <netinterface.h>
#include <NetworkDebug.h>
#include <predef.h>
#include <serial.h>
#include <smarttrap.h>
#include <startnet.h>
#include <stdio.h>
#include <taskmon.h>
#include <tcp.h>

#include "Constants.h"
#include "TestStateData.h"

extern "C" {
    void UserMain(void * pd);
}

const char * AppName="SerialToEthernetThroughputTest";

bool gTestInitiator       = false;    // If true, this device initiates a connection to another device
int gFdSerial             = 0;        // File descriptor for serial connection
int gFdNet                = 0;        // File descriptor for Ethernet connection
TestStateData gTestData;              // This hold all of the data and test related information
HiResTimer * gHrTimer;

IPADDR gClientIp    = AsciiToIp4( "10.1.1.151" );  // Address of listening device
WORD   gClientPort  = 4001;                        // Port both devices will use for connection

/*
 * @brief Either listens for a connection, or initiates one.
 *
 * Whether a device listens or initiates the connection is decided by the IP address assigned to gClientIp.
 * If the device has the same IP address, it will listen, otherwise it will initiate the connection.
 */
void ManageTcpConnection()
{
    int FirstInterface  = GetFirstInterface();  // Get first interface identifier
    InterfaceBlock *ib  = GetInterFaceBlock(FirstInterface);  // Get interface data

    // If the IP address does not match the client IP given, then we are initiating the connection
    if( ib != nullptr && ib->netIP != gClientIp )
    {
        gTestInitiator = true;
        // Try to connect
        iprintf( "Trying connection...\n");
        gFdNet = connect( gClientIp, gClientPort, gClientPort, 0 );
        iprintf( "Connection accepted from ");
        ShowIP( gClientIp );
        iprintf( ":%d\r\n", gClientPort );
    }
    // Otherwise we are listening
    else
    {
        // If there is no active listen socket, create one.
        int fdListen = listen( INADDR_ANY, 4001, 1 );
        iprintf("Listening on port %d, fdListen = %d\r\n", 4001, fdListen );

        // Wait indefinitely for an incoming network connection
        iprintf( "Pending accept...");
        gFdNet = accept( fdListen, &gClientIp, &gClientPort, 0 );
        iprintf( "   Connection accepted from ");
        ShowIP( gClientIp );
        iprintf( ":%d\r\n", gClientPort );
    }

    if( gFdNet > 0 )
    {
        iprintf( "Connected!\n" );
    }
}

/*
 * @brief Writes data to the specified file descriptor.
 *
 * @param fd The file descriptor to write to
 * @param readingData If true, it will read data from the file descriptor, if false it will write.
 *
 * @returns Nothing
 *
 * @details This function will try to write data from a buffer to the given file descriptor until
 * BUFFER_SIZE number of bytes have been written.
 */
void ReadOrWriteTestData( int fd, bool readingData )
{
    // Read or write only as much data as BUFFER_SIZE specifies.
    int s = 0;
    if( readingData )
    {
        gHrTimer->start();
        s = read( fd, gTestData.curBufPtr, BUFFER_SIZE - gTestData.byteCount);
        gTestData.totalTestTime += gHrTimer->readTime();
    }
    else
    {
        gHrTimer->start();
        s = write( fd, gTestData.curBufPtr, BUFFER_SIZE - gTestData.byteCount );
        gTestData.totalTestTime += gHrTimer->readTime();
    }
    gHrTimer->stopClear();
    gTestData.byteCount += s;

    if( DEBUG_ENABLED )
    {
        iprintf( "     Buffer position: %c at %d", *gTestData.curBufPtr, gTestData.byteCount );
        iprintf( "     Sent: %d bytes\n", s );
    }

    // If the byte count has met or exceeded BUFFER_SIZE, update gTestData, validate the buffer if reading,
    // and print the test report.
    if( gTestData.byteCount >= BUFFER_SIZE )
    {
        bool testSuccess = gTestData.UpdateCurTestComplete();
        gTestData.ReportCurTestData();
        gTestData.ReportTotalTestData( gTestData.currentState );

        // If we are reading data, ensure what we received is correct.
        if( readingData )
        {
            const char* validMsg = testSuccess ? "Buffer validated, test successful." : "Buffer not valid, test failed.";
            iprintf( "%s\n", validMsg  );
        }
        gTestData.Reset( readingData, gTestInitiator );
    }
    // Otherwise, update the buffer pointer
    else
    {
        gTestData.UpdateBufPtr();
    }
}

/*
 * @brief Main function that initializes required data and structures, and runs the test.
 *
 * @details In addition to the standard setup, we this function will wait for a connection to be made
 * before stepping into the main function loop.
 */
void UserMain(void * pd) {
    InitializeStack();
    GetDHCPAddressIfNecessary();
    OSChangePrio( MAIN_PRIO );
    EnableAutoUpdate();
    EnableTaskMonitor();

    #ifndef _DEBUG
    EnableSmartTraps();
    #endif

    #ifdef _DEBUG
    InitializeNetworkGDB_and_Wait();
    #endif

    iprintf( "Starting app\n");

    // Close serial for polled mode, and re-open it for interrupt
    SerialClose( SERIAL_PORT );
    gFdSerial = OpenSerial( SERIAL_PORT, 115200, 1, 8, eParityNone );

    // Initialize our timer
    gHrTimer = HiResTimer::getHiResTimer();
    gHrTimer->init();

    // Wait for a connection before moving on to the testing
    ManageTcpConnection();

    // Initialize test state and fill data buffer with random data
    bool fillData = gTestInitiator ? true : false;
    gTestData.Reset( fillData, gTestInitiator );

    while( 1 ){
        if( gFdSerial > 0 )
        {
            // Determine which state of the test we are in and read or write data accordingly
            switch( gTestData.currentState )
            {
                case eTestSerialSend:
                {
                    if( gTestData.byteCount == 0 )
                    {
                        iprintf( "\n\nSerial send test started...\n" );
                    }
                    ReadOrWriteTestData( gFdSerial, false );
                    break;
                }
                case eTestSerialReceive:
                {
                    if( gTestData.byteCount == 0 )
                    {
                        iprintf( "\n\nSerial receive test started...\n" );
                    }

                    ReadOrWriteTestData( gFdSerial, true );
                    break;
                }
                case eTestEthernetSend:
                {
                    if( gTestData.byteCount == 0 )
                    {
                        iprintf( "\n\nEthernet send test started...\n" );
                    }
                    ReadOrWriteTestData( gFdNet, false );
                    break;
                }
                case eTestEthernetReceive:
                {
                    if( gTestData.byteCount == 0 )
                    {
                        iprintf( "\n\nEthernet receive test started...\n" );
                    }

                    ReadOrWriteTestData( gFdNet, true );
                    break;
                }
                case eTestComplete:
                {
                    // If we've completed the tests, start them again
                    bool fillData = gTestInitiator ? true : false;
                    gTestData.Reset( fillData, gTestInitiator );
                    OSTimeDly( TICKS_PER_SECOND * 3 );
                    break;
                }
                default:
                {
                    iprintf( "Error, invalid test state: %d\r\n", gTestData.currentState );
                }
            }
        }
    }
}
