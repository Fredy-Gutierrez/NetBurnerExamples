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
 * @file nbrtosMBox.cpp
 * @brief Unit tests for nbrtos mailbox object.
 *
 * This holds tests for the nbrtos mailbox object.
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
extern void *gResultData;
extern TestObject gTestObj;

OS_MBOX gMailBox;

/**
 * @brief The startup() and teardown() routines for the uCOS used to reinitialize variables and cleanup after
 * individual tests are run.
 */
TEST_GROUP(NbrtosTestMbox){void setup(){// MemoryLeakWarningPlugin::turnOffNewDeleteOverloads();
                                        gMailBox.Init(nullptr);
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

void TestMboxPend(void *data)
{
    if (data == nullptr) { return; }
    TestObject *testObj = (TestObject *)data;

    uint8_t err = OS_NO_ERR;
    int taskId = testObj->GetIntVar2();
    time(testObj->GetTimeVar1());
    gResultData = gMailBox.Pend(TICKS_PER_SECOND * testObj->GetIntVar1(), err);
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

void TestMboxPendRepeat(void *data)
{
    if (data == nullptr) { return; }
    TestObject *testObj = (TestObject *)data;

    uint8_t err = OS_NO_ERR;
    int taskId = testObj->GetIntVar2();

    gResultData = gMailBox.Pend(TICKS_PER_SECOND * testObj->GetIntVar1(), err);
    testObj->SetIntVar3(taskId);

    gResultData = gMailBox.Pend(TICKS_PER_SECOND * testObj->GetIntVar1(), err);
    testObj->SetIntVar3(taskId + 1);

    testObj->SetByteVar1(err);
}

void TestMboxPendNoWait(void *data)
{
    if (data == nullptr) { return; }
    TestObject *testObj = (TestObject *)data;

    uint8_t err = OS_NO_ERR;
    gResultData = gMailBox.PendNoWait(err);
    testObj->SetByteVar1(err);
}

/**
 * @brief This set of tests check OSMboxInit().
 *
 * Tests that are completed here are as follows:
 *     - Verify that the OSMboxInit() fails to initialize without the mailbox
 *     - Verify that the OSMboxInit() initializes correctly
 */
TEST(NbrtosTestMbox, MboxInit)
{
    // Object that stores test related data and essentially holds the state of the test
    gParamData = &gTestObj;

    // Verify MboxInit
    gResult = gMailBox.Init(nullptr);
    CHECK_EQUAL(OS_NO_ERR, gResult);
}

/**
 * @brief This set of tests check OSMboxPost() and OSMboxPend().
 *
 * Tests that are completed here are as follows:
 *     - Verify that OSMboxPost() functions as expected
 *     - Verify that OSMboxPend() with an available message returns `OS_NO_ERR` and points to the message
 */
TEST(NbrtosTestMbox, MboxPostPend)
{
    // Verify MboxPosts
    gResult = gMailBox.Post(&gTestObj);
    CHECK_EQUAL(OS_NO_ERR, gResult);

    // Create a pending task after posting our test object to the mailbox
    gTestObj.SetIntVar1(1);   // Timeout in seconds
    gResult = OSTaskCreatewName(TestMboxPend, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack, gPriority,
                                "TestMboxPend");
    CHECK_EQUAL(OS_NO_ERR, gResult);   // Make sure the task was created
    OSTimeDly(TICKS_PER_SECOND);

    CHECK_EQUAL(OS_NO_ERR, gTestObj.GetByteVar1());   // Ensure that the result returned from MboxPend was success
    CHECK(gResultData == &gTestObj);                  // Verify that the message received was the test object
}

/**
 * @brief This test verifies that MboxPend() without an available message returns `OS_TIMEOUT` after the
 *       specified duration.
 */
TEST(NbrtosTestMbox, MboxPendTimeout)
{
    double timeDif = 0.0;

    // Create a pending task for an empty mailbox and ensure that it times out
    gTestObj.SetIntVar1(1);   // Timeout in seconds
    gResult = OSTaskCreatewName(TestMboxPend, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack, gPriority,
                                "TestMboxPend");
    CHECK_EQUAL(OS_NO_ERR, gResult);   // Make sure the task was created
    OSTimeDly(TICKS_PER_SECOND * 2);

    CHECK_EQUAL(OS_TIMEOUT, gTestObj.GetByteVar1());   // Ensure the MboxPend timed out
    CHECK(gResultData == nullptr);                     // Ensure the message we got back was NULL

    timeDif = difftime(*gTestObj.GetTimeVar2(), *gTestObj.GetTimeVar1());
    CHECK_EQUAL(1.0, timeDif);   // Verify that the timeout actually held for the specified duration
}

/**
 * @brief This test verifies that MboxPendNoWait() with an available message returns `OS_NO_ERR` and points
 *       to the message.
 */
TEST(NbrtosTestMbox, MboxPendNoWait)
{
    // Post a message to the mailbox, create a pending no wait task for a mailbox and ensure that it is successful
    gResult = gMailBox.Post(&gTestObj);
    gResult = OSTaskCreatewName(TestMboxPendNoWait, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack,
                                gPriority, "TestMboxPendNoWait");
    CHECK_EQUAL(OS_NO_ERR, gResult);   // Make sure the task was created
    OSTimeDly(TICKS_PER_SECOND);

    CHECK_EQUAL(OS_NO_ERR, gTestObj.GetByteVar1());   // Ensure that the result returned from MboxPend was success
    CHECK(gResultData == &gTestObj);                  // Verify that the message received was the test object
}

/**
 * @brief This test verifies that MboxPendNoWait() without an available message returns `OS_TIMEOUT`.
 */
TEST(NbrtosTestMbox, MboxPendNoWaitTimeout)
{
    // Create a pending no wait task for an empty mailbox and ensure that it it fails
    gResult = OSTaskCreatewName(TestMboxPendNoWait, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack,
                                gPriority, "TestMboxPendNoWait");
    CHECK_EQUAL(OS_NO_ERR, gResult);   // Make sure the task was created
    OSTimeDly(TICKS_PER_SECOND);

    CHECK_EQUAL(OS_TIMEOUT, gTestObj.GetByteVar1());   // Ensure the MboxPendNoWait fails
    CHECK(gResultData == nullptr);                     // Ensure the message we got back was NULL
}

/**
 * @brief This test Verifies that two tasks using MboxPend() receive messages in the correct order as dictated by their priority.
 */
TEST(NbrtosTestMbox, MboxPendPriority)
{
    // Have two tasks pend of different priorities (higher priority pends last), post two messages, and ensure they both receive the correct
    // one
    gTestObj.SetIntVar1(2);   // Timeout in seconds
    gTestObj.SetIntVar2(1);   // Id for first task
    gResult = OSTaskCreatewName(TestMboxPend, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack, gPriority,
                                "TestMboxPend");
    CHECK_EQUAL(OS_NO_ERR, gResult);   // Make sure the task was created

    gTestObj.SetIntVar2(2);   // Id for second task
    gResult = OSTaskCreatewName(TestMboxPend, &gTestObj, (void *)&gTestTaskStack2[USER_TASK_STK_SIZE], (void *)gTestTaskStack2,
                                gPriority - 1, "TestMboxPend");
    CHECK_EQUAL(OS_NO_ERR, gResult);   // Make sure the task was created
    OSTimeDly(1);

    gResult = gMailBox.Post(&gTestObj);
    CHECK_EQUAL(2, ((TestObject *)gResultData)->GetIntVar3());   // Check that task with higher priority gets message first
    gResult = gMailBox.Post(&gTestObj);
    CHECK_EQUAL(1, ((TestObject *)gResultData)->GetIntVar3());
}

/**
 * @brief This test verifies that lower level tasks correctly pend through multiple pends of higher tasks
 */
TEST(NbrtosTestMbox, MboxLowPrioPend)
{
    gTestObj.SetIntVar1(15);   // Timeout in seconds
    gTestObj.SetIntVar2(1);    // Id for first task
    gResult = OSTaskCreatewName(TestMboxPendRepeat, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack,
                                gPriority - 1, "TestMboxPendRepeat");
    CHECK_EQUAL(OS_NO_ERR, gResult);   // Make sure the task was created
    OSTimeDly(1);

    gTestObj.SetIntVar2(5);   // Id for second task
    gResult = OSTaskCreatewName(TestMboxPend, &gTestObj, (void *)&gTestTaskStack2[USER_TASK_STK_SIZE], (void *)gTestTaskStack2, gPriority,
                                "TestMboxPend");
    CHECK_EQUAL(OS_NO_ERR, gResult);   // Make sure the task was created
    OSTimeDly(1);

    // Check that task with higher priority gets message first in both cases
    gResult = gMailBox.Post(&gTestObj);
    CHECK_EQUAL(1, ((TestObject *)gResultData)->GetIntVar3());
    gResult = gMailBox.Post(&gTestObj);
    CHECK_EQUAL(
        2,
        ((TestObject *)gResultData)->GetIntVar3());   // We increment the id by one on the second pend so we can distinguish between the two

    OSTimeDly(3);

    gResult = gMailBox.Post(&gTestObj);
    CHECK_EQUAL(5, ((TestObject *)gResultData)->GetIntVar3());   // Check that task with higher priority gets message first
}

/**
 * @brief This test verifies that lower level tasks correctly timeout through multiple posts that are grabbed by higher priority pends.
 */
TEST(NbrtosTestMbox, MboxLowPrioPendTimeout)
{
    double timeDif = 0.0;

    gTestObj.SetIntVar1(1);   // Timeout in seconds
    gTestObj.SetIntVar2(1);   // Id for first task
    gResult = OSTaskCreatewName(TestMboxPendRepeat, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack,
                                gPriority - 1, "TestMboxPendRepeat");
    CHECK_EQUAL(OS_NO_ERR, gResult);   // Make sure the task was created
    OSTimeDly(1);

    gTestObj.SetIntVar1(4);   // Timeout in seconds
    gTestObj.SetIntVar2(5);   // Id for second task
    gResult = OSTaskCreatewName(TestMboxPend, &gTestObj, (void *)&gTestTaskStack2[USER_TASK_STK_SIZE], (void *)gTestTaskStack2, gPriority,
                                "TestMboxPend");
    CHECK_EQUAL(OS_NO_ERR, gResult);   // Make sure the task was created
    OSTimeDly(1);

    // Check that task with higher priority gets message first in both cases
    gResult = gMailBox.Post(&gTestObj);
    CHECK_EQUAL(1, ((TestObject *)gResultData)->GetIntVar3());
    gResult = gMailBox.Post(&gTestObj);
    CHECK_EQUAL(
        2,
        ((TestObject *)gResultData)->GetIntVar3());   // We increment the id by one on the second pend so we can distinguish between the two

    // Wait for the lower priority task to time out and ensure that it does so correctly
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
