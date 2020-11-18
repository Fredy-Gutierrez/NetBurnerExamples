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

/*
 * @brief The size of the buffer used in the throughput tests.
 */
const int BUFFER_SIZE = 100000;

/*
 * @brief Which serial port the throughput test should be conducted on.
 */
const int SERIAL_PORT = 2;

/*
 * @brief Whether or not to enable additional debug print statements.
 */
const int DEBUG_ENABLED = 0;

/*
 * @brief The possible test states the device will run through.
 *
 * @details The states will not necessarily be entered in this order, and is
 * dependent on whether or not the device initiated the connection.
 */
enum TestStates
{
    eTestInit = 0,
    eTestSerialSend,
    eTestSerialReceive,
    eTestEthernetSend,
    eTestEthernetReceive,
    eTestComplete
};

#endif /* _CONSTANTS_H_ */
