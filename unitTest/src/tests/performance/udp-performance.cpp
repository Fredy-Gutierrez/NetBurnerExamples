#include "CppUTest/TestHarness.h"
#include <stdio.h>
#include <udp.h>
#include <utils.h>
#include <iosys.h>

#include <stopwatch.h>

// This test group is for testing UDP performance

TEST_GROUP(udp_performance)
{
    StopWatch stopWatch;

    IPADDR UDPDestIP;

    // constants
    static const uint32_t loop_1 = 1;
    static const uint32_t loop_16 = 16;
    static const uint32_t loop_1000 = 1000;

    static const uint32_t udpPayloadLength = MAX_UDPDATA;
    static const uint32_t portNum = 20031;

    // Header lengths for calculating packet size
    static const uint32_t ethHeader = 14;
    static const uint32_t ipHeader = 20;
    static const uint32_t udpHeader = 8;

    uint8_t udpPayload[udpPayloadLength];

    void setup()
    {
        stopWatch.Clear();

        // This ip address means nothing
        UDPDestIP = AsciiToIp("10.1.30.1");

        // Fill payload with random data
        for (uint32_t i = 0; i < udpPayloadLength; i++)
        {
            udpPayload[i] = 'A' + i % 64;
        }

        // Send a single packet to allow for ARPs
        {
            UDPPacket pkt;
            pkt.SetSourcePort(portNum);
            pkt.SetDestinationPort(portNum);
            pkt.AddData((uint8_t *)udpPayload, udpPayloadLength);
            pkt.Send(UDPDestIP);
        }
    }

    void teardown() {}
};

TEST(udp_performance, tx_throughput_using_send_and_keep)
{
    UDPPacket pkt;
    pkt.SetSourcePort(portNum);
    pkt.SetDestinationPort(portNum);
    pkt.AddData((uint8_t *)udpPayload, udpPayloadLength);

    // Send packets
    stopWatch.Start();
    pkt.SendAndKeep(UDPDestIP);
    unsigned long long totalTime = stopWatch.Stop();
    stopWatch.Clear();
    UT_MEASURE_DOUBLE("1 packet sent (ns)", totalTime * 1000000);

    pkt.SetSourcePort(portNum);
    pkt.SetDestinationPort(portNum);
    pkt.AddData((uint8_t *)udpPayload, udpPayloadLength);

    stopWatch.Start();
    for (uint32_t Pleft = 0; Pleft < loop_16; Pleft++)
    {
        pkt.SendAndKeep(UDPDestIP);
    }
    totalTime = stopWatch.Stop();
    stopWatch.Clear();
    UT_MEASURE_DOUBLE("16 packet sent (ns)", totalTime * 100000);

    pkt.SetSourcePort(portNum);
    pkt.SetDestinationPort(portNum);
    pkt.AddData((uint8_t *)udpPayload, udpPayloadLength);

    stopWatch.Start();
    for (uint32_t Pleft = 0; Pleft < loop_1000 * 2; Pleft++)
    {
        pkt.SendAndKeep(UDPDestIP);
    }
    pkt.Send(UDPDestIP);

    totalTime = stopWatch.Stop();
    stopWatch.Clear();

    pkt.ResetData();

    int UDPPacketSize = (udpPayloadLength + ethHeader + ipHeader + udpHeader);
    double totalDataSentMb = (UDPPacketSize * loop_1000 * 2 * 8) / 1000000.0;
    double speedMb = totalDataSentMb / totalTime;

    UT_MEASURE_DOUBLE("Mb sent per second", speedMb);
}

TEST(udp_performance, tx_throughput_using_send)
{
    // Send packets
    stopWatch.Start();
    {
        UDPPacket pkt;
        pkt.SetSourcePort(portNum);
        pkt.SetDestinationPort(portNum);
        pkt.AddData((uint8_t *)udpPayload, udpPayloadLength);
        pkt.Send(UDPDestIP);
    }
    unsigned long long totalTime = stopWatch.Stop();
    stopWatch.Clear();
    UT_MEASURE_DOUBLE("1 packet sent (\xC2\xB5s)", totalTime * 1'000'000);

    stopWatch.Start();
    for (uint32_t Pleft = 0; Pleft < loop_16; Pleft++)
    {
        UDPPacket pkt;
        pkt.SetSourcePort(portNum);
        pkt.SetDestinationPort(portNum);
        pkt.AddData((uint8_t *)udpPayload, udpPayloadLength);
        pkt.Send(UDPDestIP);
    }
    totalTime = stopWatch.Stop();
    stopWatch.Clear();
    UT_MEASURE_DOUBLE("16 packet sent (\xC2\xB5s)", totalTime * 1'000'000);

    stopWatch.Start();
    for (uint32_t Pleft = 0; Pleft < loop_1000; Pleft++)
    {
        UDPPacket pkt;
        pkt.SetSourcePort(portNum);
        pkt.SetDestinationPort(portNum);
        pkt.AddData((uint8_t *)udpPayload, udpPayloadLength);
        pkt.Send(UDPDestIP);
    }
    totalTime = stopWatch.Stop();
    stopWatch.Clear();

    int UDPPacketSize = (udpPayloadLength + ethHeader + ipHeader + udpHeader);
    double totalDataSentMb = (UDPPacketSize * loop_1000 * 8) / 1'000'000.0;
    double speedMb = totalDataSentMb / totalTime;

    UT_MEASURE_DOUBLE("Mb sent per second", speedMb);
}

TEST(udp_performance, tx_throughput_using_udp_socket)
{
    int fdUDP = CreateTxUdpSocket(UDPDestIP, portNum, portNum);
    CHECK_TEXT(fdUDP > 0, "Unable to create Udp Socket");

    // Send packets
    stopWatch.Start();
    sendto(fdUDP, (uint8_t *)udpPayload, udpPayloadLength, UDPDestIP, portNum);
    unsigned long long totalTime = stopWatch.Stop();
    stopWatch.Clear();
    UT_MEASURE_DOUBLE("1 packet sent (ns)", totalTime * 1'000'000);

    stopWatch.Start();
    for (uint32_t Pleft = 0; Pleft < loop_16; Pleft++)
    {
        sendto(fdUDP, (uint8_t *)udpPayload, udpPayloadLength, UDPDestIP, portNum);
    }
    totalTime = stopWatch.Stop();
    stopWatch.Clear();
    UT_MEASURE_DOUBLE("16 packet sent (ns)", totalTime * 100'000);

    stopWatch.Start();
    for (uint32_t Pleft = 0; Pleft < loop_1000; Pleft++)
    {
        sendto(fdUDP, (uint8_t *)udpPayload, udpPayloadLength, UDPDestIP, portNum);
    }
    totalTime = stopWatch.Stop();
    stopWatch.Clear();

    close(fdUDP);

    int UDPPacketSize = (udpPayloadLength + ethHeader + ipHeader + udpHeader);
    double totalDataSentMb = (UDPPacketSize * loop_1000 * 8) / 1'000'000.0;
    double speedMb = totalDataSentMb / totalTime;

    UT_MEASURE_DOUBLE("Mb sent per second", speedMb);
}
