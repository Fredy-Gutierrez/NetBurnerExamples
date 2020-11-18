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

#include "datalog.h"

#include <stdio.h>
#include <string.h>

RingLog::RingLog( dataStruct *buffer, uint32_t bufferSize )
    : pHead( buffer ), pTail( buffer ), count( 0 ), pStart( buffer ),
    pEnd( buffer + bufferSize ), maxSize( bufferSize )
{
}


RingLog::~RingLog()
{
}


/**
 *  @brief Add
 *  Adds an item to the end of the RingLog. This performs a deep copy of the input.
 *
 *  @param item The item to be added.
 */
void RingLog::Add( dataStruct *item )
{
    if( maxSize == 0 ) { return; }
    if( !pTail ) { return; }

    memcpy( pTail, item, sizeof( *item ) );
    count++;
    IncrementTail();
}


/**
 *  @brief Remove
 *  Removes an item from the front of the RingLog. If the input is non-null,
 *  it copy the head element into the structure pointed to.
 *
 *  @param item Pointer to a return structure
 */
void RingLog::Remove( dataStruct *item )
{
    if( maxSize == 0 ) { return; }

    if( count && item )
    {
        memcpy( item, pHead, sizeof( *item ) );
        count--;
        IncrementHead();
    }
}


/**
 *  @brief Clear
 *  Clears the RingLog
 */
void RingLog::Clear()
{
    pHead = pTail = const_cast<dataStruct *>(pStart);
    count = 0;
}


/**
 *  @brief IncrementTail
 */
void RingLog::IncrementTail()
{
    if( pTail == pHead )
    {
        IncrementHead();
    }
    pTail++;

    if( pTail >= pEnd )
    {
        pTail = const_cast<dataStruct *>(pStart);
    }
}


/**
 *  @brief IncrementHead
 */
void RingLog::IncrementHead()
{
    pHead++;
    if( pHead >= pEnd )
    {
        pHead = const_cast<dataStruct *>(pStart);
    }
}


/**
 *  @brief IncrementPtr
 */
void RingLog::IncrementPtr( dataStruct *&ptr )
{
    ptr++;
    if( ptr >= pEnd )
    {
        ptr = const_cast<dataStruct *>(pStart);
    }
}


/**
 *  @brief GetCount
 *
 *  @return A count of the number of entries currently in the RingLog
 */
int32_t RingLog::GetCount()
{
    return count;
}


/**
 *  @brief Dump
 *  Prints the values of vital members of the RingLog.
 */
void RingLog::Dump()
{
    printf( "  pHead: %p\r\n", pHead );
    printf( "  pTail: %p\r\n", pTail );
    printf( "  pStart: %p\r\n", pStart );
    printf( "  pEnd: %p\r\n", pEnd );
    printf( "  count: %ld\r\n", count );
    printf( "  maxSize: %ld\r\n", maxSize );
}


/**
 *  @brief Serialize
 *  This method will iterate over the whole RingLog and pass each
 *  item to the serializer function, so as to serialize the data
 *
 *  @param serializer The serializer function to be used
 *  @param args Any additional arguments to be passed to the serializer function
 */
void RingLog::Serialize( RingLogSerializer serializer, void *args )
{
    if( maxSize == 0 ){ return; }

    dataStruct *curr = pHead;
    bool nowrap = true;
    while( ( curr != pHead ) || nowrap )
    {
        serializer( curr, args );
        IncrementPtr( curr );
        nowrap = false;
    };
}
