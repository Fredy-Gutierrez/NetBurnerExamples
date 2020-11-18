#include <iosys.h>

#include "CppUTest/TestHarness.h"
#include "tests/testObject.h"

extern TestObject gTestObj;

TEST_GROUP( NbioTest )
{
    void setup()
    {
        gTestObj.Reset();
    }

    void teardown()
    {

    }
};

void TestVsnPrintf( int( *printFunction )(char *str, size_t size, const char* format, va_list va_args_list), const char* format, ... )
{
    va_list args;
    va_list args2;
    va_start( args, format );
    va_copy( args2, args );
    gTestObj.SetIntVar1( printFunction( gTestObj.GetSmallBuffer(), gTestBufferSizeSmall, format, args ) );
    //iprintf( "\r\nwrote 1: %d\r\n", gTestObj.GetIntVar1() );
    gTestObj.SetIntVar2( printFunction( gTestObj.GetLargeBuffer(), gTestBufferSizeLarge, format, args2 ) );
    //iprintf( "wrote 2: %d\r\n", gTestObj.GetIntVar2() );
    va_end( args );
    va_end( args2 );
}

/**
 * @brief This set of tests check the vnsprintf function with integer parameters
 */
TEST( NbioTest, VsnprintfOutputTestInt )
{
    TestVsnPrintf( vsnprintf, "%d", 1 );
    STRCMP_EQUAL( "1", gTestObj.GetLargeBuffer() );
    CHECK_EQUAL( gTestObj.GetIntVar1(), gTestObj.GetIntVar2() );

    TestVsnPrintf( vsnprintf, "%d", 123 );
    STRCMP_EQUAL( "123", gTestObj.GetLargeBuffer() );
    CHECK_EQUAL( gTestObj.GetIntVar1(), gTestObj.GetIntVar2() );

    TestVsnPrintf( vsnprintf, "%d %d", 1, 2 );
    STRCMP_EQUAL( "1 2", gTestObj.GetLargeBuffer() );
    CHECK_EQUAL( gTestObj.GetIntVar1(), gTestObj.GetIntVar2() );

    TestVsnPrintf( vsnprintf, "%d %d", 1, 23 );
    STRCMP_EQUAL( "1 23", gTestObj.GetLargeBuffer() );
    CHECK_EQUAL( gTestObj.GetIntVar1(), gTestObj.GetIntVar2() );

    TestVsnPrintf( vsnprintf, "%d %d %d", 1, 2, 3 );
    STRCMP_EQUAL( "1 2 3", gTestObj.GetLargeBuffer() );
    CHECK_EQUAL( gTestObj.GetIntVar1(), gTestObj.GetIntVar2() );

    TestVsnPrintf( vsnprintf, "%d %d %d", 1, 234, 5 );
    STRCMP_EQUAL( "1 234 5", gTestObj.GetLargeBuffer() );
    CHECK_EQUAL( gTestObj.GetIntVar1(), gTestObj.GetIntVar2() );

    TestVsnPrintf( vsnprintf, "%5d", 1 );
    STRCMP_EQUAL( "    1", gTestObj.GetLargeBuffer() );
    CHECK_EQUAL( gTestObj.GetIntVar1(), gTestObj.GetIntVar2() );

    TestVsnPrintf( vsnprintf, "%10d", 1 );
    STRCMP_EQUAL( "         1", gTestObj.GetLargeBuffer() );
    CHECK_EQUAL( gTestObj.GetIntVar1(), gTestObj.GetIntVar2() );

    TestVsnPrintf( vsnprintf, "%010d", 1 );
    STRCMP_EQUAL( "0000000001", gTestObj.GetLargeBuffer() );
    CHECK_EQUAL( gTestObj.GetIntVar1(), gTestObj.GetIntVar2() );

    TestVsnPrintf( vsnprintf, "%+05d", 1 );
    STRCMP_EQUAL( "+0001", gTestObj.GetLargeBuffer() );
    CHECK_EQUAL( gTestObj.GetIntVar1(), gTestObj.GetIntVar2() );

    TestVsnPrintf( vsnprintf, "%0*d", 5, -1 );
    STRCMP_EQUAL( "-0001", gTestObj.GetLargeBuffer() );
    CHECK_EQUAL( gTestObj.GetIntVar1(), gTestObj.GetIntVar2() );
}
