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
 * @file nbrtosTests.cpp
 * @brief Unit tests for nbrtos
 *
 * This holds the UcosTestGroup and the associated tests.  Here every function available from nbrtos
 * is tested.  Tests are representative of a single function call.
 *
 * @author NetBurner, Inc.
 * @version 1.01
 * @date 10-18-2016
 */

#include <basictypes.h>
#include <constants.h>
#include <stopwatch.h>
#include <nbrtos.h>
#include <stdio.h>
#include <time.h>

#ifdef MOD5441X
#include <intcdefs.h>
#endif

#include "CppUTest/TestHarness.h"
#include "tests/testObject.h"

extern uint32_t gTestTaskStack[USER_TASK_STK_SIZE] __attribute__((aligned(4)));
extern uint32_t gTestTaskStack2[USER_TASK_STK_SIZE] __attribute__((aligned(4)));

extern TestObject gTestObj;
extern uint8_t gPriority;   // One above the main function priority
extern uint8_t gResult;
extern void *gParamData;
extern void *gResultData;
extern bool gTestFuncRan;

int gTest = 0;

void TestFunction(void *data)
{
    gTestFuncRan = true;
}

void TestFunctionChangePrio(void *data)
{
    if (data == nullptr) { return; }
    TestObject *testObj = (TestObject *)data;

    while (1)
    {
        // Only request the priority change once
        if (!testObj->GetBoolVar2())
        {
            testObj->SetByteVar1(OSChangePrio(testObj->GetByteVar2()));
            testObj->SetBoolVar2(true);
        }

        // If set, delete the task
        if (testObj->GetBoolVar1()) { return; }

        OSTimeDly(TICKS_PER_SECOND);   // Short delay to allow main take over
    }
}

void TestFunctionDelay(void *data)
{
    if (data == nullptr) { return; }
    TestObject *testObj = (TestObject *)data;

    // Set the time before and after the delay
    time(testObj->GetTimeVar1());
    OSTimeDly(testObj->GetIntVar1());
    time(testObj->GetTimeVar2());
}

/**
 * @brief The startup() and teardown() routines for the nbrtos used to reinitialize variables and cleanup after
 * individual tests are run.
 */
TEST_GROUP(NbrtosTestGroup){void setup(){// MemoryLeakWarningPlugin::turnOffNewDeleteOverloads();
                                         gTestObj.Reset();
gResult = OS_NO_ERR;
gTestFuncRan = false;
gParamData = nullptr;
gResultData = nullptr;
}

void teardown()
{
    // MemoryLeakWarningPlugin::turnOnNewDeleteOverloads();
    gTestFuncRan = false;
    gParamData = nullptr;
    gResultData = nullptr;
}
}
;

/**
 * @brief This set of tests check OSTaskDelete().
 *
 * To test OSTaskDelete(), the following steps are taken:
 *     - Create a task with a given priority that will call OSTaskDelete() and ensure it is successful
 *     - Pause to allow the task to delete itself
 *     - Create a task with the same priority as the previous task and ensure that it is successful
 */
TEST(NbrtosTestGroup, TaskDelete)
{
    // First task
    gResult = OSTaskCreatewName(TestFunction, gParamData, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack, gPriority,
                                "TestFunction");
    CHECK_EQUAL(OS_NO_ERR, gResult);
    OSTimeDly(TICKS_PER_SECOND);   // Short delay to allow for proper cleanup of task

    // Second task with same priority
    gResult = OSTaskCreatewName(TestFunction, gParamData, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack, gPriority,
                                "TestFunction");
    CHECK_EQUAL(OS_NO_ERR, gResult);
}

/**
 * @brief This set of tests check OSChangePrio().
 *
 * Tests that are completed here are as follows:
 *     - Test that priority can be changed successfully
 *     - Test that a task can be changed to the old priority level of a task that had it's priority changed
 *     - Test that a new task cannot be changed to a priority that is in use
 *     - Test that a new task cannot be changed to a priority that is outside of the valid range
 */
TEST(NbrtosTestGroup, ChangePrio)
{
    // Object that stores test related data and essentially holds the state of the test
    gParamData = &gTestObj;

    // Create a task
    gTestObj.SetByteVar2(gPriority - 1);   // Set the priority that we change to
    gResult = OSTaskCreatewName(TestFunctionChangePrio, gParamData, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack,
                                gPriority, "TestFunctionChangePrio");
    CHECK_EQUAL(OS_NO_ERR, gResult);                 // Make sure the task was created
    OSTimeDly(TICKS_PER_SECOND);                     // Short delay to ensure task has time to change the priority level
    CHECK_EQUAL(OS_NO_ERR, gTestObj.GetByteVar1())   // Verify that the priority change was successful, set in TestFunctionChangePrio()

    // Create a task with that same priority as the first task to ensure the priority is free
    gResult = OSTaskCreatewName(TestFunction, nullptr, (void *)&gTestTaskStack2[USER_TASK_STK_SIZE], (void *)gTestTaskStack2, gPriority,
                                "TestFunction");
    CHECK_EQUAL(OS_NO_ERR, gResult);
    gTestObj.SetBoolVar1(true);    // Signal that the task can shut down
    OSTimeDly(TICKS_PER_SECOND);   // Short delay to allow for proper cleanup of task

    // Try to change to a priority that is used by the OS
    gTestObj.Reset();
    gTestObj.SetByteVar2(63);   // Set the priority that we change to
    gResult = OSTaskCreatewName(TestFunctionChangePrio, gParamData, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack,
                                gPriority, "TestFunctionChangePrio");
    CHECK_EQUAL(OS_NO_ERR, gResult);                      // Make sure the task was created
    OSTimeDly(TICKS_PER_SECOND);                          // Short delay to ensure task has time to change the priority level
    CHECK_EQUAL(OS_PRIO_EXIST, gTestObj.GetByteVar1());   // Verify that the priority change failed, set in testFunctionChangePrio()
    gTestObj.SetBoolVar1(true);                           // Set to signal that task can shut down
    OSTimeDly(TICKS_PER_SECOND);                          // Short delay to allow for proper cleanup of task

    // Try to change to a priority outside of the acceptable range
    gTestObj.Reset();
    gTestObj.SetByteVar2(100);   // Set the priority that we change to
    gResult = OSTaskCreatewName(TestFunctionChangePrio, gParamData, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack,
                                gPriority, "TestFunctionChangePrio");
    CHECK_EQUAL(OS_NO_ERR, gResult);                        // Make sure the task was created
    OSTimeDly(TICKS_PER_SECOND);                            // Short delay to ensure task has time to change the priority level
    CHECK_EQUAL(OS_PRIO_INVALID, gTestObj.GetByteVar1());   // Verify that the priority change failed, set in testFunctionChangePrio()
    gTestObj.SetBoolVar1(true);                             // Set to signal that task can shut down
    OSTimeDly(TICKS_PER_SECOND);                            // Short delay to allow for proper cleanup of task
}

/**
 * @brief This set of tests check OSTimeDly().
 *
 *  Tests that are completed here are as follows:
 *      - Create a task with a delay of 1 second, and verify the delay time with C++'s difftime()
 *      - Create a task with a delay of 5 seconds, and verify the delay time with C++'s difftime()
 *
 * @todo Test with delay of 0
 * @todo Test with negative delay
 */
TEST(NbrtosTestGroup, TimeDly)
{
    // Get the time before and after calling OSTimeDly and verify that the specified amount of time has passed
    time_t preTest;
    time_t postTest;
    double timeDif = 0.0;

    time(&preTest);
    OSTimeDly(TICKS_PER_SECOND);
    time(&postTest);

    timeDif = difftime(postTest, preTest);
    CHECK_EQUAL(1.0, timeDif);

    // Run again with a longer delay
    time(&preTest);
    OSTimeDly(TICKS_PER_SECOND * 5);
    time(&postTest);

    timeDif = difftime(postTest, preTest);
    CHECK_EQUAL(5.0, timeDif);
}

/**
 * @brief This set of tests check OSChangeTaskDly().
 *
 * Tests that are completed here are as follows:
 *    - Create a task with a delay, and then extend it
 *    - Create a task with a delay, and then shorten it
 */
TEST(NbrtosTestGroup, ChangeTaskDly)
{
    // Object that stores test related data and essentially holds the state of the test
    gParamData = &gTestObj;

    double timeDif = 0.0;

    // Create a task that will change the delay from 3 seconds to 1 second
    gTestObj.SetIntVar1(TICKS_PER_SECOND * 3);
    gResult = OSTaskCreatewName(TestFunctionDelay, gParamData, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack,
                                gPriority, "TestFunctionDelay");
    CHECK_EQUAL(OS_NO_ERR, gResult);   // Make sure the task was created

    OSChangeTaskDly(gPriority, TICKS_PER_SECOND);   // Change the delay to 1 second
    OSTimeDly(TICKS_PER_SECOND * 5);                // Delay this task to wait for the other to complete

    timeDif = difftime(*gTestObj.GetTimeVar2(), *gTestObj.GetTimeVar1());
    CHECK_EQUAL(1.0, timeDif);

    // Create a task that will change the delay from 1 second to 3 seconds
    gTestObj.Reset();
    timeDif = 0.0;
    gTestObj.SetIntVar1(TICKS_PER_SECOND);
    gResult = OSTaskCreatewName(TestFunctionDelay, gParamData, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack,
                                gPriority, "TestFunctionDelay");
    CHECK_EQUAL(OS_NO_ERR, gResult);   // Make sure the task was created

    OSChangeTaskDly(gPriority, TICKS_PER_SECOND * 3);   // Change the delay to 1 second
    OSTimeDly(TICKS_PER_SECOND * 5);                    // Delay this task to wait for the other to complete

    timeDif = difftime(*gTestObj.GetTimeVar2(), *gTestObj.GetTimeVar1());
    CHECK_EQUAL(3.0, timeDif);
}
