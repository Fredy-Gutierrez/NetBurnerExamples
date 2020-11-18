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

/**
 * @file testObject.h
 * @brief Simple data object used for general purpose in testing.
 *
 * @author NetBurner, Inc.
 * @version 1.01
 * @date 10-21-2016
 */

#ifndef TESTOBJECT_H_
#define TESTOBJECT_H_

#include <basictypes.h>
#include <time.h>

const int gTestBufferSizeSmall = 1;
const int gTestBufferSizeLarge = 500;

/**
 * @class TestObject
 * @brief Test Object that can be used to store and track state information and data.
 *
 * This class is used to hold data and store state information that is used during the process of testing.
 * One example of its use is setting a flag on the object that will signal a task to shut down.  Another
 * example is to hold the return value of a function called within a task.
 */
class TestObject
{
public:
    TestObject(){};
    ~TestObject(){};

    inline bool GetBoolVar1(){ return m_boolVar1; }
    inline void SetBoolVar1( bool var1 ){ m_boolVar1 = var1; }

    inline bool GetBoolVar2(){ return m_boolVar2; }
    inline void SetBoolVar2( bool var1 ){ m_boolVar2 = var1; }

    inline uint8_t GetByteVar1(){ return m_byteVar1; }
    inline void SetByteVar1(uint8_t byteVar ){ m_byteVar1 = byteVar; }

    inline uint8_t GetByteVar2(){ return m_byteVar2; }
    inline void SetByteVar2(uint8_t byteVar ){ m_byteVar2 = byteVar; }

    inline int GetIntVar1(){ return m_int1; }
    inline void SetIntVar1( int i ){ m_int1 = i; }

    inline int GetIntVar2(){ return m_int2; }
    inline void SetIntVar2( int i ){ m_int2 = i; }

    inline int GetIntVar3(){ return m_int3; }
    inline void SetIntVar3( int i ){ m_int3 = i; }

    inline time_t* GetTimeVar1(){ return &m_timeVar1; }
    inline time_t* GetTimeVar2(){ return &m_timeVar2; }

    inline char* GetLargeBuffer(){ return m_testBufferLarge;  }
    inline char* GetSmallBuffer() { return m_testBufferSmall; }

    void ClearBuffers();
    void Reset();

private:
    bool m_boolVar1     = false;
    bool m_boolVar2     = false;
    uint8_t m_byteVar1     = 0;
    uint8_t m_byteVar2     = 0;
    int m_int1          = 0;
    int m_int2          = 0;
    int m_int3          = 0;
    time_t m_timeVar1   = 0;
    time_t m_timeVar2   = 0;

    char m_testBufferLarge[ gTestBufferSizeLarge ] = { '\0' };
    char m_testBufferSmall[ gTestBufferSizeSmall ] = { '\0' };
};

#endif /* TESTOBJECT_H_ */
