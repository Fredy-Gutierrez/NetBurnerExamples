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

#include "TestStateData.h"

TestStateData::TestStateData()
{
    InitBuffer( baseBuffer, true );
}

/*
 * @brief Reset to the start of a new test.
 *
 * @param fillBuffer Whether to fill the buffer with random data or zero it out
 * @param testInitiator Whether the device running is the one that initiated the tests
 *
 * @returns Nothing
 */
void TestStateData::Reset( bool fillBuffer, bool testInitiator )
{
    byteCount = 0;
    totalTestTime = 0.0;
    AdvanceState( testInitiator );
    InitBuffer( dataBuffer, fillBuffer );
    UpdateBufPtr();
}

/*
 * @brief Updates the current buffer pointer to the location where we are reading/writing the data.
 *
 * @returns Nothing
 */
void TestStateData::UpdateBufPtr()
{
    curBufPtr = &dataBuffer[byteCount];
}

/*
 * @brief Advances the current testing state.
 *
 * @params testInitator Whether this device initiated the connect or not, which is used to determine
 * the order of the tests (the testInitiator always sends then receives)
 *
 * @returns Nothing
 */
void TestStateData::AdvanceState( bool testInitiator )
{
    switch( currentState )
    {
        case eTestSerialSend:
        {
            currentState =
                    testInitiator ? eTestSerialReceive : eTestEthernetReceive;
            break;
        }
        case eTestSerialReceive:
        {
            currentState = testInitiator ? eTestEthernetSend : eTestSerialSend;
            break;
        }
        case eTestEthernetSend:
        {
            currentState = testInitiator ? eTestEthernetReceive : eTestComplete;
            break;
        }
        case eTestEthernetReceive:
        {
            currentState = testInitiator ? eTestComplete : eTestEthernetSend;
            break;
        }
        case eTestComplete:
        default:
        {
            currentState = eTestInit;
            break;
        }
    };

    if( currentState == eTestInit )
    {
        currentState = testInitiator ? eTestSerialSend : eTestSerialReceive;
    }
}

/*
 * @brief Validates if the data buffer used to receive and send data from matches our base data buffer.
 *
 * @returns True if the buffers match, and false if they do not.
 */
bool TestStateData::ValidateDataBuffer()
{
    for( int i = 0; i < BUFFER_SIZE; i++ )
    {
        if( dataBuffer[i] != baseBuffer[i] )
        {
            return false;
        }
    }

    return true;
}

/*
 * @brief Prints out data associated with the current test.
 *
 * @returns Nothing
 */
void TestStateData::ReportCurTestData()
{
    printf( "     Total Time:         %f seconds\n", totalTestTime );
    printf( "     Total Bytes:        %u\n", byteCount );
    double bps = totalTestTime > 0 ? ( BUFFER_SIZE / totalTestTime ) : 0.0;
    printf( "     Bytes per second:   %f\n", bps );
}

/*
 * @brief Updates the data and averages associated with the overall set of tests that have been run.
 *
 * @returns True if the data buffer and the validation buffer match, and false if they do not.
 */
bool TestStateData::UpdateCurTestComplete()
{
    TestResults* curTestResults = GetResultForState( currentState );

    if( curTestResults == nullptr )
    {
        iprintf( "Error: Trying to update test results in an invalid test state.\r\n");
        return false;
    }

    bool recSuccess = ValidateDataBuffer();
    curTestResults->count++;
    curTestResults->failCount += recSuccess ? 0 : 1;

    curTestResults->avgTime = ( curTestResults->avgTime * ( ( (float)curTestResults->count - 1.0f ) / (float)curTestResults->count ) )
                              + ( totalTestTime * ( 1.0f / (float)curTestResults->count ) );

    return recSuccess;
}

/*
 * @brief Prints out the cumulative data for a specific test type.
 *
 * @param state The state that the data should be printed for
 *
 * @returns Nothing
 */
void TestStateData::ReportTotalTestData( TestStates state )
{
    TestResults* curTestResults = GetResultForState( state );

    if( curTestResults == nullptr )
    {
        iprintf( "Error: Trying to report test results with an invalid test state.\r\n");
        return;
    }

    printf( "     Total Stats:\r\n" );
    printf( "          Total Tests:         %u\n", curTestResults->count );
    printf( "          Avg Test Time:       %f\n", curTestResults->avgTime );
    printf( "          Failure Count:       %u\n", curTestResults->failCount );
}

/*
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
        buf[ i ] = fill ? ( 'A' + (i % 64) ) :  '\0';
    }
}

/*
 * @brief Determines and returns a pointer to the test result structure associated with the test state provided.
 *
 * @param state Which state to find the test result structure for
 *
 * @returns A pointer to the test results structure associated with the provided state
 */
TestResults* TestStateData::GetResultForState( TestStates state )
{
    switch( currentState )
    {
        case eTestSerialSend:
        {
            return &serialSendResults;
        }
        case eTestSerialReceive:
        {
            return &serialRecResults;
        }
        case eTestEthernetSend:
        {
            return &etherSendResults;
        }
        case eTestEthernetReceive:
        {
            return &etherRecResults;
        }
        default:
        {
            return nullptr;
        }
    }
}
