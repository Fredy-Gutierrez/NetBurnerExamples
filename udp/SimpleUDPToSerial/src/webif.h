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
  @file webif.h
**/

#define APP_VERSION "Version 1.1 11-Jan-2018"
#define LOCAL_PORT 3333
#define DEST_PORT  3334
#define DEST_IPADDR "10.1.1.104"

#define POST_BUFSIZE (4096)

#define VERIFY_KEY (0x18256052) // NV Settings key code
#define MAX_IPADDR_LEN (20)

struct NV_SettingsStruct                 // Non-volatile storage structure
{
    uint32_t VerifyKey = 0;              // Flash memory key for initialization
    int   nLocalPort = 0;                // Local UDP port to listen on
    int   nDestPort = 0;                 // Destination UDP port to send serial data
    char  szDestIpAddr[MAX_IPADDR_LEN];  // Destination UDP ip address
};

