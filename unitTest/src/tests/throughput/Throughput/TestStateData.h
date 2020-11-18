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

#ifndef _TEST_STATE_DATA_H_
#define _TEST_STATE_DATA_H_

#include <basicTypes.h>
#include "Constants.h"

/*
 * @brief This holds the cumulative results for a specific set of tests.
 */
struct TestResults
{
    uint32_t count         = 0;
    double avgTime         = 0.0;
    uint32_t failCount     = 0;
};

/*
 * @brief This is the main object that will keep track of overall test progress and results.
 */
class TestStateData
{
public:
    TestStateData();
    ~TestStateData(){}

    void Reset( bool fillBuffer, bool testInitiator );
    void UpdateBufPtr();
    void AdvanceState( bool testInitiator );
    bool ValidateDataBuffer();
    void ReportCurTestData();
    bool UpdateCurTestComplete();
    void ReportTotalTestData( TestStates state );
    inline void IncrementTestTime( double incVal ){ totalTestTime += incVal; }

    char dataBuffer[ BUFFER_SIZE ];                 // This is the buffer that should be used when sending data back and forth
    char baseBuffer[ BUFFER_SIZE ];                 // This buffer is used to valide successful data transfers
    uint byteCount                  = 0;            // The total count of bytes read for the current test
    char* curBufPtr                 = nullptr;      // Pointer to the spot in the buffer where we are currently reading/writing to
    TestStates currentState         = eTestInit;    // Current state of the tests
    double totalTestTime            = 0.0;          // How long have we been running this specific test

    // Reporting metrics for each series of tests
    TestResults serialSendResults;
    TestResults serialRecResults;
    TestResults etherSendResults;
    TestResults etherRecResults;

private:
    void InitBuffer( char* buf, bool fill );
    TestResults* GetResultForState( TestStates state );
};

#endif /* _TEST_STATE_DATA_H_ */

