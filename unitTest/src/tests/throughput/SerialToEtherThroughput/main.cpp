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

/**
 * @file       unitTest\src\tests\throughput\SerialToEtherThroughput\main.cpp
 * @brief      Serial and Ethernet throughput testing.
 *
 * This application will run a set of throughput tests from a serial to Ethernet connection, or vice versa.  To use, load
 * this program onto a device that will be connected to a Serial to Ethernet device (such as the SB800EX).  Connect the
 * Serial to Ethernet device to the testing device via the serial port, and connect both devices up to a network using
 * the Ethernet ports.  Ensure that the Serial to Ethernet device is configured to accept connections on a specified port,
 * and modify this program in the following ways:
 * 
 * 1. Change gClientIp in main.cpp to reflect that of the Serial to Ethernet device
 * 2. Change gClientPort in main.cpp to reflect that of the listening port on the Serial to Ethernet device
 * 3. Change gTestSerialPort in main.cpp to the port which the testing device will connect to the Serial to Ethernet device
 * 4. (Optional) Change gSerialToEther in main.cpp to false if the desired test direction is from Ethernet to serial
 * 5. (Optional) Change BUFFER_SIZE in Constant.h to the size of data you wish to test
 * 6. (Optional) Change TEST_COUNT in Constants.h to the number of times you would like to perform the test
 *
 * The following steps are optional and can be 
 */

#include <autoupdate.h>
#include <ctype.h>
#include <dhcpclient.h>
#include <HiResTimer.h>
#include <netinterface.h>
#include <NetworkDebug.h>
#include <predef.h>
#include <stdio.h>
#include <serial.h>
#include <smarttrap.h>
#include <startnet.h>
#include <stdio.h>
#include <stdlib.h>
#include <taskmon.h>
#include <tcp.h>
#include <nbrtos.h>
#include <udp.h>

#include <atcommand.h>

#include "AtCommandProc.h"
#include "CommandBuffer.h"
#include "ThroughputConstants.h"
#include "TestStateData.h"

extern "C" {
    void UserMain(void * pd);
}

const char * AppName="SerialToEthernetThroughputTest";

IPADDR gClientIp = AsciiToIp4( "10.1.1.56" );  // Address of listening device
WORD   gClientPort = 1234;                     // Port both devices will use for connection

int gFdSerial             = 0;        // File descriptor for serial connection
int gFdNet                = 0;        // File descriptor for Ethernet connection
int gFdDebug              = 0;        // File descriptor for debug window
TestStateData gTestData;              // This hold all of the data and test related information
AtCommandProc gAtCommandProc;         // Manages the AT commands that are issued to the connected device
CommandBuffer gCommandBuffer;         // Buffer that reads and processes commands from the debug window
HiResTimer * gHrTimer1;               // Timer used for total time spent
HiResTimer * gHrTimer2;               // Timer used for sending data
HiResTimer * gHrTimer3;               // Timer used for receiving data
bool gTimerStartedWrite     = false;
bool gTimerStartedRead      = false;
bool gTestInProgress        = false;
TestModes gConnectionMode   = eTestModeTCP;
DWORD gReadTimeout          = 10;
bool gSerialToEther         = true;
int gTestSerialPort         = 1;

asm( " .align 4 " );
DWORD TaskStack1[ USER_TASK_STK_SIZE ] __attribute__( (aligned( 4 )) );
DWORD TaskStack2[ USER_TASK_STK_SIZE ] __attribute__( (aligned( 4 )) );

OS_CRIT gCritSection;
OS_FIFO gUdpFifo;

/**
 * @brief Displays a prompt symbol to assist with debug menu navigation
 *
 * @returns Nothing
 */
void DisplayPrompt( int fd )
{
    writestring( fd, "> " );
}

/**
 * @brief Either listens for a connection, or initiates one.
 *
 * Whether a device listens or initiates the connection is decided by the IP address assigned to gClientIp.
 * If the device has the same IP address, it will listen, otherwise it will initiate the connection.
 */
void ManageTcpConnection()
{
    // Try to connect
    iprintf( "Trying connection...\n" );

    int FirstInterface  = GetFirstInterface();  // Get first interface identifier
    InterfaceBlock *ib  = GetInterFaceBlock(FirstInterface);  // Get interface data

    // If the IP address does not match the client IP given, then we are initiating the connection
    if( ib != nullptr && ib->netIP != gClientIp )
    {
        gFdNet = connect( gClientIp, 0, gClientPort, TICKS_PER_SECOND * 15 );
        if( gFdNet > 0 )
        {
            iprintf( "Connection accepted from " );
            ShowIP( gClientIp );
            iprintf( ":%d\r\n", gClientPort );
        }
        else
        {
            iprintf( "Unable to connect to " );
            ShowIP( gClientIp );
            iprintf( ":%d.\r\n", gClientPort );
        }
    }
}

/**
 * @brief Reads data to the specified file descriptor.
 *
 * @param fd The file descriptor to write to
 * @param dataObj The buffer object used to hold data that we are reading, and to track progress in that read
 *
 * @returns True if the read portion of the test is finished, and false otherwise
 *
 * @details This function will try to read data from a buffer to the given file descriptor until
 * BUFFER_SIZE number of bytes have been read.
 */
bool ReadTestData( int fd, BufferObj& dataObj )
{
    if( dataObj.m_byteCount >= BUFFER_SIZE )
    {
        return true;
    }

    if( DEBUG_ENABLED )
    {
        iprintf( "     Reading " );
    }

    // Read only as much data as BUFFER_SIZE specifies.
    int s = 0;
    bool forceQuit = false; // Use this to force the test to complete if we timeout in UDP mode after receiving data
    DWORD tempTimeout = gReadTimeout;

    // Due to the fire and forget nature of UDP, we use the timeout as a way to determine if the test is over, and
    // track how many bytes we have lost
    if( gConnectionMode == eTestModeUDP )
    {
        tempTimeout = TICKS_PER_SECOND * 3;
    }

    // If we are receiving serially, or by TCP through Ethernet
    if( !gSerialToEther || ( gConnectionMode == eTestModeTCP ) )
    {
        s = ReadWithTimeout( fd, dataObj.m_curBufPtr, BUFFER_SIZE - dataObj.m_byteCount, tempTimeout );

        // If we're in UDP and timeout, assume the data we're missing is lost
        if( ( gConnectionMode == eTestModeUDP ) && ( s == 0 ) )
        {
            forceQuit = true;
        }
    }
    else if( gConnectionMode == eTestModeUDP )
    {
        UDPPacket gUdpPacket( &gUdpFifo, tempTimeout );
        if( !gUdpPacket.Validate() && ( dataObj.m_byteCount > 0 ) )
        {
            forceQuit = true;
        }
        s = gUdpPacket.GetDataSize();
    }

    dataObj.m_byteCount += s;

    // If this is our first pass reading data, go ahead and start the timer
    if( !gTimerStartedRead && s > 0 )
    {
        gHrTimer3->start();
        gTimerStartedRead = true;
    }

    if( DEBUG_ENABLED )
    {
        iprintf( " - Buffer pos: %c at %d", *dataObj.m_curBufPtr, dataObj.m_byteCount );
        iprintf( " received: %d bytes\n", s );
    }

    // If the byte count has met or exceeded BUFFER_SIZE, update gTestData, validate the buffer,
    // and print the test report.
    if( ( dataObj.m_byteCount >= BUFFER_SIZE ) || forceQuit )
    {
        gTestData.m_totalTestTime += gHrTimer1->readTime();
        gHrTimer1->stopClear();
        
        gTestData.m_totalRecTime += gHrTimer3->readTime();
        gHrTimer3->stopClear();

        // Don't include the timeout value for UDP tests
        if( forceQuit )
        {
            iprintf( "Used forcequit option\r\n" );
            gTestData.m_totalTestTime -= 3.0;
            gTestData.m_totalRecTime -= 3.0;
        }
        
        bool testSuccess = gTestData.UpdateCurTestComplete();
        gTestData.ReportCurTestData();
        
        // Ensure what we received is correct if in TCP mode
        if( gConnectionMode == eTestModeTCP )
        {
            const char* validMsg = testSuccess ? "     Buffer validated, test successful.\n" : "     Buffer not valid, test failed.\n";
            iprintf( "%s\n", validMsg );
        }
        
        if( ( gTestData.m_results.m_count >= TEST_COUNT ) && ( TEST_COUNT > 1 ) )
        {
            gTestData.ReportTotalTestData();
        }
        else
        {
            OSCritEnter( &gCritSection, 0 );
            gTestData.ResetTestData();
            gTimerStartedRead = false;
            gTimerStartedWrite = false;
            OSCritLeave( &gCritSection );
        }
        return true;
    }
    // Otherwise, update the buffer pointer
    else
    {
        gTestData.UpdateBufPtr( dataObj );
    }

    return false;
}

/**
 * @brief Writes data to the specified file descriptor.
 *
 * @param fd The file descriptor to write to
 * @param dataObj The buffer object used to hold data that we are writing, and to track progress in that write
 *
 * @returns True if the write portion of the test is finished, and false otherwise
 *
 * @details This function will try to write data from a buffer to the given file descriptor until
 * BUFFER_SIZE number of bytes have been written.
 */
bool WriteTestData( int fd, BufferObj& dataObj )
{
    if( dataObj.m_byteCount >= BUFFER_SIZE )
    {
        return true;
    }

    // If this is our first pass writing data, go ahead and start the timer
    if( !gTimerStartedWrite )
    {
        iprintf( "Test %d starting... ", gTestData.m_results.m_count + 1 );
        gHrTimer1->start();
        gHrTimer2->start();
        gTimerStartedWrite = true;
    }

    if( DEBUG_ENABLED )
    {
        iprintf( "     Writing " );
    }

    int s = 0;
    int writeSize = ( ( BUFFER_SIZE - dataObj.m_byteCount ) > TRANSMISSION_RATE ) ? TRANSMISSION_RATE : BUFFER_SIZE - dataObj.m_byteCount;
    if( gSerialToEther || ( gConnectionMode == eTestModeTCP ) )
    {
        s = write( fd, dataObj.m_curBufPtr, writeSize );
    }
    else if( gConnectionMode == eTestModeUDP )
    {
        UDPPacket gUdpPacket;
        gUdpPacket.SetSourcePort( gClientPort );
        gUdpPacket.SetDestinationPort( gClientPort );
        gUdpPacket.AddData( dataObj.m_curBufPtr );
        gUdpPacket.SendAndKeep( gClientIp );
        s = ( ( BUFFER_SIZE - dataObj.m_byteCount ) > MAX_UDPDATA ) ? MAX_UDPDATA : BUFFER_SIZE - dataObj.m_byteCount;
    }
    
    dataObj.m_byteCount += s;

    if( DEBUG_ENABLED )
    {
        iprintf( " - Buffer pos: %c at %d", *dataObj.m_curBufPtr, dataObj.m_byteCount );
        iprintf( " sent: %d bytes\n", s );
    }

    // If the byte count has met or exceeded BUFFER_SIZE, mark the timers
    if( dataObj.m_byteCount >= BUFFER_SIZE )
    {
        gTestData.m_totalSendTime = gHrTimer2->readTime();
        gHrTimer2->stopClear();

        return true;
    }
    // Otherwise, update the buffer pointer
    else
    {
        gTestData.UpdateBufPtr( dataObj );
    }

    return false;
}

/**
 * @brief Starts the send task for a throughput test.
 *
 * @returns Nothing.
 */
void ThroughputSendTask( void *pdata )
{
    // Delay enough time so that we can start the receive task
    OSTimeDly( TICKS_PER_SECOND );
    bool testComplete = false;
    int fd = gSerialToEther ? gFdSerial : gFdNet;
    while( !testComplete )
    {
        testComplete = WriteTestData( fd, gTestData.m_sendData );
    }
    iprintf( "Write complete\r\n" );
}

/**
* @brief Starts the receive task for a throughput test.
*
* @returns Nothing.
*/
void ThroughputRecTask( void *pdata )
{
    bool testComplete = false;
    if( gConnectionMode == eTestModeUDP )
    {
        RegisterUDPFifo( gClientPort, &gUdpFifo );
    }

    int fd = gSerialToEther ? gFdNet : gFdSerial;
    while( !testComplete )
    {
        testComplete = ReadTestData( fd, gTestData.m_recData );
    }
    gTestInProgress = false;
}

/**
 * @brief Initiates a connection and starts the test
 *
 * @returns Nothing
 */
void StartTest()
{
    iprintf( "Throughput test starting with buffer size %d, and repeat count %d:\r\n\n", BUFFER_SIZE, TEST_COUNT );
    gTestData.ResetTestData();
    gTestData.ResetAvgData();
    gTimerStartedRead = false;
    gTimerStartedWrite = false;

    close( gFdNet );
    gFdNet = 0;
    while( gTestData.m_results.m_count < TEST_COUNT )
    {
        if( gTestInProgress == false )
        {
            gTestInProgress = true;
            if( ( gFdNet == 0 ) && ( gConnectionMode == eTestModeTCP ) )
            {
                ManageTcpConnection();
                if( gFdNet < 0 )
                {
                    iprintf( "Aborting test due to no connection.\r\n" );
                    gTestInProgress = false;
                    gFdNet = 0;
                    return;
                }
            }

            if( OSTaskCreate( ThroughputSendTask,
                nullptr,
                (void*)&TaskStack1[ USER_TASK_STK_SIZE ],
                (void *)TaskStack1, MAIN_PRIO - 1
            ) != OS_NO_ERR )
            {
                iprintf( "Failed to create send task!\r\n" );
            }

            if( OSTaskCreate( ThroughputRecTask,
                nullptr,
                (void*)&TaskStack2[ USER_TASK_STK_SIZE ],
                (void *)TaskStack2, MAIN_PRIO - 2
            ) != OS_NO_ERR )
            {
                iprintf( "Failed to create receive task!\r\n" );
            }
        }

        OSTimeDly( TICKS_PER_SECOND );
    }

    close( gFdNet );
    gFdNet = 0;
}

/**
 * @brief Displays the debug menu
 * 
 * @returns Nothing
 */
void DisplayDebugMenu( int fd )
{
    iprintf( "Connection IP: " );
    ShowIP( gClientIp );
    iprintf( "     Connection Port: %d\r\n", gClientPort );
    iprintf( "Current Connection Mode: %s\r\n", ( gConnectionMode == eTestModeTCP ) ? "TCP" : "UDP" );
    iprintf( "Debug Commands: \r\n" );
    iprintf( "     I - Set IP address for connection\r\n" );
    iprintf( "     P - Set port number for connection\r\n" );
    iprintf( "     S - Start serial to Ethernet test\r\n" );
    iprintf( "     E - Start Ethernet to serial test\r\n" );
    iprintf( "     O - Set serial port number to use with test\r\n" );
    iprintf( "     T - Set connection mode to TCP\r\n" );
    iprintf( "     U - Set connection mode to UDP\r\n" );
    iprintf( "     ? - Display debug menu\r\n" );
    DisplayPrompt( fd );
}

/**
* @brief Process debug commands that are entered
*
* @returns Nothing
*/
void ProcessDebugCommand( int fd )
{
    if( !gCommandBuffer.ReadChar( fd ) )
    {
        iprintf( "Unable to read command from file descriptor: %d\r\n", fd );
        return;
    }

    write( fd, &gCommandBuffer.GetCommandBuffer()[ 0 ], 1 );
    writestring( fd, "\r\n" );

    switch( toupper( gCommandBuffer.GetCommandBuffer()[ 0 ] ) )
    {
        case 'I':
        {
            writestring( fd, "Enter IP address: " );
            gCommandBuffer.ReadString( fd );
            gClientIp = AsciiToIp4( gCommandBuffer.GetCommandBuffer() );
            iprintf( "IP set to " );
            ShowIP( gClientIp );
            iprintf( "\r\n" );
            break;
        }
        case 'P':
        {
            writestring( fd, "Enter port: " );
            gCommandBuffer.ReadString( fd );
            gClientPort = atoi( gCommandBuffer.GetCommandBuffer() );
            iprintf( "Port set to %d\r\n", gClientPort );
            break;
        }
        case 'S':
        {
            gSerialToEther = true;
            StartTest();
            break;
        }
        case 'E':
        {
            gSerialToEther = false;
            StartTest();
            break;
        }
        case 'O':
        {
            writestring( fd, "Enter port: " );
            gCommandBuffer.ReadString( fd );
            int defVal = gTestSerialPort;
            gTestSerialPort = atoi( gCommandBuffer.GetCommandBuffer() );
            if( gTestSerialPort > NB_FACTORY_SERIAL_PORTS )
            {
                iprintf( "Value requested is out of range. Valid range is up to but not including %d\r\n", NB_FACTORY_SERIAL_PORTS );
                gTestSerialPort = defVal;
            }
            else
            {
                // Close old port value and open new value, ensuring interrupt mode
                SerialClose( defVal );
                SerialClose( gTestSerialPort );
                gFdSerial = OpenSerial( gTestSerialPort, 115200, 1, 8, eParityNone );
            }
            iprintf( "Test Serial Port set to: %d\r\n", gTestSerialPort );
            break;
        }
        case 'T':
        {
            gAtCommandProc.SetMode( eTestModeTCP );
            break;
        }
        case 'U':
        {
            gAtCommandProc.SetMode( eTestModeUDP );
            break;
        }
        case '?':
        {
            iprintf( "\r\n" );
            DisplayDebugMenu( fd );
            return;
        }
        default:
        {
            iprintf( "Unrecognized command: %c\r\n\n", gCommandBuffer.GetCommandBuffer()[ 0 ] );
            DisplayDebugMenu( fd );
            return;
        }
    }

    DisplayPrompt( fd );
}

/**
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
    SerialClose( gTestSerialPort );
    gFdSerial = OpenSerial( gTestSerialPort, 115200, 1, 8, eParityNone );

    // Close serial for polled mode, and re-open it for interrupt
    SerialClose( DEBUG_SERIAL_PORT );
    gFdDebug = OpenSerial( DEBUG_SERIAL_PORT, 115200, 1, 8, eParityNone );

    // Initialize our timers
    gHrTimer1 = HiResTimer::getHiResTimer();
    gHrTimer1->init();

    gHrTimer2 = HiResTimer::getHiResTimer();
    gHrTimer2->init();

    gHrTimer3 = HiResTimer::getHiResTimer();
    gHrTimer3->init();

    OSCritInit( &gCritSection );

    // Wait for a connection before moving on to the testing
    OSTimeDly( TICKS_PER_SECOND );

    // Initialize test state and fill data buffer with random data
    gTestData.ResetTestData();

    DisplayDebugMenu( gFdDebug );
    
    while( 1 )
    {
        if( gTestInProgress )
        {
            OSTimeDly( TICKS_PER_SECOND );
            continue;
        }

        fd_set fd_read;
        FD_ZERO( &fd_read );
        FD_SET( gFdDebug, &fd_read );

        if( select( FD_SETSIZE, &fd_read, nullptr, nullptr, TICKS_PER_SECOND / 2 ) )
        {
            if( FD_ISSET( gFdDebug, &fd_read ) )
            {
                ProcessDebugCommand( gFdDebug );
            }
        }
    }
}
