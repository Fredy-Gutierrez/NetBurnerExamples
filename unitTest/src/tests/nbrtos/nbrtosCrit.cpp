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
 * @file nbrtosCrit.cpp
 * @brief Unit tests for nbrtos crit objects.
 *
 * @author NetBurner, Inc.
 * @version 1.01
 * @date 06-05-2017
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
extern bool gTestFuncRan;
extern TestObject gTestObj;

OS_CRIT gCritSect;

/**
 * @brief The startup() and teardown() routines for the uCOS used to reinitialize variables and cleanup after
 * individual tests are run.
 */
TEST_GROUP(NbrtosTestCrit){void setup(){// MemoryLeakWarningPlugin::turnOffNewDeleteOverloads();
                                        gCritSect.Init();
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

void TestOsCrit(void *data)
{
    if (data == nullptr) { return; }
    TestObject *testObj = (TestObject *)data;

    gCritSect.Enter(0);
    // Make enough iterations that the initiating test actually gets blocked
    for (int i = 0; i < testObj->GetIntVar2(); i++)
    {
        OSTimeDly(1);
        if (i == 10)
        {
            // Make sure we've done a few cycles before setting this to true
            testObj->SetBoolVar1(true);
        }
        testObj->SetIntVar1(testObj->GetIntVar1() + 1);
    }

    gCritSect.Leave();
}

void TestOsCritSecond(void *data)
{
    if (data == nullptr) { return; }
    TestObject *testObj = (TestObject *)data;

    // Wait for the main task to set it's bool and claim before this does, to test that the higher priority task (this one) will
    // get the critical section first, even though it claimed the section second
    while (!gTestObj.GetBoolVar2())
    {
        OSTimeDly(1);
    }

    gCritSect.Enter(0);
    // Test to make sure that the first task critical section completed before we were able to interrupt
    if (testObj->GetIntVar1() != 500) { testObj->SetByteVar1(1); }

    // Update the counter and spin through, so that we can test whether this task was interrupted during it's critical section
    for (int i = 0; i < testObj->GetIntVar2(); i++)
    {
        OSTimeDly(1);
        testObj->SetIntVar1(testObj->GetIntVar1() + 1);
    }

    gCritSect.Leave();
}

void TestOsCritTimeout(void *data)
{
    if (data == nullptr) { return; }
    TestObject *testObj = (TestObject *)data;

    gCritSect.Enter(0);
    testObj->SetBoolVar1(true);
    OSTimeDly(TICKS_PER_SECOND * 2);
    gCritSect.Leave();
}

void TestUserCrit(void *data)
{
    if (data == nullptr) { return; }
    TestObject *testObj = (TestObject *)data;

    USER_ENTER_CRITICAL();
    if (critical_count == 2) { testObj->SetBoolVar2(true); }
    testObj->SetBoolVar1(true);
    USER_EXIT_CRITICAL();
}

/**
 * @brief This test validates OSCritInit() and OSCritLeave().
 *
 * Tests that are completed here are as follows:
 *     - Test that OSCritInit() correctly initializes the critical section object and returns a valid value
 *     - Test that OSCritLeave() correctly fails when no sections are owned
 */
TEST(NbrtosTestCrit, CritInit)
{
    gParamData = &gTestObj;

    gResult = gCritSect.Init();
    CHECK_EQUAL(OS_NO_ERR, gResult);
    CHECK_EQUAL(0, gCritSect.OSCritDepthCount);

    gResult = gCritSect.Leave();
    CHECK_EQUAL(OS_CRIT_ERR, gResult);
}

/**
 * @brief This test validates the ability to nest OSCritEnter() and OSCritLeave().
 */
TEST(NbrtosTestCrit, CritEnterAndLeave)
{
    /**
     * Test correct functionality of OSCritEnter and OSCritLeave
     */
    gResult = gCritSect.Enter(0);
    CHECK_EQUAL(OS_NO_ERR, gResult);

    gResult = gCritSect.Enter(0);
    CHECK_EQUAL(OS_NO_ERR, gResult);

    gResult = gCritSect.Leave();
    CHECK_EQUAL(OS_NO_ERR, gResult);

    gResult = gCritSect.Leave();
    CHECK_EQUAL(OS_NO_ERR, gResult);
}

/**
 * @brief This set of tests validate the basic functionality of OSCritEnter() and OSCritLeave().
 *
 * Tests that are completed here are as follows:
 *     - Test that OSCritEnter() correctly claims a section of code, and blocks when it's unable to do so
 *     - Test that OSCritLeave() correctly unblocks other sections of code, and returns the appropriate value
 */
TEST(NbrtosTestCrit, CritBlock)
{
    gParamData = &gTestObj;

    /**
     * Test correct blocking functionality of OSCritEnter
     */
    gTestObj.SetIntVar1(0);     // Initial count
    gTestObj.SetIntVar2(500);   // How many times to iterate through the for loop
    OSTaskCreatewName(TestOsCrit, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack, gPriority + 3,
                      "TestOsCrit");

    while (!gTestObj.GetBoolVar1())
    {
        OSTimeDly(1);
    }

    gResult = gCritSect.Enter(0);
    CHECK_EQUAL(OS_NO_ERR, gResult);
    // By checking this value, we can ensure that the critical section ran uninterrupted through the loop in TestOsCrit
    CHECK_EQUAL(500, gTestObj.GetIntVar1());

    gTestObj.SetIntVar1(1000);
    for (int i = 0; i < 20; i++)
    {
        gTestObj.SetIntVar1(gTestObj.GetIntVar1() + 1);
    }
    gResult = gCritSect.Leave();
    CHECK_EQUAL(OS_NO_ERR,
                gResult);   // Leave this check to ensure that OSCritLeave() correctly functions when leaving a section that was blocked

    // By checking this value, we can ensure that the critical section above blocked until the other critical section
    // had finished
    CHECK_EQUAL(1020, gTestObj.GetIntVar1());
    OSTimeDly(TICKS_PER_SECOND * 2);   // Ensure delay task finishes before moving on
}

TEST(NbrtosTestCrit, CritBlockThreeTask)
{
    gParamData = &gTestObj;

    /**
     * Test correct blocking functionality of OSCritEnter
     */
    gTestObj.SetIntVar1(0);     // Initial count
    gTestObj.SetIntVar2(500);   // How many times to iterate through the for loop
    OSTaskCreatewName(TestOsCrit, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack, gPriority - 1,
                      "TestOsCrit");
    OSTaskCreatewName(TestOsCritSecond, &gTestObj, (void *)&gTestTaskStack2[USER_TASK_STK_SIZE], (void *)gTestTaskStack2, gPriority - 2,
                      "TestOsCritSecond");

    while (!gTestObj.GetBoolVar1())
    {
        OSTimeDly(1);
    }
    gTestObj.SetBoolVar2(true);

    gResult = gCritSect.Enter(0);
    CHECK_EQUAL(OS_NO_ERR, gResult);
    // By checking this value, we can ensure that the critical section ran uninterrupted through the loop in TestOsCrit and TestOsCritSecond
    CHECK_EQUAL(1000, gTestObj.GetIntVar1());
    CHECK_EQUAL(0, gTestObj.GetByteVar1());

    gTestObj.SetIntVar1(1000);
    for (int i = 0; i < 20; i++)
    {
        gTestObj.SetIntVar1(gTestObj.GetIntVar1() + 1);
    }
    gResult = gCritSect.Leave();
    CHECK_EQUAL(OS_NO_ERR,
                gResult);   // Leave this check to ensure that OSCritLeave() correctly functions when leaving a section that was blocked

    // By checking this value, we can ensure that the critical section above blocked until the other critical sections
    // had finished
    CHECK_EQUAL(1020, gTestObj.GetIntVar1());
    OSTimeDly(TICKS_PER_SECOND * 2);   // Ensure delay task finishes before moving on
}

/**
 * @brief This validates that OSCritEnter() will correctly timeout.
 *
 * @TODO Verify duration of timeout
 */
TEST(NbrtosTestCrit, CritTimeout)
{
    gParamData = &gTestObj;

    /**
     * Test timeout on OSCritEnter
     */
    OSTaskCreatewName(TestOsCritTimeout, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack, gPriority + 3,
                      "TestOsCritTimeout");

    while (!gTestObj.GetBoolVar1())
    {
        OSTimeDly(1);
    }

    gResult = gCritSect.Enter(TICKS_PER_SECOND);
    CHECK_EQUAL(OS_TIMEOUT, gResult);

    OSTimeDly(TICKS_PER_SECOND * 2);   // Ensure delay task finishes before moving on
}

/**
 * @brief This test validates that OSCritEnterNoWait() correctly claims a section of code.
 */
TEST(NbrtosTestCrit, CritEnterNoWait)
{
    /**
     * Test CritEnterNoWait() functionality
     */
    gResult = gCritSect.EnterNoWait();
    CHECK_EQUAL(OS_NO_ERR, gResult);

    gResult = gCritSect.EnterNoWait();
    CHECK_EQUAL(OS_NO_ERR, gResult);

    gResult = gCritSect.Leave();
    CHECK_EQUAL(OS_NO_ERR, gResult);

    gResult = gCritSect.Leave();
    CHECK_EQUAL(OS_NO_ERR, gResult);
}

/**
 * @brief This test verifies that OSCritEnterNoWait() fails with a timeout when it is unable to claim a section of code.
 */
TEST(NbrtosTestCrit, CritEnterNoWaitTimeout)
{
    gParamData = &gTestObj;

    /**
     * Test timeout for CritEnterNoWait()
     */
    OSTaskCreatewName(TestOsCritTimeout, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack, gPriority + 3,
                      "TestOsCritTimeout");

    while (!gTestObj.GetBoolVar1())
    {
        OSTimeDly(1);
    }

    gResult = gCritSect.EnterNoWait();
    CHECK_EQUAL(OS_TIMEOUT, gResult);

    OSTimeDly(TICKS_PER_SECOND * 2);   // Ensure delay task finishes before moving on

    iprintf("Test complete\r\n");
}

/**
 * @brief This set of tests verify the following macros: USER_ENTER_CRITICAL, USER_EXIT_CRITICAL
 *
 * Tests that are completed here are as follows:
 *     - Test that critical_count is properly incremented with USER_ENTER_CRITICAL
 *     - Test that critical_count is properly decremented with USER_EXIT_CRITICAL
 *     - Test that nesting USER_ENTER_CRITICAL and USER_EXIT_CRITICAL properly increments and decrements critical_count
 *     - Test that USER_ENTER_CRITICAL and USER_EXIT_CRITICAL function properly with scope
 */
TEST(NbrtosTestCrit, UserCrit)
{
    USER_ENTER_CRITICAL();
    CHECK_EQUAL(1, critical_count);
    USER_EXIT_CRITICAL();
    CHECK_EQUAL(0, critical_count);

    USER_ENTER_CRITICAL();
    USER_ENTER_CRITICAL();
    CHECK_EQUAL(2, critical_count);
    USER_EXIT_CRITICAL();
    CHECK_EQUAL(1, critical_count);
    USER_EXIT_CRITICAL();
    CHECK_EQUAL(0, critical_count);

    USER_ENTER_CRITICAL();
    {
        USER_ENTER_CRITICAL();
        CHECK_EQUAL(2, critical_count);
        USER_EXIT_CRITICAL();
        CHECK_EQUAL(1, critical_count);
    }
    USER_EXIT_CRITICAL();
    CHECK_EQUAL(0, critical_count);
}

/**
 * @brief This test verifies that USER_ENTER_CRITICAL is correctly maintained in different tasks
 */
TEST(NbrtosTestCrit, UserCritMultiTask)
{
    USER_ENTER_CRITICAL();

    OSTaskCreatewName(TestUserCrit, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack, gPriority,
                      "TestUserCrit");

    while (!gTestObj.GetBoolVar1())
    {
        OSTimeDly(1);
    }

    CHECK_EQUAL(true, gTestObj.GetBoolVar2());

    USER_EXIT_CRITICAL();
    CHECK_EQUAL(0, critical_count);
}
