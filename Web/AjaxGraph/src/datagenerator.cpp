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

#include <math.h>
#include <nbrtos.h>
#include <stdlib.h>

#include "datalog.h"
#include "datagenerator.h"

static dataStruct logBuffer[ LOG_SIZE ];
RingLog dataLog( logBuffer, LOG_SIZE );

static float sensor1Reading = 0.0;
static float sensor2Reading = 0.0;

/**
 *  @brief Sensor1Delta
 *  Helper function for generating data for sensor 1. Creates the delta to use
 *  from the previous data points.
 *
 *  @param tick The current tick to use for calculations.
 *
 *  @return The delta value for the sensor.
 */
float Sensor1Delta( int32_t tick )
{
    int32_t base = (TimeTick * 5) & 0xFFF;

    float sinScale = 0.50;
    float sign = (float( rand() ) / RAND_MAX > 0.5) ? 1 : -1;

    return sign * (sinScale * sin( base / (180.0 * 4) ) * float( rand() ) / RAND_MAX);
}


/**
 *  @brief Sensor2Delta
 *  Helper function for generating data for sensor 2. Creates the delta to use
 *  from the previous data points.
 *
 *  @param tick The current tick to use for calculations.
 *
 *  @return The delta value for the sensor.
 */
float Sensor2Delta( int32_t tick )
{
    int32_t base = (tick * 5) & 0xFFF;

    float sinScale = .900;
    float sign = (float( rand() ) / RAND_MAX > 0.5) ? 1 : -1;

    return sign * (sinScale * sin( base / (180.0 * 4) ) * float( rand() ) / RAND_MAX);

}


/**
 *  @brief LogSensor
 *  Inserts a data point for the given sensor into the dataLog
 *
 *  @param id The id of the sensor to log
 *  @param tick The current tick for logging
 */
void LogSensor( int id, int32_t tick )
{
    dataStruct tmp;
    if( id == 1 )
    {
        sensor1Reading += Sensor1Delta( tick );
        tmp.value = sensor1Reading;
    }
    else if( id == 2 )
    {
        sensor2Reading += Sensor2Delta( tick );
        tmp.value = sensor2Reading;
    }

    tmp.sensorID = id;
    tmp.timeStamp = tick;

    dataLog.Add( &tmp );
}


/**
 *  @brief SeedLog
 *  Seeds the dataLog with random data
 *
 *  @return returns the 'tick' when the log finished filling
 */
int32_t SeedLog()
{
    uint32_t sensorTick = 0;
    sensor1Reading = SENSOR_1_INIT;
    sensor2Reading = SENSOR_2_INIT;

    for( int i = 0; i < LOG_SIZE; sensorTick++ )
    {
        if( ( sensorTick % SENSOR_1_PERIOD ) == 0 )
        {
            LogSensor( 1, sensorTick );
            i++;
        }
        if( ( sensorTick % SENSOR_2_PERIOD ) == 0 )
        {
            LogSensor( 2, sensorTick );
            i++;
        }
    }

    return sensorTick;
}
