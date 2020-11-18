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
 * @file nbrtosLock.cpp
 * @brief Unit tests for nbrtos lock
 *
 * This holds tests for using OSLock and the lock objects.
 *
 * @author NetBurner, Inc.
 * @version 1.01
 * @date 06-05-2017
 */

#include <basictypes.h>
#include <constants.h>
#include <nbrtos.h>

#include "CppUTest/TestHarness.h"
#include "tests/testObject.h"

extern uint32_t gTestTaskStack[USER_TASK_STK_SIZE] __attribute__((aligned(4)));
extern uint32_t gTestTaskStack2[USER_TASK_STK_SIZE] __attribute__((aligned(4)));

extern uint8_t gPriority;   // One above the main function priority
extern uint8_t gResult;
extern void *gParamData;

/**
 * @brief The startup() and teardown() routines for the uCOS used to reinitialize variables and cleanup after
 * individual tests are run.
 */
TEST_GROUP(NbrtosTestLock){void setup(){// MemoryLeakWarningPlugin::turnOffNewDeleteOverloads();
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

void TestLockObj()
{
    OSLockObj lock;
    {
        OSLockObj lock;
        {
            OSLockObj lock;
            return;
        }
    }
}

/**
 * @brief This set of tests check OSLock() and OSUnlock().
 *
 * Tests that are completed here are as follows:
 *     - Calls OSLock() and OSUnlock(), and checks that OSLockNesting is being properly incremented and
 *       decremented.
 *
 * @todo Establish better tests that actually attempt to task switch.
 *
 * @bug It appears to be possible to create a task in a locked portion of the code that the OS switches too, despite the lock.
 */
TEST(NbrtosTestLock, LockAndUnlock)
{
    OSLock();
    OSLock();
    OSLock();

    CHECK(OSLockNesting == 3);

    OSUnlock();

    CHECK(OSLockNesting == 2);

    OSLock();
    OSLock();

    CHECK(OSLockNesting == 4);

    OSUnlock();

    CHECK(OSLockNesting == 3);

    OSUnlock();
    OSUnlock();
    OSUnlock();

    CHECK(OSLockNesting == 0);

    OSUnlock();

    CHECK(OSLockNesting == 0);
}

/**
 * @brief This set of tests check OSLockObj.
 *
 * Tests that are completed here are as follows:
 *     - Test that the lock object correctly locks and unlocks in scope.
 *     - Test that multiple lock objects correctly lock and unlock in nested scopes.
 *     - Test that multiple lock objects in the same scope resolve correctly.
 *     - Test that lock objects created and locked in a separate function correctly unlock when out of scope.
 *
 * @todo Add testing for locks across tasks
 */
TEST(NbrtosTestLock, LockObj)
{
    // Test that the lock object correctly locks and unlocks in scope
    {
        OSLockObj lock;
        CHECK(OSLockNesting == 1);
    }
    CHECK(OSLockNesting == 0);

    // Test that the lock object correctly locks and unlocks in nested scopes
    {
        OSLockObj lock;
        CHECK(OSLockNesting == 1);

        {
            OSLockObj lock;
            CHECK(OSLockNesting == 2);
        }
        CHECK(OSLockNesting == 1);
    }
    CHECK(OSLockNesting == 0);

    // Test multiple lock objects in one scope
    {
        OSLockObj lock;
        CHECK(OSLockNesting == 1);

        OSLockObj lock2;
        CHECK(OSLockNesting == 2);
    }
    CHECK(OSLockNesting == 0);

    // Test multiple lock objects being set from a separate function and return
    TestLockObj();
    CHECK(OSLockNesting == 0);
}
