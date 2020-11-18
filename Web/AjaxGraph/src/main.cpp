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

#include <init.h>
#include <startnet.h>

#include "datagenerator.h"
#include "datalog.h"

extern "C"
{
    void getHTMLTicks(int sock, PCSTR url);
}

const char *AppName = "AjaxGraph";

/**
 *  @brief DumpLogData
 *  A function to be passed to the RingLog's Serialize function, which will serialize
 *  the data log to JSON objects inside an array
 *
 *  @param item A pointer to a dataStruct about to be serialized
 *  @param args A file descriptor to the TCP socket of the HTTP stream
 */
void DumpLogData(dataStruct *item, void *args)
{
    char buf[80];
    int fd = (int)args;
    snprintf(buf, 80, "{id:%d,timeStamp:%ld,value:%.4f},", item->sensorID, item->timeStamp, item->value);
    writestring(fd, buf);
}

/**
 *  @brief DumpAddr
 *  Another RingLog Serialize function. This will serialize the log to CSV format
 *
 *  @param item A pointer to a dataStruct about to be serialized
 *  @param args A file descriptor to the stream to serialize to
 */
void DumpAddr(dataStruct *item, void *args)
{
    char buf[60];
    snprintf(buf, 60, "%d,%ld,%.4f\r\n", item->sensorID, item->timeStamp, item->value);
    writestring((int)args, buf);
}

/**
 *  @brief getHTMLTicks
 *  Simply sends a string representation of the current TimeTick out
 *  the given socket/file descriptor
 *
 *  @param sock file descriptor of the TCP socket for the HTTP stream
 *  @param url Pointer to a string containing the URL requested
 */
void getHTMLTicks(int sock, PCSTR url)
{
    char tick[40];
    sniprintf(tick, 40, "%ld", TimeTick);
    writestring(sock, tick);
}

/**
 *  @brief getSensorData
 *  Is called by one of the AJAX methods to return the data log in
 *  JSON form
 *
 *  @param sock file descriptor for the TCP socket of the HTTP stream
 *  @param url Pointer to a string containing the URL requested
 */
void getSensorData(int sock, PCSTR url)
{
    writestring(sock, "{entries:[");
    dataLog.Serialize(DumpLogData, (void *)sock);
    writestring(sock, "]}");
}

/**
 *  @brief UserMain
 *
 *  Main entry point for the example
 */
void UserMain(void *pd)
{
    init();                                       // Initialize network stack
    StartHttp();                                  // Start web server, default port 80
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

    uint32_t sensorTick = 0;
    uint32_t logTickOffset = SeedLog();
    uint32_t nextSensor1Tick = ((logTickOffset / SENSOR_1_PERIOD) + 1) * SENSOR_1_PERIOD;
    uint32_t nextSensor2Tick = ((logTickOffset / SENSOR_2_PERIOD) + 1) * SENSOR_2_PERIOD;

    iprintf("Application: %s\r\nNNDK Revision: %s\r\n", AppName, GetReleaseTag());
    //uint32_t tickCount = 0;
    while (1)
    {
        OSTimeDly(4);
        sensorTick = logTickOffset + TimeTick;

        // if it's time to do so, log the sensor value
        if (sensorTick > nextSensor1Tick)
        {
            LogSensor(1, sensorTick);
            nextSensor1Tick += SENSOR_1_PERIOD;
        }
        if (sensorTick > nextSensor2Tick)
        {
            LogSensor(2, sensorTick);
            nextSensor2Tick += SENSOR_2_PERIOD;
        }

        // output the data log if the user wants
        if (charavail())
        {
            getchar();
            dataLog.Serialize(DumpLogData, (void *)CurrentStdioFD(1));
        }
    }
}
