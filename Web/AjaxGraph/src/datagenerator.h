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

#ifndef  __DATAGENERATOR_H
#define  __DATAGENERATOR_H

#include "datalog.h"

#define LOG_SIZE        1000
#define SENSOR_1_PERIOD 13
#define SENSOR_2_PERIOD 11
#define SENSOR_1_INIT   44.23
#define SENSOR_2_INIT   -10.0

extern RingLog dataLog;

int32_t SeedLog();
void LogSensor( int id, int32_t tick );

#endif /* __DATAGENERATOR_H */
