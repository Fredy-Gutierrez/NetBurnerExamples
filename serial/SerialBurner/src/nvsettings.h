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

#ifndef _NVSETTINGS_H_
#define _NVSETTINGS_H_

#define VERIFY_KEY (0x48666050)   // NV Settings key code

/*
 * The default number of seconds between receiving TCP chars before a timeout occurs.
 * The system default timeout, TCP_WRITE_TIMEOUT, is 10 seconds, which is the minimum
 * timeout value. A timeout value of 0 will disable the timeout feature.
 * */
#define DEF_INACTIVITY_TIMEOUT (60)  // inactivity timeout in seconds

/* If a new client TCP connection is attempted while one is active,
 * one of the following three actions can be taken:
 *      a) Ignore the incoming connection (leave current connection active)
 *           (set override timeout to 0xFFFFFFFF)
 *      b) Replace the existing connection if it has been idle for a specified number of seconds.
 *           (set override to the number of seconds to wait)
 *      c) Always replace the existing connection.
 *           (set override to 0 seconds)
 *
 *    This is done with the override timeout setting below:
 *    The default number of seconds to wait before a new connection can override an
 *    existing connection.
 */
#define DEF_OVERRIDE_TIMEOUT (20)


struct NV_SettingsStruct
{
    uint32_t VerifyKey;
    uint16_t ServerListenPort;
    uint16_t ClientTimeout;
    uint16_t ClientOverrideTimeout;
    uint32_t DataBaudRate;
    uint16_t SerialDataFlowControl;
};

extern NV_SettingsStruct NV_Settings;      // Non-volatile settings to store in flash memory


#endif

