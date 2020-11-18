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
 * @file nbrtosTimeTick.cpp
 * @brief Unit tests for nbrtos Time Tick 
 *
 * @author NetBurner, Inc.
 * @version 1.01
 * @date 06-05-2017
 */


#include <iosys.h>
#include "CppUTest/TestHarness.h"
#include "tests/testObject.h"
#include "CppUTest/PlatformSpecificFunctions_c.h"

#include <nbrtos.h>
#include <stopwatch.h>

extern TestObject gTestObj;
extern StopWatch cppUTestTimer;

TEST_GROUP( NbrtosTestTimeTick )
{
    void setup()
    {
        gTestObj.Reset();
    }

    void teardown()
    {
    }
};

TEST(NbrtosTestTimeTick, MinuteTest)
{
   unsigned long long startTime = cppUTestTimer.GetTime();
   OSTimeDly(TICKS_PER_SECOND * 60 );
   unsigned long long endTime = cppUTestTimer.GetTime();

   printf("\r\n%g",cppUTestTimer.Convert(endTime - startTime));

}

TEST(NbrtosTestTimeTick, HourTest)
{
   unsigned long long startTime = cppUTestTimer.GetTime();
   OSTimeDly(TICKS_PER_SECOND * 60 * 60 );
   unsigned long long endTime = cppUTestTimer.GetTime();

   printf("\r\n%g",cppUTestTimer.Convert(endTime - startTime));

}

TEST(NbrtosTestTimeTick, TwelveHourTest)
{
   unsigned long long startTime = cppUTestTimer.GetTime();
   OSTimeDly(TICKS_PER_SECOND * 60 * 60 * 12 );
   unsigned long long endTime = cppUTestTimer.GetTime();

   printf("\r\n%g",cppUTestTimer.Convert(endTime - startTime));
}
