#include "CppUTest/TestHarness.h"
#include <stdio.h>
#include <constants.h>
#include <nbrtos.h>
#include <stopwatch.h>
#include <string.h>

const uint32_t RAMTestSize = 1024;

uint8_t SDRAMTest[RAMTestSize] __attribute__((aligned( 16 )));
uint8_t SRAMTest[RAMTestSize] __attribute__((aligned( 16 ))) FAST_USER_VAR;

TEST_GROUP(ram_performance)
{
    StopWatch stopWatch;

	// constants
	static const uint32_t loop_1k = 1;
	static const uint32_t loop_16k = 16;
	static const uint32_t loop_1m = 1024;
	static const uint32_t loop_10m = 10240;

	void setup()
	{
		// Fill buffers with random data
		for (uint32_t i = 0; i < RAMTestSize; i++) {
			SDRAMTest[i] = 'A' + i % 64;
			SRAMTest[i] = 'A' + i % 64;
		}
	}

	void teardown()
	{
	}
};

TEST(ram_performance,memset_clear_sdram)
{
	uint32_t s = sizeof(SDRAMTest);
	stopWatch.Start();
	for(uint32_t i = 0; i<loop_1k; i++) {
	   memset( SDRAMTest, 0, s );
	}
	unsigned long long totalTime = stopWatch.Stop();
	stopWatch.Clear();

    double clearInNanoSeconds = totalTime * 1000000.0;
	UT_MEASURE_DOUBLE("1k cleared (ns)", clearInNanoSeconds);

	stopWatch.Start();
	for(uint32_t i = 0; i<loop_16k; i++) {
	   memset( SDRAMTest, 0, s );
	}
	totalTime = stopWatch.Stop();
	stopWatch.Clear();

	clearInNanoSeconds = totalTime * 1000000.0;
	UT_MEASURE_DOUBLE("16k cleared (ns)", clearInNanoSeconds);

	stopWatch.Start();
	for(uint32_t i = 0; i<loop_1m; i++) {
	   memset( SDRAMTest, 0, s );
	}
	totalTime = stopWatch.Stop();
	stopWatch.Clear();

	clearInNanoSeconds = totalTime * 1000000.0;
	UT_MEASURE_DOUBLE("1m cleared (ns)", clearInNanoSeconds);

	stopWatch.Start();
	for(volatile uint32_t i = 0; i<loop_10m; i++) {
	   memset( SDRAMTest, 0, s );
	}
	totalTime = stopWatch.Stop();
	stopWatch.Clear();

	double mbCleared = (s * loop_10m * 8) / 1048576.0;
	double mbClearedPerSecond = mbCleared / totalTime;

	UT_MEASURE_DOUBLE("mb cleared per second", mbClearedPerSecond);
}



TEST(ram_performance,memset_clear_fast_sram)
{
	uint32_t s = sizeof(SRAMTest);
	stopWatch.Start();
	for(uint32_t i = 0; i<loop_1k; i++) {
	   memset( SRAMTest, 0, s );
	}
	unsigned long long totalTime = stopWatch.Stop();
	stopWatch.Clear();

    double clearInNanoSeconds = totalTime * 1000000;
	UT_MEASURE_DOUBLE("1k cleared (ns)", clearInNanoSeconds);

	stopWatch.Start();
	for(uint32_t i = 0; i<loop_16k; i++) {
	   memset( SRAMTest, 0, s );
	}
	totalTime = stopWatch.Stop();
	stopWatch.Clear();

	clearInNanoSeconds = totalTime * 1000000;
	UT_MEASURE_DOUBLE("16k cleared (ns)", clearInNanoSeconds);

	stopWatch.Start();
	for(uint32_t i = 0; i<loop_1m; i++) {
	   memset( SRAMTest, 0, s );
	}
	totalTime = stopWatch.Stop();
	stopWatch.Clear();

	clearInNanoSeconds = totalTime * 1000000;
	UT_MEASURE_DOUBLE("1m cleared (ns)", clearInNanoSeconds);

	stopWatch.Start();
	for(uint32_t i = 0; i<loop_10m; i++) {
	   memset( SRAMTest, 0, s );
	}
	totalTime = stopWatch.Stop();
	stopWatch.Clear();

	double mbCleared = (s * loop_10m * 8) / 1048576;
	double mbClearedPerSecond = mbCleared / totalTime;

	UT_MEASURE_DOUBLE("mb cleared per second", mbClearedPerSecond);
}

TEST(ram_performance,memcpy_sdram_to_sdram)
{
	uint32_t s = sizeof(SDRAMTest);
	stopWatch.Start();
	for(uint32_t i = 0; i<loop_1k; i++) {
		memcpy(&SDRAMTest[RAMTestSize/2],SDRAMTest,s/2); // Copy first half of buffer in to second half
	}
	unsigned long long totalTime = stopWatch.Stop();
	stopWatch.Clear();

    double copyInNanoSeconds = totalTime * 1000000;
	UT_MEASURE_DOUBLE("1k copied (ns)", copyInNanoSeconds);

	stopWatch.Start();
	for(uint32_t i = 0; i<loop_16k; i++) {
		memcpy(&SDRAMTest[RAMTestSize/2],SDRAMTest,s/2); // Copy first half of buffer in to second half
	}
	totalTime = stopWatch.Stop();
	stopWatch.Clear();

	copyInNanoSeconds = totalTime * 1000000;
	UT_MEASURE_DOUBLE("16k copied (ns)", copyInNanoSeconds);

	stopWatch.Start();
	for(uint32_t i = 0; i<loop_1m; i++) {
		memcpy(&SDRAMTest[RAMTestSize/2],SDRAMTest,s/2); // Copy first half of buffer in to second half
	}
	totalTime = stopWatch.Stop();
	stopWatch.Clear();

	copyInNanoSeconds = totalTime * 1000000;
	UT_MEASURE_DOUBLE("1m copied (ns)", copyInNanoSeconds);

	stopWatch.Start();
	for(uint32_t i = 0; i<loop_10m; i++) {
		memcpy(&SDRAMTest[RAMTestSize/2],SDRAMTest,s/2); // Copy first half of buffer in to second half
	}
	totalTime = stopWatch.Stop();
	stopWatch.Clear();

	uint32_t mbCopied = ((s/2) * loop_10m * 8) / 1000000;
	double mbCopiedPerSecond = mbCopied / totalTime;

	UT_MEASURE_DOUBLE("mb copied per second", mbCopiedPerSecond);
}

TEST(ram_performance,memcpy_fast_sram_to_fast_sram)
{
	uint32_t s = sizeof(SRAMTest);
	stopWatch.Start();
	for(uint32_t i = 0; i<loop_1k; i++) {
		memcpy(&SRAMTest[RAMTestSize/2],SRAMTest,s/2); // Copy first half of buffer in to second half
	}
    unsigned long long totalTime = stopWatch.Stop();
	stopWatch.Clear();

    double copyInNanoSeconds = totalTime * 1000000;
	UT_MEASURE_DOUBLE("1k copied (ns)", copyInNanoSeconds);

	stopWatch.Start();
	for(uint32_t i = 0; i<loop_16k; i++) {
		memcpy(&SRAMTest[RAMTestSize/2],SRAMTest,s/2); // Copy first half of buffer in to second half
	}
	totalTime = stopWatch.Stop();
	stopWatch.Clear();

	copyInNanoSeconds = totalTime * 1000000;
	UT_MEASURE_DOUBLE("16k copied (ns)", copyInNanoSeconds);

	stopWatch.Start();
	for(uint32_t i = 0; i<loop_1m; i++) {
		memcpy(&SRAMTest[RAMTestSize/2],SRAMTest,s/2); // Copy first half of buffer in to second half
	}
	totalTime = stopWatch.Stop();
	stopWatch.Clear();

	copyInNanoSeconds = totalTime * 1000000;
	UT_MEASURE_DOUBLE("1m copied (ns)", copyInNanoSeconds);

	stopWatch.Start();
	for(uint32_t i = 0; i<loop_10m; i++) {
		memcpy(&SRAMTest[RAMTestSize/2],SRAMTest,s/2); // Copy first half of buffer in to second half
	}
	totalTime = stopWatch.Stop();
	stopWatch.Clear();

	uint32_t mbCopied = ((s/2) * loop_10m * 8) / 1000000;
	double mbCopiedPerSecond = mbCopied / totalTime;

	UT_MEASURE_DOUBLE("mb copied per second", mbCopiedPerSecond);
}

TEST(ram_performance,memcpy_fast_sram_to_sdram)
{
	uint32_t s = sizeof(SDRAMTest);
	stopWatch.Start();
	for(uint32_t i = 0; i<loop_1k; i++) {
		memcpy(SDRAMTest,SRAMTest,s);
	}
    unsigned long long totalTime = stopWatch.Stop();
	stopWatch.Clear();

    double copyInNanoSeconds = totalTime * 1000000;
	UT_MEASURE_DOUBLE("1k copied (ns)", copyInNanoSeconds);

	stopWatch.Start();
	for(uint32_t i = 0; i<loop_16k; i++) {
		memcpy(SDRAMTest,SRAMTest,s);
	}
	totalTime = stopWatch.Stop();
	stopWatch.Clear();

	copyInNanoSeconds = totalTime * 1000000;
	UT_MEASURE_DOUBLE("16k copied (ns)", copyInNanoSeconds);

	stopWatch.Start();
	for(uint32_t i = 0; i<loop_1m; i++) {
		memcpy(SDRAMTest,SRAMTest,s);
	}
	totalTime = stopWatch.Stop();
	stopWatch.Clear();

	copyInNanoSeconds = totalTime * 1000000;
	UT_MEASURE_DOUBLE("1m copied (ns)", copyInNanoSeconds);

	stopWatch.Start();
	for(uint32_t i = 0; i<loop_10m; i++) {
		memcpy(SDRAMTest,SRAMTest,s);
	}
	totalTime = stopWatch.Stop();
	stopWatch.Clear();

	uint32_t mbCopied = (s * loop_10m * 8) / 1000000;
	double mbCopiedPerSecond = mbCopied / totalTime;

	UT_MEASURE_DOUBLE("mb copied per second", mbCopiedPerSecond);
}

TEST(ram_performance,memcpy_sdram_to_fast_sram)
{
	uint32_t s = sizeof(SRAMTest);
	stopWatch.Start();
	for(uint32_t i = 0; i<loop_1k; i++) {
		memcpy(SRAMTest,SDRAMTest,s);
	}
    unsigned long long totalTime = stopWatch.Stop();
	stopWatch.Clear();

    double copyInNanoSeconds = totalTime * 1000000;
	UT_MEASURE_DOUBLE("1k copied (ns)", copyInNanoSeconds);

	stopWatch.Start();
	for(uint32_t i = 0; i<loop_16k; i++) {
		memcpy(SRAMTest,SDRAMTest,s);
	}
	totalTime = stopWatch.Stop();
	stopWatch.Clear();

	copyInNanoSeconds = totalTime * 1000000;
	UT_MEASURE_DOUBLE("16k copied (ns)", copyInNanoSeconds);

	stopWatch.Start();
	for(uint32_t i = 0; i<loop_1m; i++) {
		memcpy(SRAMTest,SDRAMTest,s);
	}
	totalTime = stopWatch.Stop();
	stopWatch.Clear();

	copyInNanoSeconds = totalTime * 1000000;
	UT_MEASURE_DOUBLE("1m copied (ns)", copyInNanoSeconds);

	stopWatch.Start();
	for(uint32_t i = 0; i<loop_10m; i++) {
		memcpy(SRAMTest,SDRAMTest,s);
	}
	totalTime = stopWatch.Stop();
	stopWatch.Clear();

	uint32_t mbCopied = (s * loop_10m * 8) / 1000000;
	double mbCopiedPerSecond = mbCopied / totalTime;

	UT_MEASURE_DOUBLE("mb copied per second", mbCopiedPerSecond);
}

