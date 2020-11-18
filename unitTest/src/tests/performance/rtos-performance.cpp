#include "CppUTest/TestHarness.h"
#include <stdio.h>
#include <constants.h>
#include <nbrtos.h>
#include <stopwatch.h>
#include <string.h>

TEST_GROUP(rtos_performance)
{
    StopWatch stopWatch;

    // constants
    static const uint32_t smallStackSize = 200;
    static const uint32_t pendLimit = 100000;

    void setup() {}

    void teardown() {}
};

uint32_t FastAlignedSmallStack[TEST_GROUP_CppUTestGrouprtos_performance::smallStackSize] __attribute__((aligned(4))) FAST_USER_STK;
uint32_t SmallStack[TEST_GROUP_CppUTestGrouprtos_performance::smallStackSize];

void PostTask(void *pdata)
{
    // This task is out of test scope, so use full name for pendLimit
    OS_SEM *_semaphore = (OS_SEM *)pdata;
    uint32_t _pendLimit = TEST_GROUP_CppUTestGrouprtos_performance::pendLimit;
    while (_pendLimit)
    {
        _semaphore->Post();
        _pendLimit--;
    }
    OSTaskDelete();
}

TEST(rtos_performance, task_switching_aligned_sram)
{
    static OS_SEM MyFastSemaphore FAST_USER_VAR;
    OSTaskCreatewName(PostTask, (void *)&MyFastSemaphore, (void *)&FastAlignedSmallStack[smallStackSize], (void *)FastAlignedSmallStack,
                      TEST_PRIO + 1, "PostTask");
    stopWatch.Start();
    uint32_t _pendLimit = pendLimit;
    while (_pendLimit)
    {
        MyFastSemaphore.Pend();
        _pendLimit--;
    }
    unsigned long long totalTime = stopWatch.Stop();
    stopWatch.Clear();

    double pendsPerSecond = pendLimit / totalTime;
    double nanoSecondsPerPend = (totalTime / pendLimit) * 1000000;

    iprintf("Task Switching Aligned SRAM\r\n");
    printf("   pendLimit: %d, TotalTime: %f\r\n", pendLimit, totalTime);
    printf("   pendsPerSecond: %f, nanoSecondsPerPend : %f\r\n\r\n", pendsPerSecond, nanoSecondsPerPend);

    UT_MEASURE_DOUBLE("PendsPerSecond", pendsPerSecond);
    UT_MEASURE_DOUBLE("NanoSecondsPerPend", nanoSecondsPerPend);

    OSTimeDly(2);   // Allow PostTask to cleanup

    // TODO: Find the actual expected values here, not just avg
#ifdef SB800EX
    CHECK_TEXT((nanoSecondsPerPend < 4) && (nanoSecondsPerPend > 2), "Expect SRAM pends to be between 2-4 ns");
#elif defined MOD5441X
    CHECK_TEXT((nanoSecondsPerPend < 4) && (nanoSecondsPerPend > 2), "Expect SRAM pends to be between 2-4 ns");
#elif MODM7AE70
    CHECK_TEXT((nanoSecondsPerPend < 7) && (nanoSecondsPerPend > 4), "Expect SRAM pends to be between 4-7 ns");
#endif
}

TEST(rtos_performance, task_switching)
{
    OS_SEM MySemaphore;
    OSTaskCreatewName(PostTask, (void *)&MySemaphore, (void *)&SmallStack[smallStackSize], (void *)SmallStack, TEST_PRIO + 1, "PostTask");
    stopWatch.Start();
    uint32_t _pendLimit = pendLimit;
    while (_pendLimit)
    {
        MySemaphore.Pend();
        _pendLimit--;
    }
    unsigned long long totalTime = stopWatch.Stop();
    stopWatch.Clear();

    double pendsPerSecond = (double)pendLimit / totalTime;
    double nanoSecondsPerPend = (totalTime / (double)pendLimit) * 1000000;

    iprintf("Task Switch\r\n");
    printf("   pendLimit: %d, TotalTime: %f\r\n", pendLimit, totalTime);
    printf("   pendsPerSecond: %f, nanoSecondsPerPend : %f\r\n\r\n", pendsPerSecond, nanoSecondsPerPend);

    UT_MEASURE_DOUBLE("PendsPerSecond", pendsPerSecond);
    UT_MEASURE_DOUBLE("NanoSecondsPerPend", nanoSecondsPerPend);

    OSTimeDly(2);   // Allow PostTask to cleanup

    // TDOO: Find the actual expected values here, not just avg
#ifdef SB800EX
    CHECK_TEXT((nanoSecondsPerPend < 8) && (nanoSecondsPerPend > 6), "Expect SRAM pends to be between 6-8 ns");
#elif defined MOD5441X
    CHECK_TEXT((nanoSecondsPerPend < 8) && (nanoSecondsPerPend > 6), "Expect SRAM pends to be between 6-8 ns");
#elif MODM7AE70
    CHECK_TEXT((nanoSecondsPerPend < 8) && (nanoSecondsPerPend > 6), "Expect SRAM pends to be between 6-8 ns");
#endif
}
