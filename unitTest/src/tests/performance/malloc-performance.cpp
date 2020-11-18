#include <stopwatch.h>
#include <stdio.h>

#include "CppUTest/TestHarness.h"
#include "tests/testObject.h"

extern TestObject gTestObj;
int gMsgBufSize = 80;

TEST_GROUP( MallocTest )
{
    StopWatch stopWatch;

    void setup()
    {
        gTestObj.Reset();
    }

    void teardown()
    {
    }
};

double TestMalloc( int byteCount, void*& memPtr, StopWatch& stopWatch )
{
    unsigned long long mallocTime = 0.0;
    stopWatch.Start();
    memPtr = malloc( byteCount );
    mallocTime = stopWatch.Stop() * 1000;
    stopWatch.Clear();
    return mallocTime;
}

double TestFree( void*& memPtr, StopWatch& stopWatch )
{
    unsigned long long mallocTime = 0.0;
    stopWatch.Start();
    free( memPtr );
    mallocTime = stopWatch.Stop() * 1000;
    stopWatch.Clear();
    return mallocTime;
}

/**
 * @brief This set of tests checks the performance of the malloc operation for varying sizes of data
 */
TEST( MallocTest, MallocLargeDataTest )
{
    char msgBuf[ gMsgBufSize ] = { '\0' };
    double testTime = 0.0;
    int byteCount = 1024;
    int testCount = 7;  // 7 iterations gets us around 4MB, which is safe for all currently tested platforms
    void* memPtr = nullptr;
    iprintf( "\r\nStart MallocLargeDataTest\r\n" );
    for( int i = 0; i < testCount; i++ )
    {
        testTime = TestMalloc( byteCount, memPtr, stopWatch );
        CHECK( memPtr != nullptr );
        snprintf( msgBuf, gMsgBufSize, "Malloc %d b (ms)", byteCount );
        UT_MEASURE_DOUBLE( msgBuf, testTime );

        testTime = TestFree( memPtr, stopWatch );
        snprintf( msgBuf, gMsgBufSize, "Freed %d b (ms)", byteCount );
        UT_MEASURE_DOUBLE( msgBuf, testTime );
        byteCount *= 4;
    }
}

/**
* @brief This set of tests checks the performance of the malloc operation over several iterations
*/
TEST( MallocTest, MallocSpeedTest )
{
    char msgBuf[ gMsgBufSize ] = { '\0' };
    double testTime = 0.0;
    int byteCount = 1;
    int maxTestCount = 1000;
    int curTestCount = 10;
    void* addressList[ maxTestCount ];

    iprintf( "\r\nStart MallocSpeedTest\r\n" );
    while( curTestCount <= maxTestCount )
    {
        for( int i = 0; i < curTestCount; i++ )
        {
            testTime += TestMalloc( byteCount, addressList[ i ], stopWatch );
            CHECK( addressList[ i ] != nullptr );
        }
        snprintf( msgBuf, gMsgBufSize, "Malloc %d b %d times (ms)", byteCount, curTestCount );
        UT_MEASURE_DOUBLE( msgBuf, testTime );

        testTime = 0.0;
        for( int i = 0; i < curTestCount; i++ )
        {
            testTime += TestFree( addressList[ i ], stopWatch );
        }
        snprintf( msgBuf, gMsgBufSize, "Freed %d b %d times (ms)", byteCount, curTestCount );
        UT_MEASURE_DOUBLE( msgBuf, testTime );

        curTestCount *= 10;
    }
}

