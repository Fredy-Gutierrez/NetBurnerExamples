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

#include <dhcpd.h>
#include <dhcpinternals.h>
#include <init.h>
#include <nbrtos.h>
#include <netinterface.h>
#include <string.h>
#include <system.h>

#include "MyAlloc.h"

extern DHCPProcessFunction *pDHCPProcessFunction FAST_IP_VAR;
extern DHCPProcessFunction *pDHCPServerProcessFunction FAST_IP_VAR;

OS_SEM DhcpServerFound;

static DHCPProcessFunction *prevDhcpFunc = nullptr;

const char *AppName = "Class DHCPD";

void AddMyDHCPServer(int ifc = 0);

static void DhcpServCheck(PoolPtr pp)
{
    DhcpServerFound.Post();
    FreeBuffer(pp);
}

/**
 * @brief Find DHCP servers
 */
bool FindOtherDHCPServer(int intf = 0)
{
    DHCPMessage SendMsg;
    DhcpServerFound.Init();
    prevDhcpFunc = pDHCPProcessFunction;
    pDHCPProcessFunction = DhcpServCheck;

    if (intf == 0) { intf = GetFirstInterface(); }

    CreateDhcpDiscoverMsg(SendMsg, GetInterfaceBlock(intf));
    SendMsg.SendMsg(IPADDR4::GlobalBroadCast(), GetInterfaceBlock(intf));

    // if our semaphore doesn't time out, then there is another server already up...
    if (DhcpServerFound.Pend(5 * TICKS_PER_SECOND) == OS_NO_ERR)
    {
        pDHCPProcessFunction = prevDhcpFunc;
        return true;   // abort the launch
    }
    pDHCPProcessFunction = prevDhcpFunc;
    return false;
}

/**
 *  @brief Main entry point for the example.
 */
void UserMain(void *pd)
{
    // Initialize the stack, web server, get IP addresses, etc.

    init();                                       // Initialize network stack
    StartHttp();                                  // Start web server, default port 80
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

    while (FindOtherDHCPServer())
    {
        iprintf("There is an active DHCP server on this net\r\n");
        iprintf("Kill it and hit any key to try again...");
        getchar();
    }

    AddMyDHCPServer(GetFirstInterface());

    iprintf("DHCPD Special Application: %s\r\nNNDK Revision: %s Started\r\n", AppName, GetReleaseTag());

    while (1)
    {
        OSTimeDly(20);
    }
}
