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
 * @file nbrtosSem.cpp
 * @brief Unit tests for nbrtos semaphores
 *
 * This holds tests for the nbrtos semaphore objects.
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
extern TestObject gTestObj;

OS_SEM gSem;

/**
 * @brief The startup() and teardown() routines for the uCOS used to reinitialize variables and cleanup after
 * individual tests are run.
 */
TEST_GROUP(NbrtosTestSem){void setup(){// MemoryLeakWarningPlugin::turnOffNewDeleteOverloads();
                                       gSem.Init(0);
gTestObj.Reset();
gResult = OS_NO_ERR;
gParamData = nullptr;
gResultData = nullptr;
}

void teardown()
{
    // MemoryLeakWarningPlugin::turnOnNewDeleteOverloads();
    gParamData = nullptr;
    gResultData = nullptr;
}
}
;

void TestSemPendNoWait(void *data)
{
    if (data == nullptr) { return; }
    TestObject *testObj = (TestObject *)data;

    // Set byte 1 to the result of OSSemPendNoWait so it can be checked
    testObj->SetByteVar1(gSem.PendNoWait());
    OSTimeDly(TICKS_PER_SECOND * testObj->GetIntVar1());
    if (testObj->GetByteVar1() == OS_NO_ERR) { gSem.Post(); }
}

void TestSemPend(void *data)
{
    if (data == nullptr) { return; }
    TestObject *testObj = (TestObject *)data;
    int taskId = testObj->GetIntVar1();   // Task id used in some tests

    // Set byte 2 to the result of OSSemPend so it can be checked
    time(testObj->GetTimeVar1());
    testObj->SetByteVar2(gSem.Pend(TICKS_PER_SECOND * testObj->GetIntVar2()));
    time(testObj->GetTimeVar2());
    if (testObj->GetByteVar2() == OS_NO_ERR) { gSem.Post(); }
    else
    {
        testObj->SetBoolVar1(true);
    }

    testObj->SetIntVar3(taskId);
}

void TestSemPendRepeat(void *data)
{
    if (data == nullptr) { return; }
    TestObject *testObj = (TestObject *)data;
    int taskId = testObj->GetIntVar1();

    testObj->SetByteVar2(gSem.Pend(TICKS_PER_SECOND * testObj->GetIntVar2()));
    testObj->SetIntVar3(taskId);

    testObj->SetByteVar2(gSem.Pend(TICKS_PER_SECOND * testObj->GetIntVar2()));
    testObj->SetIntVar3(taskId + 1);
}

/**
 * @brief This set of tests check OSSemInit().
 *
 * Tests that are completed here are as follows:
 *     - Test that OSSemInit() fails with a negative count
 *     - Test that OSSemInit() correctly initializes semaphore with a valid count
 */
TEST(NbrtosTestSem, SemInit)
{
    // Initialize and verify the semaphore with a negative count fails
    gResult = gSem.Init(-1);
    CHECK_EQUAL(OS_SEM_ERR, gResult);

    // Initialize and verify the semaphore with a valid count
    gResult = gSem.Init(0);
    CHECK_EQUAL(OS_NO_ERR, gResult);
}

/**
 * @brief This set of tests check OSSemPost() and OSSemPend().
 *
 * Tests that are completed here are as follows:
 *     - Test that of OSSemPost() correctly posts and increments counter
 *     - Test that of OSSemPend() correctly pends and decrements counter
 */
TEST(NbrtosTestSem, SemPostPend)
{
    gSem.Init(0);

    gResult = gSem.Post();
    CHECK_EQUAL(OS_NO_ERR, gResult);
    CHECK_EQUAL(1, gSem.OSSemCnt);
    CHECK_EQUAL(0, gSem.OSSemUsed);

    gResult = gSem.Pend();
    CHECK_EQUAL(OS_NO_ERR, gResult);
    CHECK_EQUAL(1, gSem.OSSemCnt);
    CHECK_EQUAL(1, gSem.OSSemUsed);
}

/**
 * @brief This set of tests check OSSemPendNoWait().
 *
 * Tests that are completed here are as follows:
 *     - Test that OSPendWithNoWait() returns correct error when unable to pend
 */
TEST(NbrtosTestSem, SemPendNoWaitTimeout)
{
    // Object that stores test related data and essentially holds the state of the test
    gSem.Init(0);

    // Test that OS_TIMEOUT is returned for SemPendNoWait when trying to pend on a semaphore with a count of 0
    gResult = gSem.PendNoWait();
    CHECK_EQUAL(OS_TIMEOUT, gResult);
}

/**
 * @brief This set of tests check OSSemPendNoWait() and OSSemPend().
 *
 * Tests that are completed here are as follows:
 *     - Test that OSPendNoWait() can correctly grab and release semaphore
 *     - Test that OSSemPend() can grab and release semaphore after OSPendNoWait()
 */
TEST(NbrtosTestSem, SemTwoTaskPend)
{
    // The first task will call SemPendNoWait and hold it for 1 second, while the second task
    // will call SemPend and wait for 3 seconds.  Ensure that both succeed.
    gResult = gSem.Init(1);
    gTestObj.SetIntVar1(1);   // How long to grab semaphore
    gResult = OSTaskCreatewName(TestSemPendNoWait, gParamData, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack,
                                gPriority, "TestSemPendNoWait");
    CHECK_EQUAL(OS_NO_ERR, gResult);   // Make sure the task was created

    gTestObj.SetIntVar2(3);   // How long the timeout should be set for for the pend
    gResult = OSTaskCreatewName(TestSemPend, gParamData, (void *)&gTestTaskStack2[USER_TASK_STK_SIZE], (void *)gTestTaskStack2,
                                gPriority - 1, "TestSemPend");
    CHECK_EQUAL(OS_NO_ERR, gResult);   // Make sure the task was created

    OSTimeDly(TICKS_PER_SECOND * 5);   // Wait long enough to allow the first two tasks to finish
    CHECK_EQUAL(OS_NO_ERR, gTestObj.GetByteVar1());
    CHECK_EQUAL(OS_NO_ERR, gTestObj.GetByteVar2());
}

/**
 * @brief This set of tests check OSSemPendNoWait() and OSSemPend().
 *
 * Tests that are completed here are as follows:
 *     - Test that OSSemPend() will correctly return timeout when unable to pend past the timeout duration
 *     - Test that OSSemPend() will timeout after the specified duration
 */
TEST(NbrtosTestSem, SemTwoTaskPendTimeout)
{
    // Object that stores test related data and essentially holds the state of the test
    gParamData = &gTestObj;
    double timeDif = 0.0;

    gResult = gSem.Init(1);

    // The first task will call SemPendNoWait and hold it for 3 seconds, while the second task
    // will call SemPend and wait for 1 second.  Ensure that the first succeeds, and the second times out.
    gTestObj.SetIntVar1(3);   // How long the pend should hold
    gResult = OSTaskCreatewName(TestSemPendNoWait, gParamData, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack,
                                gPriority - 1, "TestSemPendNoWait");
    CHECK_EQUAL(OS_NO_ERR, gResult);   // Make sure the task was created

    gTestObj.SetIntVar2(1);   // How long the timeout should be set for for the pend
    gResult = OSTaskCreatewName(TestSemPend, gParamData, (void *)&gTestTaskStack2[USER_TASK_STK_SIZE], (void *)gTestTaskStack2, gPriority,
                                "TestSemPend");
    CHECK_EQUAL(OS_NO_ERR, gResult);   // Make sure the task was created

    OSTimeDly(TICKS_PER_SECOND * 5);   // Wait long enough to allow the first two tasks to finish
    CHECK_EQUAL(OS_NO_ERR, gTestObj.GetByteVar1());
    CHECK_EQUAL(OS_TIMEOUT, gTestObj.GetByteVar2());

    // Verify that the timeout held for the specified duration
    timeDif = difftime(*gTestObj.GetTimeVar2(), *gTestObj.GetTimeVar1());
    CHECK_EQUAL(1.0, timeDif);
}

/**
 * @brief This test verifies that that lower level tasks correctly pend through multiple pends of higher tasks.
 */
TEST(NbrtosTestSem, SemLowPrioPend)
{
    // Ensure that lower level tasks correctly pend through multiple pends of higher tasks
    gTestObj.SetIntVar1(1);    // Id for first task
    gTestObj.SetIntVar2(15);   // Timeout in seconds
    gResult = OSTaskCreatewName(TestSemPendRepeat, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack,
                                gPriority - 1, "TestSemPendRepeat");
    CHECK_EQUAL(OS_NO_ERR, gResult);   // Make sure the task was created
    OSTimeDly(1);

    gTestObj.SetIntVar1(5);   // Id for second task
    gResult = OSTaskCreatewName(TestSemPend, &gTestObj, (void *)&gTestTaskStack2[USER_TASK_STK_SIZE], (void *)gTestTaskStack2, gPriority,
                                "TestSemPend");
    CHECK_EQUAL(OS_NO_ERR, gResult);   // Make sure the task was created
    OSTimeDly(1);

    // Check that task with higher priority gets message first in both cases
    gResult = gSem.Post();
    CHECK_EQUAL(1, gTestObj.GetIntVar3());
    gResult = gSem.Post();
    CHECK_EQUAL(2, gTestObj.GetIntVar3());   // We increment the id by one on the second pend so we can distinguish between the two

    OSTimeDly(3);

    gResult = gSem.Post();
    CHECK_EQUAL(5, gTestObj.GetIntVar3());   // Check that task with higher priority gets message first
}

/**
 * @brief This test verifies that lower level tasks correctly timeout through multiple posts that are grabbed by higher priority pends.
 */
TEST(NbrtosTestSem, SemLowPrioPendTimeout)
{
    // Ensure that lower level tasks correctly timeout through multiple posts that are grabbed by higher priority pends
    double timeDif = 0.0;
    gTestObj.SetIntVar1(1);   // Id for first task
    gTestObj.SetIntVar2(1);   // Timeout in seconds
    gResult = OSTaskCreatewName(TestSemPendRepeat, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack,
                                gPriority - 1, "TestSemPendRepeat");
    CHECK_EQUAL(OS_NO_ERR, gResult);   // Make sure the task was created
    OSTimeDly(1);

    gTestObj.SetIntVar1(5);   // Id for second task
    gTestObj.SetIntVar2(4);   // Timeout in seconds
    gResult = OSTaskCreatewName(TestSemPend, &gTestObj, (void *)&gTestTaskStack2[USER_TASK_STK_SIZE], (void *)gTestTaskStack2, gPriority,
                                "TestSemPend");
    CHECK_EQUAL(OS_NO_ERR, gResult);   // Make sure the task was created
    OSTimeDly(1);

    // Check that task with higher priority gets message first in both cases
    gResult = gSem.Post();
    CHECK_EQUAL(1, gTestObj.GetIntVar3());
    gResult = gSem.Post();
    CHECK_EQUAL(2, gTestObj.GetIntVar3());   // We increment the id by one on the second pend so we can distinguish between the two

    // Wait for the lower priority task to timeout and ensure that it does so correctly
    while (gTestObj.GetBoolVar1() == false)
    {
        OSTimeDly(1);
    }
    CHECK_EQUAL(5, gTestObj.GetIntVar3());   // Check that task with higher priority gets message first
    CHECK(gResultData == nullptr);           // Check that we correctly received the timeout response

    // Verify that the timeout held for the specified duration
    timeDif = difftime(*gTestObj.GetTimeVar2(), *gTestObj.GetTimeVar1());
    CHECK_EQUAL(4.0, timeDif);
}

/**
 * @brief This set of tests check OSSemPendNoWait() and OSSemPend().
 *
 * Tests that are completed here are as follows:
 *     - Test the limits for member variables OSSemCnt and OSSemUsed
 */
TEST(NbrtosTestSem, SemLimits)
{
    gSem.Init(0);

    // Test that OSSemCnt and OSSemUsed correctly roll over when OSSemUsed hits 0x80000000
    gSem.OSSemCnt = 0xFFFFFFFF;
    gSem.OSSemUsed = 0xFFFFFFFE;

    int rv = gSem.OSSemCnt - gSem.OSSemUsed;
    gResult = gSem.Pend(TICKS_PER_SECOND);
    CHECK_EQUAL(OS_NO_ERR, gResult);
    CHECK_EQUAL(0xFFFFFFFF, gSem.OSSemUsed);
    CHECK_EQUAL(0xFFFFFFFF, gSem.OSSemCnt);

    // Check trying to post over limit
    gSem.Init(0);
    gSem.OSSemCnt = 0xFFFFFFFF;
    gSem.OSSemUsed = 0xFFFFFFFE;
    gResult = gSem.Post();   // Post() again to trigger rollover
    CHECK_EQUAL(OS_NO_ERR, gResult);

    // Pend(), which should work despite rollover of OSSemCnt because of check with modulodiff()
    gResult = gSem.Pend(TICKS_PER_SECOND);
    // CHECK_EQUAL(OS_SEM_OVF, gResult);
    CHECK_EQUAL(OS_NO_ERR, gResult);

    // Check trying to post over limit
    gSem.Init(0);
    gSem.OSSemCnt = 0xFFFFFFFF;
    gSem.OSSemUsed = 0x0;
    gResult = gSem.Post();   // Trigger rollover

    gResult = gSem.Pend(TICKS_PER_SECOND);   // Should be able to Pend(), but end up timing out
    iprintf("4) Count: %u (%d)     Used: %u (%d)\r\n", gSem.OSSemCnt, gSem.OSSemCnt, gSem.OSSemUsed, gSem.OSSemUsed);
    // This currently fails based on the design of OS_SEM. This will possibly get changed to prevent rollover.
    CHECK_EQUAL(OS_NO_ERR, gResult);
}

// TODO: Create two tasks that alternately pend and post, and check rollover condition
