#include <predef.h>
#include <stdio.h>
#include <ctype.h>
#include <stopwatch.h>
#include <startnet.h>
#include <dns.h>
#include <tcp.h>
#include <nbtime.h>
#include <taskmon.h>
#include <init.h>

#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTest/config.h"
#include "tests/testObject.h"

extern "C"
{
    void UserMain(void *pd);
}

asm (".global _scanf_float");

const char *AppName = "3.0 UnitTester";

TestObject gTestObj;
TestObject gTestObj2;

StopWatch cppUTestTimer;

uint32_t gTestTaskStack[USER_TASK_STK_SIZE];
uint32_t gTestTaskStack2[USER_TASK_STK_SIZE];

uint8_t gPriority = TEST_PRIO - 1;   // One above the main function priority
uint8_t gResult = OS_NO_ERR;
void *gParamData = nullptr;
void *gResultData = nullptr;
bool gTestFuncRan = false;
void *gQueueStorage[5];

void StartNTP()
{
    BOOL result = FALSE;
    while (!result)
    {
        IPADDR ipa = IPADDR::NullIP();

        iprintf("Resolving NTP server name: %s\r\n", "pool.ntp.org");
        int rv = GetHostByName("pool.ntp.org", &ipa, INADDR_ANY, TICKS_PER_SECOND * 10);
        if (rv == DNS_OK)
        {
            iprintf("Setting NTP time.\r\n");
            result = SetNTPTime(ipa);
            if (!result)
            {
                iprintf("SetNTPTime() failed, waiting 5 seconds to try again\r\n");
                OSTimeDly(TICKS_PER_SECOND * 5);
            }
        }
        else
        {
            iprintf("Name resolution failed, waiting 5 seconds to try again\r\n");
            OSTimeDly(TICKS_PER_SECOND * 5);
        }
        iprintf("NTP server name resolved: %d\r\n", rv);
    }
}

void UserMain(void *pd)
{
    init();
    WaitForActiveNetwork(TICKS_PER_SECOND * 10);

    // StartNTP();

    cppUTestTimer.Start();

    OSTimeDly(10 * TICKS_PER_SECOND);   // Allow all networking startup to finish before starting tests.

    /*
                Useful options:
                -v verbose, print each test name as it runs
                -r# repeat the tests some number (#) of times
                -ojunit output to JUnit ant plugin style xml files (for CI systems)
                -g<name> group only run test whose group contains the substring group
                -n<name> name only run test whose name contains the substring name
                -lg<group> print a list of group names, separated by spaces
                -ln<name> print a list of test names in the form of group.name, separated by spaces
                -sg<group> group only run test whose group exactly matches the string group
                -sn<name> name only run test whose name exactly matches the string name
                -xg<group> group exclude tests whose group contains the substring group
                -xn<name> name exclude tests whose name contains the substring name
        */

// #if JENKINS_OUTPUT
//     const char *options[] = {"", "-ojunit"};
// #else
    const char *options[] = {"", "-v", "-gNbrtosTestSem"};
// #endif

    iprintf("Tests started\n");
    OSChangePrio(TEST_PRIO);
    CommandLineTestRunner::RunAllTests(3, options);
    OSChangePrio(MAIN_PRIO);
    iprintf("Test Complete\n");

    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND);
    }
}
