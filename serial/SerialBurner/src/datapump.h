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

#ifndef _DATAPUMP_H_
#define _DATAPUMP_H_

/*
 * Since this program may be used for machine to machine connections you
 * can configure the messages that are sent.
 * 
 * MTS messages are Message To Serial
 * MTN messages are Messages to the Network
 */
#define MTS_WHEN_NOTCONNECTED           "Waiting for connection \r\n"
#define MTS_WHEN_CONNECTION_OPENED      "New Connection Opened\r\n"
#define MTS_WHEN_CONNECTION_CLOSED      "Connection Closed by Network \r\n"
#define MTS_WHEN_CONNECTION_TIMEDOUT    "Connection Timed out and Closed\r\n"
#define MTS_WHEN_CONNECTION_OVERIDDEN   "This Connection is being Overidden.\r\n"

#define MTN_WHEN_CONNECTION_OVERIDDEN   "Your Connection was just Overidden\r\n"
#define MTN_WHEN_CONNECTION_OPENED      "Connection Opened \r\n"
#define MTN_WHEN_CONNECTION_TIMEDOUT    "Your Connection Timed out and will be Closed\r\n"

#define BUFFER_SIZE (1500)
#define CLIENT_WRITE_BUF_SIZE (256)

int DataPump(int fd1, int fd2, int serverfd);    // file descriptor data pump

#endif
