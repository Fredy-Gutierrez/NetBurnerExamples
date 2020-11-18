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
 * @file nbrtosQue.cpp
 * @brief Unit tests for nbrtos queue object.
 *
 * This holds tests for the nbrtos queue object.
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
extern TestObject gTestObj2;
extern void *gQueueStorage;

OS_Q gQueue;

void TestFunction(void *data);

/**
 * @brief The startup() and teardown() routines for the uCOS used to reinitialize variables and cleanup after
 * individual tests are run.
 */
TEST_GROUP(NbrtosTestQueue){void setup(){// MemoryLeakWarningPlugin::turnOffNewDeleteOverloads();
                                         gQueue.Init(&gQueueStorage, 5);
gTestObj.Reset();
gTestObj2.Reset();
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

void TestQPend(void *data)
{
    if (data == nullptr) { return; }
    TestObject *testObj = (TestObject *)data;

    uint8_t err = OS_NO_ERR;
    int taskId = testObj->GetIntVar2();

    time(testObj->GetTimeVar1());
    gResultData = gQueue.Pend(TICKS_PER_SECOND * testObj->GetIntVar1(), err);
    if (gResultData != nullptr)
    {
        // Set the id to var 3 so we know which task completed the pend first for multi-task tests
        ((TestObject *)gResultData)->SetIntVar3(taskId);
    }
    else
    {
        testObj->SetIntVar3(taskId);
        testObj->SetBoolVar1(true);
    }

    time(testObj->GetTimeVar2());

    testObj->SetByteVar1(err);
}

void TestQPendRepeat(void *data)
{
    if (data == nullptr) { return; }
    TestObject *testObj = (TestObject *)data;
    int taskId = testObj->GetIntVar2();
    uint8_t err = OS_NO_ERR;

    gResultData = gQueue.Pend(TICKS_PER_SECOND * testObj->GetIntVar1(), err);
    testObj->SetIntVar3(taskId);

    gResultData = gQueue.Pend(TICKS_PER_SECOND * testObj->GetIntVar1(), err);
    testObj->SetIntVar3(taskId + 1);

    testObj->SetByteVar1(err);
}

void TestQPendNoWait(void *data)
{
    if (data == nullptr) { return; }
    TestObject *testObj = (TestObject *)data;

    uint8_t err = OS_NO_ERR;
    gResultData = gQueue.PendNoWait(err);
    testObj->SetByteVar1(err);
}

/**
 * @brief This set of tests check OSQInit.
 *
 * Tests that are completed here are as follows:
 *     - Test that OSQInit() fails if queue is not provided
 *     - Test that OSQInit() fails if queue storage is not provided
 *     - Test that OSQInit() fails if queue an invalid storage size is provided
 *     - Test that OSQInit() properly initializes
 */
TEST(NbrtosTestQueue, QueueInit)
{
    // Verify QInit fails with a nullptr queue storage
    gResult = gQueue.Init(nullptr, 5);
    CHECK_EQUAL(OS_CRIT_ERR, gResult);

    // Verify QInit fails with an invalid size
    gResult = gQueue.Init(&gQueueStorage, 0);
    CHECK_EQUAL(OS_CRIT_ERR, gResult);

    // Verify QInit succeeds with correct parameters
    gResult = gQueue.Init(&gQueueStorage, 5);
    CHECK_EQUAL(OS_NO_ERR, gResult);
}

/**
 * @brief This set of tests check OSQPost() and OSQPend().
 *
 * Tests that are completed here are as follows:
 *     - Test that OSQPost() properly puts a message in the queue
 *     - Test that OSQPend() properly pulls a message from the queue
 */
TEST(NbrtosTestQueue, QueuePostPend)
{
    // Verify QPost
    gResult = gQueue.Post(&gTestObj);
    CHECK_EQUAL(OS_NO_ERR, gResult);

    // Create pending task that has valid message posted
    gTestObj.SetIntVar1(1);   // Timeout in seconds
    gResult = OSTaskCreatewName(TestQPend, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack, gPriority,
                                "TestQPend");
    CHECK_EQUAL(OS_NO_ERR, gResult);   // Make sure the task was created
    OSTimeDly(TICKS_PER_SECOND * 2);

    CHECK_EQUAL(OS_NO_ERR, gTestObj.GetByteVar1());   // Ensure that the result returned from QPend was success
    CHECK(gResultData == &gTestObj);                  // Verify that the message received was the test object
}

/**
 * @brief This test verifies that OSQPend() properly timeouts when no message is available after the specified duration.
 */
TEST(NbrtosTestQueue, QueuePostTimeout)
{
    double timeDif;

    // Create pending task that should timeout
    gTestObj.SetIntVar1(1);   // Timeout in seconds
    gResult = OSTaskCreatewName(TestQPend, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack, gPriority,
                                "TestQPend");
    CHECK_EQUAL(OS_NO_ERR, gResult);   // Make sure the task was created
    OSTimeDly(TICKS_PER_SECOND * 2);

    CHECK_EQUAL(OS_TIMEOUT, gTestObj.GetByteVar1());   // Ensure the QPend timed out
    CHECK(gResultData == nullptr);                     // Ensure the message we got back was NULL

    timeDif = difftime(*gTestObj.GetTimeVar2(), *gTestObj.GetTimeVar1());
    CHECK_EQUAL(1.0, timeDif);   // Verify that the timeout held for the specified duration
}

/**
 * @brief This test verifies that OSQPendNoWait() properly pulls a message from the queue.
 */
TEST(NbrtosTestQueue, QueuePendNoWait)
{
    // Post message for QPendNoWait test
    gResult = gQueue.Post(&gTestObj);
    CHECK_EQUAL(OS_NO_ERR, gResult);

    gResult = OSTaskCreatewName(TestQPendNoWait, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack, gPriority,
                                "TestQPendNoWait");
    CHECK_EQUAL(OS_NO_ERR, gResult);

    CHECK_EQUAL(OS_NO_ERR, gTestObj.GetByteVar1());   // Ensure the QPendNoWait was successful
    CHECK(gResultData == &gTestObj);                  // Verify that the message received was the test object
}

/**
 * @brief This test verifies that OSQPendNoWait() properly fails if no message is avaiable.
 */
TEST(NbrtosTestQueue, QueuePendNoWaitFail)
{
    // Create a pending no wait task for an empty queue and ensure that it it fails
    gResult = OSTaskCreatewName(TestQPendNoWait, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack, gPriority,
                                "TestQPendNoWait");
    CHECK_EQUAL(OS_NO_ERR, gResult);

    CHECK_EQUAL(OS_TIMEOUT, gTestObj.GetByteVar1());   // Ensure the QPend timed out
    CHECK(gResultData == nullptr);                     // Verify that the message received was the test object
}

/**
 * @brief This test verifies that that posted messages are received in the correct order.
 */
TEST(NbrtosTestQueue, QueuePostOrder)
{
    // Post two messages, and ensure you can get them back in the correct order
    gQueue.Post(&gTestObj);
    gQueue.Post(&gTestObj2);

    // Get first message
    gResult = OSTaskCreatewName(TestQPendNoWait, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack, gPriority,
                                "TestQPendNoWait");
    CHECK_EQUAL(OS_NO_ERR, gResult);

    CHECK_EQUAL(OS_NO_ERR, gTestObj.GetByteVar1());   // Ensure the QPend timed out
    CHECK(gResultData == &gTestObj);                  // Verify that the message received was object pushed with OSQPostFirst

    // Get second message
    gResult = OSTaskCreatewName(TestQPendNoWait, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack, gPriority,
                                "TestQPendNoWait");
    CHECK_EQUAL(OS_NO_ERR, gResult);

    CHECK_EQUAL(OS_NO_ERR, gTestObj2.GetByteVar1());   // Ensure the QPend timed out
    CHECK(gResultData == &gTestObj2);                  // Verify that the message received was object pushed with OSQPost
}

/**
 * @brief This test verifies that OSQPostFirst() properly puts a message at the front of the queue.
 */
TEST(NbrtosTestQueue, QueuePostFirst)
{
    // Post two messages, ensure that OSQPostFirst supersedes OSQPost
    gQueue.Post(&gTestObj);
    gResult = gQueue.PostFirst(&gTestObj2);
    CHECK_EQUAL(OS_NO_ERR, gResult);

    // Get first message and verify that it is object from OSQPostFirst()
    gResult = OSTaskCreatewName(TestQPendNoWait, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack, gPriority,
                                "TestQPendNoWait");
    CHECK_EQUAL(OS_NO_ERR, gResult);

    CHECK_EQUAL(OS_NO_ERR, gTestObj2.GetByteVar1());   // Ensure the QPend timed out
    CHECK(gResultData == &gTestObj2);                  // Verify that the message received was object pushed with OSQPostFirst

    // Get second message
    gResult = OSTaskCreatewName(TestQPendNoWait, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack, gPriority,
                                "TestQPendNoWait");
    CHECK_EQUAL(OS_NO_ERR, gResult);

    CHECK_EQUAL(OS_NO_ERR, gTestObj.GetByteVar1());   // Ensure the QPend timed out
    CHECK(gResultData == &gTestObj);                  // Verify that the message received was object pushed with OSQPost
}

/**
 * @brief This test verifies that posting and receiving multiple messages happens in the correct order.
 */
TEST(NbrtosTestQueue, QueuePostPriority)
{
    // Have two tasks pend of different priorities (higher priority pends last), post two messages, and ensure they both receive the correct
    // one
    gTestObj.SetIntVar1(2);   // Timeout in seconds
    gTestObj.SetIntVar2(1);   // Id for first task
    gResult = OSTaskCreatewName(TestQPend, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack, gPriority,
                                "TestQPend");
    CHECK_EQUAL(OS_NO_ERR, gResult);   // Make sure the task was created

    gTestObj.SetIntVar2(2);   // Id for second task
    gResult = OSTaskCreatewName(TestQPend, &gTestObj, (void *)&gTestTaskStack2[USER_TASK_STK_SIZE], (void *)gTestTaskStack2, gPriority - 1,
                                "TestQPend");
    CHECK_EQUAL(OS_NO_ERR, gResult);   // Make sure the task was created
    OSTimeDly(1);

    gResult = gQueue.Post(&gTestObj);
    CHECK_EQUAL(2, ((TestObject *)gResultData)->GetIntVar3());   // Check that task with higher priority gets message first
    gResult = gQueue.Post(&gTestObj);
    CHECK_EQUAL(1, ((TestObject *)gResultData)->GetIntVar3());
}

/**
 * @brief This test verifies that that lower level tasks correctly pend through multiple pends of higher tasks.
 */
TEST(NbrtosTestQueue, QueueLowPrioPend)
{
    // Ensure that lower level tasks correctly pend through multiple pends of higher tasks
    gTestObj.SetIntVar1(15);   // Timeout in seconds
    gTestObj.SetIntVar2(1);    // Id for first task
    gResult = OSTaskCreatewName(TestQPendRepeat, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack,
                                gPriority - 1, "TestQPendRepeat");
    CHECK_EQUAL(OS_NO_ERR, gResult);   // Make sure the task was created
    OSTimeDly(1);

    gTestObj.SetIntVar2(5);   // Id for second task
    gResult = OSTaskCreatewName(TestQPend, &gTestObj, (void *)&gTestTaskStack2[USER_TASK_STK_SIZE], (void *)gTestTaskStack2, gPriority,
                                "TestQPend");
    CHECK_EQUAL(OS_NO_ERR, gResult);   // Make sure the task was created
    OSTimeDly(1);

    // Check that task with higher priority gets message first in both cases
    gResult = gQueue.Post(&gTestObj);
    CHECK_EQUAL(1, ((TestObject *)gResultData)->GetIntVar3());
    gResult = gQueue.Post(&gTestObj);
    CHECK_EQUAL(
        2,
        ((TestObject *)gResultData)->GetIntVar3());   // We increment the id by one on the second pend so we can distinguish between the two

    OSTimeDly(3);

    gResult = gQueue.Post(&gTestObj);
    CHECK_EQUAL(5, ((TestObject *)gResultData)->GetIntVar3());   // Check that task with higher priority gets message first
}

/**
 * @brief This test verifies that lower level tasks correctly timeout through multiple posts that are grabbed by higher priority pends.
 */
TEST(NbrtosTestQueue, QueueLowPrioPendTimeout)
{
    // Ensure that lower level tasks correctly timeout through multiple posts that are grabbed by higher priority pends
    double timeDif = 0.0;
    gTestObj.SetIntVar1(1);   // Timeout in seconds
    gTestObj.SetIntVar2(1);   // Id for first task
    gResult = OSTaskCreatewName(TestQPendRepeat, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack,
                                gPriority - 1, "TestQPendRepeat");
    CHECK_EQUAL(OS_NO_ERR, gResult);   // Make sure the task was created
    OSTimeDly(1);

    gTestObj.SetIntVar1(4);   // Timeout in seconds
    gTestObj.SetIntVar2(5);   // Id for second task
    gResult = OSTaskCreatewName(TestQPend, &gTestObj, (void *)&gTestTaskStack2[USER_TASK_STK_SIZE], (void *)gTestTaskStack2, gPriority,
                                "TestQPend");
    CHECK_EQUAL(OS_NO_ERR, gResult);   // Make sure the task was created
    OSTimeDly(1);

    // Check that task with higher priority gets message first in both cases
    gResult = gQueue.Post(&gTestObj);
    CHECK_EQUAL(1, ((TestObject *)gResultData)->GetIntVar3());
    gResult = gQueue.Post(&gTestObj);
    CHECK_EQUAL(
        2,
        ((TestObject *)gResultData)->GetIntVar3());   // We increment the id by one on the second pend so we can distinguish between the two

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
