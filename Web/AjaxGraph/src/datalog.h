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

#ifndef  __DATALOG_H
#define  __DATALOG_H

#include <stdint.h>

struct dataStruct
{
    int         sensorID = 0;
    uint32_t    timeStamp = 0;
    float       value = 0.0;
};

typedef void( *RingLogSerializer )(dataStruct *item, void *args);

class RingLog
{
private:
    dataStruct          *pHead = nullptr;
    dataStruct          *pTail = nullptr;
    int32_t             count = 0;

    const dataStruct    *pStart = nullptr;
    const dataStruct    *pEnd = nullptr;
    const int32_t       maxSize = 0;

    void IncrementTail();
    void IncrementHead();
    void IncrementPtr( dataStruct *&ptr );

public:
    RingLog( dataStruct *buffer, uint32_t bufferSize );
    ~RingLog();

    void Add( dataStruct *item );
    void Remove( dataStruct *item );
    void Clear();
    int32_t GetCount();
    void Dump();
    void Serialize( RingLogSerializer serializer, void *args );
};

#endif   /* #ifndef __DATALOG_H  */
