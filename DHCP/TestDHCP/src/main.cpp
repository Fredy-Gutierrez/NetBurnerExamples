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

#include <arp.h>
#include <init.h>
#include <netinterface.h>
#include <serial.h>
#include <startnet.h>
#include <stdlib.h>

#ifdef DEBUG
#define DHCPTEST_DEBUG 1
#endif

#ifdef DHCPTEST_DEBUG
extern void PrintDhcpStatus(DhcpObject &dob);
#endif

// Application name that will appear in IPSetup
const char *AppName = "DHCPTest";

/*-------------------------------------------------------------------
 * Display DHCP state
 *-----------------------------------------------------------------*/
void DisplayDhcpState(DhcpObject &Dobj)
{
    switch (Dobj.GetDHCPState())
    {
        case SDHCP_NOTSTARTED:   // The System has not been initialized
            iprintf("DHCP not started\r\n");
            break;

        case SDHCP_DISCOVER:   // The system is discovering the DHCP servers
            iprintf("DHCP Discover\r\n");
            break;

        case SDHCP_OFFER:   // The system has responded to an OFFER
            iprintf("DHCP Offer\r\n");
            break;

        case SDHCP_ACK:   // The System has Acknowledged the OFFER
            iprintf("DHCP Acknowledge\r\n");
            break;

        case SDHCP_INIT:   // The System is reinitalzing
            iprintf("DHCP Initializing\r\n");
            break;

        case SDHCP_CMPL:   // The System has obtained a valid DHCP lease
            iprintf("DHCP Complete\r\n");
            break;

        case SDHCP_RENEW:   // The System is in the procecess of renewing
            iprintf("DHCP Renew in process\r\n");
            break;

        case SDHCP_REBIND:   // The System has failed the Renew and is trying to Rebind
            iprintf("DHCP Rebind in process\r\n");
            break;

        case SDHCP_RELEASE:   // The System is trying to release the Lease
            iprintf("DHCP Release in process\r\n");
            break;

        case SBOOTP_TRANSMITTING:   // Trying BOOTP
            iprintf("DHCP BOOTP in process\r\n");
            break;

        case SBOOTP_DONE:   // BOOTP complete
            iprintf("DHCP BOOTP complete\r\n");
            break;

        case SDHCP_FAILED:   // DHCP attempt failed - could not obtain a DHCP lease
            iprintf("DHCP Failed, count not obtain lease\r\n");
            break;
    }
}

/*-------------------------------------------------------------------
 Display the IP settings for a specific Interface.
 -------------------------------------------------------------------*/
void ShowDhcpSettings(int Interface)
{
    InterfaceBlock *ib = GetInterfaceBlock(Interface);   // Get interface data

    iprintf("\r\n\r\n");
    iprintf("DHCP assigned the following values:\r\n");
    iprintf("-----------------------------------\r\n");
    iprintf("Dhcp State: ");
    DisplayDhcpState(ib->dhcpClient);
    iprintf("\r\n");
    ib->ShowInterfaceValues();
    iprintf("Interface Name: %s\r\n", ib->GetInterfaceName());
    iprintf("\r\n");
}

/*-------------------------------------------------------------------
 Display Serial Port Debug Menu
 ------------------------------------------------------------------*/
void DisplayMenu()
{
    iprintf("\r\n----- Menu -----\r\n");
    iprintf("t = Get remaining lease time\r\n");
#ifdef DHCPTEST_DEBUG
    iprintf("s = Display DHCP Status\r\n");
#endif
    iprintf("v = Check for valid DHCP lease\r\n");
    iprintf("1 = Set Secs counter to 5 seconds before renew time\r\n");
    iprintf("2 = Set Secs counter to 5 seconds before rebind time\r\n");
    iprintf("3 = Set Secs counter to 5 seconds before lease exp. time\r\n");
    iprintf("4 = Force DHCP Renew\r\n");
    iprintf("5 = Force DHCP Rebind\r\n");
}

/*-------------------------------------------------------------------
 UserMain
 ------------------------------------------------------------------*/
void UserMain(void *pd)
{
    init();
    InterfaceBlock *ib = GetInterfaceBlock();
    DhcpObject *pDhcpObj = &(ib->dhcpClient);
    DisplayMenu();

    while (1)
    {
        char c = getchar();
        switch (c)
        {
            case 'a': ShowArp(); break;

            case '1':
                Secs = (uint32_t)(pDhcpObj->GetDhcpRenewTime() - 5);
                NetTimeOutManager.RegisterTriggerAt(*pDhcpObj, TimeTick + 20);
                break;

            case '2':
                Secs = (uint32_t)(pDhcpObj->GetDhcpRebindTime() - 5);
                NetTimeOutManager.RegisterTriggerAt(*pDhcpObj, TimeTick + 20);
                break;

            case '3':
                Secs = (uint32_t)(pDhcpObj->GetDhcpExpirationTime() - 5);
                NetTimeOutManager.RegisterTriggerAt(*pDhcpObj, TimeTick + 20);
                break;

            case '4':
                pDhcpObj->RenewDHCP();   // Force a Renew
                break;

            case '5':
                pDhcpObj->RebindDHCP();   // Force a Renew
                break;

            case 's':
#ifdef DHCPTEST_DEBUG
                ShowDhcpSettings(EthernetInterface);   // high level data
                // If you get an error here, read the comments at the top of the file
                PrintDhcpStatus(*pDhcpObj);   // low level data
#else
                iprintf("DHCPTEST_DEBUG not enabled\r\n");
#endif
                break;

            case 't':
                ShowDhcpSettings(GetFirstInterface());
                iprintf("\r\nRemaining lease time: %ld\r\n", pDhcpObj->GetRemainingDhcpLeaseTime());
                iprintf("Secs: %ld\r\n", Secs);
                iprintf("DhcpLeastTime: %ld\r\n", pDhcpObj->DhcpLeaseTime);
                iprintf("DhcpRenewTime: %ld\r\n", pDhcpObj->GetDhcpRenewTime());
                iprintf("DhcpRebindTime: %ld\r\n", pDhcpObj->GetDhcpRebindTime());
                break;

            case 'v':
                if (pDhcpObj->ValidDhcpLease())
                    iprintf("Lease is valid\r\n");
                else
                    iprintf("Lease is INVALID\r\n");
                break;

            default: DisplayMenu();
        }
    }
}
