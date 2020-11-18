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

#include <init.h>
#include <nbtime.h>
#include <predef.h>
#include <startnet.h>
#include <stdio.h>

#include "effs_time.h"

const char *AppName = "SSL/TLS Send Mail Example";

/**
 * @brief Main entry point of the example
 */
void UserMain(void *pd)
{
    init();                                       // Initialize network stack
    StartHttps();                                 // Start web server, default port 80
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

    // Try setting the time via a NTP Server
    if (SetTimeNTP()) { tzsetchar((char *)"PST8PDT7,M3.2.0/01:00:00,M11.1.0/02:00:00"); }
    else
    {
        iprintf("NTP failed, system time not set\r\n");
    }
    DisplaySystemTime();
    iprintf("\r\n");

    // Loop forever - all the action takes place from the web page
    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND);
    }
}
