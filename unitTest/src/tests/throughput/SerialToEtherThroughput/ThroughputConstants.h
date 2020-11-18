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

#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_
#pragma once

#include <basictypes.h>

enum TestModes
{
    eTestModeNone = 0,
    eTestModeTCP,
    eTestModeUDP
};

/**
 * @brief The size of the buffer used in the throughput tests.
 *
 * For UDP tests, this number should be a multiple of the characters that will be acumulated
 * before sending a packet.  The default for this value is 32.
 */
const int BUFFER_SIZE = 96000;

/**
 * @brief The size of the buffer that receives debug commands
 */
const int COMMAND_BUFFER_SIZE = 80;

/**
 * @brief Magic number determined by testing.  Any higher and writes 
 * will eventually hang with SecureSerialToEthernet factory app.
 */
const int TRANSMISSION_RATE = 4643;

/**
 * @brief The number of tests that should be performed.
 */
const int TEST_COUNT = 1;

/**
* @brief Which serial port debug data and AT commands will come through.
*/
const int DEBUG_SERIAL_PORT = 0;

/**
 * @brief Whether or not to enable additional debug print statements.
 */
const bool DEBUG_ENABLED = false;

/**
 * @brief Number of serial ports.
 */
// If you change this to allow other platforms to use more than 2,
// a fair bit of logic will need to change
#if ( defined NANO54415 || defined PK70 )
#define NB_FACTORY_SERIAL_PORTS              ( 5 )
#else
#define NB_FACTORY_SERIAL_PORTS              ( 2 )
#endif


#endif /* _CONSTANTS_H_ */
