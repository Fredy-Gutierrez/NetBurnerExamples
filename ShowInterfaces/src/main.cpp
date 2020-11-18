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

#include <fdprintf.h>
#include <init.h>
#include <ipv6/ipv6_interface.h>
#include <netinterface.h>
#include <stdio.h>

#include "ip_util.h"

// Uncomment to use WIFI
//#define USE_WIFI  (1)

// Uncomment to test static IPV6 addresses.
//#define  STATIC_IP6_TEST (1)

#ifdef USE_WIFI
#include <wifi/wifi.h>
#include <wifi/wifiDriver.h>
#include <qspi.h>
#endif

#if (!defined IPV6)
#error This example requires IPv6 to be defined.
#endif

const char *AppName = "Show Interfaces";

/*------------------------------------------------------------------------------
 * UserMain
 *-----------------------------------------------------------------------------*/
void UserMain(void *pd)
{
    init();                                       // Initialize network stack
    StartHttp();                                  // Start web server, default port 80
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

    iprintf("Application: %s\r\nNNDK Revision: %s\r\n", AppName, GetReleaseTag());

#ifdef USE_WIFI
    InitWifi_SPI();
#endif

#ifdef STATIC_IP6_TEST
    IPv6Interface *pIPv6If = IPv6Interface::GetFirst_IP6_Interface();
    IPADDR i6 = IPADDR6::AsciiToIp6("1234:5678:abcd::1234");
    pIPv6If->AddStaticAddress(i6, 64);
#endif

    while (1)
    {
        iprintf("\r\nType any key to display address information\r\n");
        iprintf("Note that IPv6 routable addresses may take a few seconds to be received\r\n");
        getchar();
        showIpAddressesSerial();
        showLinkStatus();
        iprintf("\r\n\r\n");
    }
}
