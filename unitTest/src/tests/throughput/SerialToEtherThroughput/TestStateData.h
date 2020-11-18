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
#pragma once

#include <basicTypes.h>

#include "ThroughputConstants.h"

/**
 * @brief This holds the cumulative results for a specific set of tests.
 */
struct TestResults
{
    uint32_t m_count         = 0;
    double m_avgTime         = 0.0;
    double m_avgBytesPerSec  = 0.0;
    uint32_t m_failCount     = 0;
};

struct BufferObj
{
    char m_buffer[ BUFFER_SIZE ];
    uint m_byteCount    = 0;
    char* m_curBufPtr   = nullptr;
};

/**
 * @brief This is the main object that will keep track of overall test progress and results.
 */
class TestStateData
{
public:
    TestStateData();
    ~TestStateData(){}

    void ResetTestData();
    void ResetAvgData();
    void UpdateBufPtr( BufferObj &bufferObj );
    bool ValidateDataBuffer();
    void ReportCurTestData();
    bool UpdateCurTestComplete();
    void ReportTotalTestData();
    inline void IncrementTestTime( double incVal ){ m_totalTestTime += incVal; }

    BufferObj m_sendData; // Buffer to hold data that will be sent
    BufferObj m_recData;  // Buffer to hold data that is received
    double m_totalTestTime  = 0.0;  // How long have we been running this specific test
    double m_totalSendTime  = 0.0;  // How long have we been sending data
    double m_totalRecTime   = 0.0;  // How long have we been receiving data

    // Reporting metrics for each series of tests
    TestResults m_results;

private:
    void InitBuffer( char* buf, bool fill );
};

#endif /* _TEST_STATE_DATA_H_ */

