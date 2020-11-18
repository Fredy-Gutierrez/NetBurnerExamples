#include "CppUTest/TestHarness.h"

#include <constants.h>
#include <stopwatch.h>
#include <iosys.h>
#include <tcp.h>
#include <stdio.h>

// This test group is for testing TCP performance

TEST_GROUP(tcp_performance)
{
    StopWatch stopWatch;

    // Header lengths for calculating packet size
    static const uint32_t ethHeader = 14;
    static const uint32_t ipHeader = 20;
    static const uint32_t tcpHeader = 20;

    static const uint32_t loop_1 = 1;
    static const uint32_t loop_16 = 16;
    static const uint32_t loop_1000 = 1000;

    static const uint32_t maxDataSend = 1'000'000;

    static const uint32_t tcpPayloadLength = ETH_MAX_PAYLOAD - (ethHeader + ipHeader + tcpHeader);
    static const uint32_t portNum = 23;

    char tcpBuffer[maxDataSend] = {1};
    IPADDR ip = GetNullIP();
    int tcpFd = -1;

    void setup()
    {
        ip.SetFromAscii("10.1.30.2");

        // Initialize TCP connection
        tcpFd = connect(ip, portNum, TICKS_PER_SECOND * 15);
    }

    void teardown()
    {
        close(tcpFd);
        OSTimeDly(TICKS_PER_SECOND * 5);
    }
};

TEST(tcp_performance, tx_stress)
{
    char *dataBufPtr = nullptr;
    uint32_t curBuffer = 100;
    unsigned long long totalTime = 0.0f;
    char strBuf[256];
    uint32_t totalTestCount = 10;

    // Check for connection error
    if (tcpFd > 0)
    {
        while (curBuffer <= maxDataSend)
        {
            dataBufPtr = &tcpBuffer[0];
            uint32_t testCount = 0;

            stopWatch.Start();
            while (testCount < totalTestCount)
            {
                int s = 0;
                s = writeall(tcpFd, dataBufPtr, curBuffer);
                if (s < (int)curBuffer) { FAIL_TEST("Failed to send data with write()."); }
                testCount++;
            }

            totalTime += stopWatch.Stop();
            stopWatch.Clear();

            // This isn't exactly accurate, since we don't know how many packets were actually sent, but it should be fairly close
            int TcpDataSize = (curBuffer + ethHeader + ipHeader + tcpHeader) * totalTestCount;
            double totalDataSentMb = (TcpDataSize * 8) / 1'000'000.0;
            double speedMb = totalDataSentMb / totalTime;

            snprintf(strBuf, 256, "Sending %d bytes with writeall(), Mb sent per second", curBuffer);
            UT_MEASURE_DOUBLE(strBuf, speedMb);

            OSTimeDly(TICKS_PER_SECOND);
            curBuffer *= 10;
            totalTime = 0.0f;
        }
    }
    else
    {
        snprintf(strBuf, 256, "Failed to connect with error: %d", tcpFd);
        FAIL_TEST(strBuf);
    }
}

TEST(tcp_performance, tx_throughput)
{
    // Check for connection error
    if (tcpFd > 0)
    {
        int s = 0;
        // Send packets
        stopWatch.Start();
        {
            s = writeall(tcpFd, tcpBuffer, tcpPayloadLength);
            if (s <= 0) { FAIL_TEST("Failed to send data with write()."); }
        }
        unsigned long long totalTime = stopWatch.Stop();
        stopWatch.Clear();
        UT_MEASURE_DOUBLE("1 packet sent (\xC2\xB5s)", totalTime * 1'000'000);

        stopWatch.Start();
        for (uint32_t Pleft = 0; Pleft < loop_16; Pleft++)
        {
            s = writeall(tcpFd, tcpBuffer, tcpPayloadLength);
            if (s <= 0) { FAIL_TEST("Failed to send data with write()."); }
        }
        totalTime = stopWatch.Stop();
        stopWatch.Clear();
        UT_MEASURE_DOUBLE("16 packet sent (\xC2\xB5s)", totalTime * 1'000'000);

        stopWatch.Start();
        for (uint32_t Pleft = 0; Pleft < loop_1000; Pleft++)
        {
            s = writeall(tcpFd, tcpBuffer, tcpPayloadLength);
            if (s <= 0) { FAIL_TEST("Failed to send data with write()."); }
        }
        totalTime = stopWatch.Stop();
        stopWatch.Clear();

        int TcpPacketSize = (tcpPayloadLength + ethHeader + ipHeader + tcpHeader);
        double totalDataSentMb = (TcpPacketSize * loop_1000 * 8) / 1'000'000.0;
        double speedMb = totalDataSentMb / totalTime;
        UT_MEASURE_DOUBLE("Mb sent per second", speedMb);
    }
    else
    {
        FAIL_TEST("Failed to connect.");
    }
}
