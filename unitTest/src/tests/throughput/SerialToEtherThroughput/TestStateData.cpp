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

#include <stdio.h>
#include <nbrtos.h>

#include "TestStateData.h"

extern OS_CRIT gCritSection;
extern TestModes gConnectionMode;

TestStateData::TestStateData()
{
    InitBuffer( m_sendData.m_buffer, true );
    InitBuffer( m_recData.m_buffer, false );
}

/**
 * @brief Reset to the start of a new test.
 *
 * @param fillBuffer Whether to fill the buffer with random data or zero it out
 * @param testInitiator Whether the device running is the one that initiated the tests
 *
 * @returns Nothing
 */
void TestStateData::ResetTestData()
{
    if( DEBUG_ENABLED )
    {
        iprintf( "Reset test data.\r\n" );
    }
    OSCritEnter( &gCritSection, 0 );

    m_totalTestTime = 0.0;
    m_totalSendTime = 0.0;
    m_totalRecTime = 0.0;

    m_sendData.m_byteCount = 0;
    InitBuffer( m_sendData.m_buffer, true );
    UpdateBufPtr( m_sendData );

    m_recData.m_byteCount = 0;
    InitBuffer( m_recData.m_buffer, false );
    UpdateBufPtr( m_recData );

    OSCritLeave( &gCritSection );
}

/**
* @brief Reset to the data that tracks the average values of tests.
*
* @returns Nothing
*/
void TestStateData::ResetAvgData()
{
    m_results.m_count = 0;
    m_results.m_avgTime = 0.0;
    m_results.m_avgBytesPerSec = 0.0;
    m_results.m_failCount = 0;
}

/**
 * @brief Updates the current buffer pointer to the location where we are reading/writing the data.
 *
 * @returns Nothing
 */
void TestStateData::UpdateBufPtr( BufferObj &bufferObj )
{
    bufferObj.m_curBufPtr = &bufferObj.m_buffer[ bufferObj.m_byteCount ];
}

/**
 * @brief Validates if the data buffer used to receive and send data from matches our base data buffer.
 *
 * @returns True if the buffers match, and false if they do not.
 */
bool TestStateData::ValidateDataBuffer()
{
    for( int i = 0; i < BUFFER_SIZE; i++ )
    {
        //iprintf( "   send: %c     rec: %c\r\n", m_sendData.m_buffer[ i ], m_recData.m_buffer[ i ] );
        if( m_sendData.m_buffer[i] != m_recData.m_buffer[i] )
        {
            iprintf( "Failed at %d\r\n", i );
            iprintf( "   send: %c     rec: %c\r\n", m_sendData.m_buffer[ i ], m_recData.m_buffer[ i ] );
            return false;
        }
    }

    return true;
}

/**
 * @brief Prints out data associated with the current test.
 *
 * @returns Nothing
 */
void TestStateData::ReportCurTestData()
{
    printf( "     Total Bytes Sent:        %u\n", m_sendData.m_byteCount );
    printf( "     Total Bytes Received:    %u\n", m_recData.m_byteCount );
    double bps = m_totalTestTime > 0 ? ( BUFFER_SIZE / m_totalTestTime ) : 0.0;
    printf( "     Bytes per second:   %f\n", bps );
    printf( "     Total Time:         %f seconds\n", m_totalTestTime );
    printf( "        Time Sending:    %f seconds\n", m_totalSendTime );
    printf( "        Time Receiving:  %f seconds\n\n", m_totalRecTime );
}

/**
 * @brief Updates the data and averages associated with the overall set of tests that have been run.
 *
 * @returns True if the data buffer and the validation buffer match, and false if they do not.
 */
bool TestStateData::UpdateCurTestComplete()
{
    bool recSuccess = true;
    if( gConnectionMode == eTestModeTCP )
    {
        recSuccess = ValidateDataBuffer();
    }

    m_results.m_count++;
    m_results.m_failCount += recSuccess ? 0 : 1;

    m_results.m_avgTime = ( m_results.m_avgTime * ( ( (float)m_results.m_count - 1.0f ) / (float)m_results.m_count ) )
                              + ( m_totalTestTime * ( 1.0f / (float)m_results.m_count ) );

    double bps = m_totalTestTime > 0 ? (BUFFER_SIZE / m_totalTestTime) : 0.0;
    m_results.m_avgBytesPerSec = ( m_results.m_avgBytesPerSec * ( ( (float)m_results.m_count - 1.0f) / (float)m_results.m_count ) )
        + ( bps * ( 1.0f / (float)m_results.m_count ) );

    return recSuccess;
}

/**
 * @brief Prints out the cumulative data for a specific test type.
 *
 * @param state The state that the data should be printed for
 *
 * @returns Nothing
 */
void TestStateData::ReportTotalTestData()
{
    printf( "Total Stats:\r\n" );
    printf( "   Total Tests:         %u\n", m_results.m_count );
    printf( "   Avg Test Time:       %f seconds\n", m_results.m_avgTime );
    printf( "   Avg Bytes Per Sec:   %f\n", m_results.m_avgBytesPerSec );
    printf( "   Failure Count:       %u\n", m_results.m_failCount );
}

/**
 * @brief Initializes a buffer.
 *
 * @param buf The buffer to initialize
 * @param fill Whether to fill the buffer with random data, or zero it out
 *
 * @returns Nothing
 */
void TestStateData::InitBuffer( char* buf, bool fill )
{
    if( buf == nullptr )
    {
        return;
    }

    for( int i = 0; i < BUFFER_SIZE; i++ )
    {
        buf[ i ] = fill ? ( 'A' + (i % 26) ) :  '\0';
    }
}
