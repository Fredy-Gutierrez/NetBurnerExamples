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
#include <ipshow.h>
#include <netinterface.h>
#include <startnet.h>
#include <syslog.h>

const char *AppName = "SysLog Demo";

/**
 *  UserMain
 *
 *  Main entry point for example
 */
void UserMain(void *pd)
{
    init();                                       // Initialize network stack
    StartHttp();                                  // Start web server, default port 80
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

    iprintf("Application started\n");

    // Wait until DHCP is complete so that we will see valid IP addresses
    InterfaceBlock *pifb = GetInterfaceBlock();
    if (pifb != nullptr)
    {
        while (pifb->dhcpClient.GetDHCPState() != SDHCP_CMPL)
        {
            OSTimeDly(1);
        }
    }

    // Show the connection information
    showIpAddresses();

    /* Syslog data will be sent as a UDP broadcast if SysLogAddress = NULL.
     * To specify a specific address, uncomment the following line and
     * specify the target host address
     */
    // SysLogAddress = AsciiToIp("10.1.1.193");

    if (SysLogAddress.IsNull()) { iprintf("\r\nStarting Syslog in UDP broadcast mode, 255.255.255.255:514\r\n"); }
    else
    {
        iprintf("\r\nSending Syslog data to: %I:514\r\n", SysLogAddress);
    }

    uint32_t count = 0;
    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND);
        SysLog("Seconds elapsed: %ld\r\n", count++);
    }
}
