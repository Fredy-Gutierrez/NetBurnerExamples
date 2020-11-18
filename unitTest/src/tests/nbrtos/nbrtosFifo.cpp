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
 * @file nbrtosFifo.cpp
 * @brief Unit tests for the nbrtos fifo object.
 *
 * This holds tests for the nbrtos fifo object.
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

OS_FIFO gFifo;

typedef struct
{
    void *pUsedByFifo;
} FifoStruct;

/**
 * @brief The startup() and teardown() routines for the uCOS used to reinitialize variables and cleanup after
 * individual tests are run.
 */
TEST_GROUP(NbrtosTestFifo){void setup(){// MemoryLeakWarningPlugin::turnOffNewDeleteOverloads();
                                        gFifo.Init();
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

void TestFifoPend(void *data)
{
    if (data == nullptr) { return; }
    uint8_t err = OS_NO_ERR;
    TestObject *testObj = (TestObject *)data;
    int taskId = testObj->GetIntVar2();
    time(testObj->GetTimeVar1());
    gResultData = (FifoStruct *)gFifo.Pend(TICKS_PER_SECOND * testObj->GetIntVar1(), err);
    if (gResultData == nullptr) { testObj->SetBoolVar1(true); }
    // Set the id to var 3 so we know which task completed the pend first for multi-task tests
    testObj->SetIntVar3(taskId);

    testObj->SetByteVar1(err);
    time(testObj->GetTimeVar2());
}

void TestFifoPendRepeat(void *data)
{
    if (data == nullptr) { return; }
    uint8_t err = OS_NO_ERR;
    TestObject *testObj = (TestObject *)data;
    int taskId = testObj->GetIntVar2();

    gResultData = (FifoStruct *)gFifo.Pend(TICKS_PER_SECOND * testObj->GetIntVar1(), err);
    // Set the id to var 3 so we know which task completed the pend first for multi-task tests
    ((TestObject *)testObj)->SetIntVar3(taskId);

    gResultData = (FifoStruct *)gFifo.Pend(TICKS_PER_SECOND * testObj->GetIntVar1(), err);
    // Set the id to var 3 so we know which task completed the pend first for multi-task tests
    ((TestObject *)testObj)->SetIntVar3(taskId + 1);

    testObj->SetByteVar1(err);
}

void TestFifoPendNoWait(void *data)
{
    uint8_t err = OS_NO_ERR;
    TestObject *testObj = (TestObject *)data;
    gResultData = (FifoStruct *)gFifo.PendNoWait(err);
    testObj->SetByteVar1(err);
}

/**
 * @brief This set of tests check OSFifoInit().
 *
 * Tests that are completed here are as follows:
 *     - Test that OSFifoInit() fails without proper FIFO
 *     - Test that OSFifoInit() properly initializes
 */
TEST(NbrtosTestFifo, FifoInit)
{
    // Test successful init
    gResult = gFifo.Init();
    CHECK_EQUAL(OS_NO_ERR, gResult);
}

/**
 * @brief This set of tests check OSFifoPend() and OSFifoPend().
 *
 * Tests that are completed here are as follows:
 *     - Test that OSFifoPost() properly puts a struct in the FIFO
 *     - Test that OSFifoPend() properly pulls a struct from the queue
 */
TEST(NbrtosTestFifo, FifoPostPend)
{
    FifoStruct fifoStruct;

    // Test successful post
    gResult = gFifo.Post((OS_FIFO_EL *)&fifoStruct);
    CHECK_EQUAL(OS_NO_ERR, gResult);

    // Post to FIFO, and ensure OSFifoPend() returns proper structure
    gTestObj.SetIntVar1(1);   // Timeout in seconds
    gResult = OSTaskCreatewName(TestFifoPend, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack, gPriority,
                                "TestFifoPend");
    CHECK_EQUAL(OS_NO_ERR, gResult);
    OSTimeDly(TICKS_PER_SECOND);

    CHECK(gResultData == &fifoStruct);   // gResultData is assigned in testFifoPend()
}

/**
 * @brief This set of tests check the error conditions of OSFifoPend().
 *
 * Tests that are completed here are as follows:
 *     - Test that OSFifoPend() properly timesout when no struct is available after the specified duration
 *     - Test that OSFifoPend() properly returns null when no struct is available
 */
TEST(NbrtosTestFifo, FifoPendTimeout)
{
    double timeDif = 0.0;

    // Ensure that OSFifoPend() returns null and waits proper amount of time when nothing has been posted
    gTestObj.SetIntVar1(1);   // Timeout in seconds
    gResult = OSTaskCreatewName(TestFifoPend, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack, gPriority,
                                "TestFifoPend");
    CHECK_EQUAL(OS_NO_ERR, gResult);
    OSTimeDly(TICKS_PER_SECOND * 2);

    timeDif = difftime(*gTestObj.GetTimeVar2(), *gTestObj.GetTimeVar1());
    CHECK_EQUAL(1.0, timeDif);       // Verify that the timeout held for the specified duration
    CHECK(gResultData == nullptr);   // gResultData is assigned in testFifoPend()
}

/**
 * @brief This test verifies OSFifoPendNoWait() functions correctly.
 */
TEST(NbrtosTestFifo, FifoPendNoWait)
{
    FifoStruct fifoStruct;

    // Post to FIFO, and ensure OSFifoPendNoWait() returns proper structure
    gFifo.Post((OS_FIFO_EL *)&fifoStruct);
    gResult = OSTaskCreatewName(TestFifoPendNoWait, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack,
                                gPriority, "TestFifoPendNoWait");
    CHECK_EQUAL(OS_NO_ERR, gResult);

    CHECK(gResultData == &fifoStruct);   // gResultData is assigned in testFifoPendNoWait()
}

/**
 * @brief This test verifies that OSFifoPendNoWait() properly fails if no struct is avaiable.
 */
TEST(NbrtosTestFifo, FifoPendNoWaitFail)
{
    // Ensure OSFifoPendNoWait() returns null when nothing has been posted
    gResult = OSTaskCreatewName(TestFifoPendNoWait, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack,
                                gPriority, "TestFifoPendNoWait");
    CHECK_EQUAL(OS_NO_ERR, gResult);

    CHECK(gResultData == nullptr);   // gResultData is assigned in testFifoPendNoWait()
}

/**
 * @brief This test verifies that OSFifoPostFirst() properly puts a struct at the front of the FIFO.
 */
TEST(NbrtosTestFifo, FifoPostFirst)
{
    FifoStruct fifoStruct;
    FifoStruct fifoStruct2;

    // Post two structs, ensure that OSFifoPostFirst supersedes OSFifoPost
    gFifo.Post((OS_FIFO_EL *)&fifoStruct);
    gResult = gFifo.PostFirst((OS_FIFO_EL *)&fifoStruct2);
    CHECK_EQUAL(OS_NO_ERR, gResult);

    gResult = OSTaskCreatewName(TestFifoPendNoWait, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack,
                                gPriority, "TestFifoPendNoWait");
    CHECK_EQUAL(OS_NO_ERR, gResult);
    CHECK(gResultData == &fifoStruct2);   // gResultData is assigned in testFifoPendNoWait()
    OSTimeDly(TICKS_PER_SECOND);

    gResult = OSTaskCreatewName(TestFifoPendNoWait, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack,
                                gPriority, "TestFifoPendNoWait");
    CHECK_EQUAL(OS_NO_ERR, gResult);
    CHECK(gResultData == &fifoStruct);   // gResultData is assigned in testFifoPendNoWait()
}

/**
 * @brief This test verifies that posting and receiving multiple structs happens in the correct order.
 */
TEST(NbrtosTestFifo, FifoPostOrder)
{
    FifoStruct fifoStruct;
    FifoStruct fifoStruct2;

    // Post two structs, ensure that they come out in the right order
    gFifo.Post((OS_FIFO_EL *)&fifoStruct);
    gFifo.Post((OS_FIFO_EL *)&fifoStruct2);
    CHECK_EQUAL(OS_NO_ERR, gResult);

    gResult = OSTaskCreatewName(TestFifoPendNoWait, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack,
                                gPriority, "TestFifoPendNoWait");
    CHECK_EQUAL(OS_NO_ERR, gResult);
    CHECK(gResultData == &fifoStruct);   // gResultData is assigned in testFifoPendNoWait()
    OSTimeDly(TICKS_PER_SECOND);

    gResult = OSTaskCreatewName(TestFifoPendNoWait, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack,
                                gPriority, "TestFifoPendNoWait");
    CHECK_EQUAL(OS_NO_ERR, gResult);
    CHECK(gResultData == &fifoStruct2)   // gResultData is assigned in testFifoPendNoWait()
}

/**
 * @brief This test verifies that two tasks using OSFifoPend() receive structs in the correct order as dictated by their priority.
 */
TEST(NbrtosTestFifo, FifoPostOrderPriority)
{
    FifoStruct fifoStruct;
    FifoStruct fifoStruct2;

    // Have two tasks pend of different priorities (higher priority pends last), post two messages, and ensure they both receive the correct
    // one
    gTestObj.SetIntVar1(2);   // Timeout in seconds
    gTestObj.SetIntVar2(1);   // Id for first task
    gResult = OSTaskCreatewName(TestFifoPend, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack, gPriority,
                                "TestFifoPend");
    CHECK_EQUAL(OS_NO_ERR, gResult);   // Make sure the task was created

    gTestObj.SetIntVar2(2);   // Id for second task
    gResult = OSTaskCreatewName(TestFifoPend, &gTestObj, (void *)&gTestTaskStack2[USER_TASK_STK_SIZE], (void *)gTestTaskStack2,
                                gPriority - 1, "TestFifoPend");
    CHECK_EQUAL(OS_NO_ERR, gResult);   // Make sure the task was created
    OSTimeDly(1);

    gResult = gFifo.Post((OS_FIFO_EL *)&fifoStruct);
    CHECK_EQUAL(2, gTestObj.GetIntVar3());   // Check that task with higher priority gets message first
    gResult = gFifo.Post((OS_FIFO_EL *)&fifoStruct2);
    CHECK_EQUAL(1, gTestObj.GetIntVar3());
}

/**
 * @brief This test verifies that lower level tasks correctly pend through multiple pends of higher tasks.
 */
TEST(NbrtosTestFifo, FifoLowPrioPend)
{
    FifoStruct fifoStruct;
    FifoStruct fifoStruct2;

    gTestObj.SetIntVar1(15);   // Timeout in seconds
    gTestObj.SetIntVar2(1);    // Id for first task
    gResult = OSTaskCreatewName(TestFifoPendRepeat, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack,
                                gPriority - 1, "TestFifoPendRepeat");
    CHECK_EQUAL(OS_NO_ERR, gResult);   // Make sure the task was created
    OSTimeDly(1);

    gTestObj.SetIntVar2(5);   // Id for second task
    gResult = OSTaskCreatewName(TestFifoPend, &gTestObj, (void *)&gTestTaskStack2[USER_TASK_STK_SIZE], (void *)gTestTaskStack2, gPriority,
                                "TestFifoPend");
    CHECK_EQUAL(OS_NO_ERR, gResult);   // Make sure the task was created
    OSTimeDly(1);

    // Check that task with higher priority gets message first in both cases
    gResult = gFifo.Post((OS_FIFO_EL *)&fifoStruct);
    CHECK_EQUAL(1, gTestObj.GetIntVar3());
    CHECK_EQUAL(OS_NO_ERR, gTestObj.GetByteVar1());
    CHECK(gResultData == &fifoStruct);   // Check that task with higher priority gets message first
    gResult = gFifo.Post((OS_FIFO_EL *)&fifoStruct2);
    CHECK_EQUAL(2, gTestObj.GetIntVar3());   // We increment the id by one on the second pend so we can distinguish between the two
    CHECK_EQUAL(OS_NO_ERR, gTestObj.GetByteVar1());
    CHECK(gResultData == &fifoStruct2);   // Check that task with higher priority gets message first

    OSTimeDly(3);

    gResult = gFifo.Post((OS_FIFO_EL *)&fifoStruct);
    CHECK_EQUAL(5, gTestObj.GetIntVar3());   // Check that task with higher priority gets message first
    CHECK(gResultData == &fifoStruct);       // Check that task with higher priority gets message first
}

/**
 * @brief This test verifies that lower level tasks correctly timeout through multiple posts that are grabbed by higher priority pends.
 */
TEST(NbrtosTestFifo, FifoLowPrioTimeout)
{
    FifoStruct fifoStruct;
    FifoStruct fifoStruct2;
    double timeDif = 0.0;

    gTestObj.SetIntVar1(1);   // Timeout in seconds
    gTestObj.SetIntVar2(1);   // Id for first task
    gResult = OSTaskCreatewName(TestFifoPendRepeat, &gTestObj, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack,
                                gPriority - 1, "TestFifoPendRepeat");
    CHECK_EQUAL(OS_NO_ERR, gResult);   // Make sure the task was created
    OSTimeDly(1);

    gTestObj.SetIntVar1(4);   // Timeout in seconds
    gTestObj.SetIntVar2(5);   // Id for second task
    gResult = OSTaskCreatewName(TestFifoPend, &gTestObj, (void *)&gTestTaskStack2[USER_TASK_STK_SIZE], (void *)gTestTaskStack2, gPriority,
                                "TestFifoPend");
    CHECK_EQUAL(OS_NO_ERR, gResult);   // Make sure the task was created
    OSTimeDly(1);

    // Check that task with higher priority gets message first in both cases
    gResult = gFifo.Post((OS_FIFO_EL *)&fifoStruct);
    CHECK_EQUAL(1, gTestObj.GetIntVar3());
    CHECK_EQUAL(OS_NO_ERR, gTestObj.GetByteVar1());
    CHECK(gResultData == &fifoStruct);   // Check that task with higher priority gets message first
    gResult = gFifo.Post((OS_FIFO_EL *)&fifoStruct2);
    CHECK_EQUAL(2, gTestObj.GetIntVar3());   // We increment the id by one on the second pend so we can distinguish between the two
    CHECK_EQUAL(OS_NO_ERR, gTestObj.GetByteVar1());
    CHECK(gResultData == &fifoStruct2);   // Check that task with higher priority gets message first

    // Wait for the lower priority task to time out and ensure that it does so correctly
    while (gTestObj.GetBoolVar1() == false)
    {
        OSTimeDly(1);
    }
    CHECK_EQUAL(5, gTestObj.GetIntVar3());
    CHECK(gResultData == nullptr);   // Check that we correctly received the timeout response

    // Verify that the timeout held for the specified duration
    timeDif = difftime(*gTestObj.GetTimeVar2(), *gTestObj.GetTimeVar1());
    CHECK_EQUAL(4.0, timeDif);
}
