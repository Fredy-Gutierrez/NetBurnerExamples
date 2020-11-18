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
 * @file nbrtosTaskCreate.cpp
 * @brief Unit tests for nbrtos create task functions
 *
 * This holds tests for creating tasks using the nbrtos functions.
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
extern bool gTestFuncRan;

void TestFunction(void *data);

/**
 * @brief The startup() and teardown() routines for the uCOS used to reinitialize variables and cleanup after
 * individual tests are run.
 */
TEST_GROUP(NbrtosTestTaskCreate){void setup(){// MemoryLeakWarningPlugin::turnOffNewDeleteOverloads();
                                              gResult = OS_NO_ERR;
gTestFuncRan = false;
gParamData = nullptr;
}

void teardown()
{
    // MemoryLeakWarningPlugin::turnOnNewDeleteOverloads();
    gTestFuncRan = false;
    gParamData = nullptr;
}
}
;

/**
 * @brief This set of tests check OSTaskCreatewName().
 *
 * Tests that are completed here are as follows:
 *     - Ensure that a task can be created with a name
 *     - Ensure that a task can be created with a NULL name
 *     - Ensure a task with no stack fails with OS_CRIT_ERR
 *     - Ensure that a task with no function fails with OS_CRIT_ERR
 *     - Ensure that a task created with a priority already in use fails with OS_PRIO_EXIST
 */
TEST(NbrtosTestTaskCreate, TaskCreateWithName)
{
    // Create a basic task, ensure that it succeeds
    gResult = OSTaskCreatewName(TestFunction, gParamData, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack, gPriority,
                                "Task Name A");
    CHECK_EQUAL(OS_NO_ERR, gResult);
    OSTimeDly(TICKS_PER_SECOND);   // Short delay to allow for proper cleanup of task

    // Ensure that a task properly cleans up and that we can create another one of the same priority
    gResult = OSTaskCreatewName(TestFunction, gParamData, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack, gPriority,
                                "Task Name A");
    CHECK_EQUAL(OS_NO_ERR, gResult);
    OSTimeDly(TICKS_PER_SECOND);   // Short delay to allow for proper cleanup of task

    // Create a basic task with a NULL name
    gResult = OSTaskCreatewName(TestFunction, gParamData, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack,
                                gPriority - 1, nullptr);
    CHECK_EQUAL(OS_NO_ERR, gResult);
    OSTimeDly(TICKS_PER_SECOND);   // Short delay to allow for proper cleanup of task

    // Create a basic task with no stack and check for critical failure
    gResult = OSTaskCreatewName(TestFunction, gParamData, nullptr, nullptr, gPriority, "Task Name B");
    CHECK_EQUAL(OS_CRIT_ERR, gResult);

    // Create a basic task with no function and check for critical failure
    gResult = OSTaskCreatewName(nullptr, gParamData, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack, gPriority,
                                "Task Name C");
    CHECK_EQUAL(OS_CRIT_ERR, gResult);

    // Create a basic task with a priority is already in use
    gResult =
        OSTaskCreatewName(TestFunction, gParamData, (void *)&gTestTaskStack[USER_TASK_STK_SIZE], (void *)gTestTaskStack, 63, "Task Name D");
    CHECK_EQUAL(OS_PRIO_EXIST, gResult);
}

/**
 * @brief This set of tests check OSSimpleTaskCreatewName().
 *
 * Because OSSimpleTaskCreatewName is actually a function-like macro, there isn't much we can do here for testing
 * other than verify that the function correctly runs.
 */
TEST(NbrtosTestTaskCreate, SimpleTaskCreatewName)
{
    OSSimpleTaskCreatewName(TestFunction, gPriority, "Task Name F");
    OSTimeDly(TICKS_PER_SECOND);   // Short delay to allow for proper cleanup of task, and ensure function runs
    CHECK(gTestFuncRan == true);
}
