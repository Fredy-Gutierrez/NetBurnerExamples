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
 * @file unitTest\src\tests\nbrtos\nbrtosFlags.cpp
 * @brief Unit tests for nbrtos flag objects.
 *
 * @author NetBurner, Inc.
 * @version 1.01
 * @date 07-05-2017
 */

#include <basictypes.h>
#include <constants.h>
#include <nbrtos.h>
#include <stdio.h>

#include "CppUTest/TestHarness.h"
#include "tests/testObject.h"

extern uint32_t gTestTaskStack[USER_TASK_STK_SIZE] __attribute__((aligned(4)));
extern uint32_t gTestTaskStack2[USER_TASK_STK_SIZE] __attribute__((aligned(4)));

extern uint8_t gPriority;   // One above the main function priority
extern uint8_t gResult;
extern void *gParamData;
extern void *gResultData;
extern TestObject gTestObj;

OS_FLAGS gFlags;

/**
 * @brief The startup() and teardown() routines for the uCOS used to reinitialize variables and cleanup after
 * individual tests are run.
 */
TEST_GROUP(NbrtosTestFlags){void setup(){// MemoryLeakWarningPlugin::turnOffNewDeleteOverloads();
                                         gTestObj.Reset();
gResult = OS_NO_ERR;
gParamData = nullptr;
}

void teardown()
{
    // MemoryLeakWarningPlugin::turnOnNewDeleteOverloads();
    gParamData = nullptr;
}
}
;

/**
 * @brief Ensures that OSFlagCreate() correctly initializes a flag object.
 */
TEST(NbrtosTestFlags, FlagCreate)
{
    gFlags.Init();
    CHECK_EQUAL(0, gFlags.State());
}

/**
 * @brief Tests the functionality of OSFlagSet(), OSFlagState(), and OSFlagClear()
 *
 * The following tests are completed:
 *      - Set, clear and check each bit of the flag
 *      - Ensure that set bits are not overwritten when other bits are set
 */
TEST(NbrtosTestFlags, FlagSetStateClearSimple)
{
    gFlags.Init();

    uint32_t testFlags = 0;
    uint32_t setFlags = 0x0000'0001;

    while (setFlags != 0x0000'0000)
    {
        gFlags.Set(setFlags);
        testFlags = gFlags.State();

        CHECK_EQUAL(setFlags, testFlags);

        gFlags.Clear(setFlags);
        testFlags = gFlags.State();
        CHECK_EQUAL(0x0000'0000, testFlags);

        testFlags = 0;
        setFlags = setFlags << 1;
    }
}
