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

#include <dns.h>
#include <init.h>
#include <netinterface.h>
#include <stdio.h>

const char *AppName = "DNS Example";

/*-------------------------------------------------------------------
 * Execute the DNS request. Check for both IPv4 and IPv6 entries.
 * ------------------------------------------------------------------*/
void ExecuteDNSRequest(char *name)
{
    IPADDR IpAddress;
    IPADDR4 IpAddress4;
    IPADDR6 IpAddress6;

    IpAddress4.SetNull();
    IpAddress6.SetNull();

    // Note: this call is using the DNS server from the system runtime value which could
    // be from DHCP or static settings. You can also explicitly specify a DNS server
    // in the function call parameters.
    iprintf("looking up name: \"%s\"\r\n", name);
    int status = GetHostByName(name, &IpAddress, IPADDR::NullIP(), TICKS_PER_SECOND * 5);

    if (status == DNS_OK) { iprintf("DNS succeeded. %s resolves to %I\r\n", name, IpAddress); }
    else
    {
        if (status == DNS_TIMEOUT)
            iprintf("DNS Error: TIMEOUT\r\n");
        else if (status == DNS_NOSUCHNAME)
            iprintf("DNS Error: NO SUCH NAME\r\n");
        else
            iprintf("DNS Error: %d\r\n", status);
    }

    // Resolve IPv4 IP address
    status = GetHostByName4(name, &IpAddress4, IPADDR4::NullIP(), TICKS_PER_SECOND * 5);
    if (status == DNS_OK) { iprintf("DNS(V4) succeeded. %s resolves to %hI\r\n", name, IpAddress4); }
    else
    {
        if (status == DNS_TIMEOUT)
            iprintf("DNS(V4) Error: TIMEOUT\r\n");
        else if (status == DNS_NOSUCHNAME)
            iprintf("DNS(V4) Error: NO SUCH NAME\r\n");
        else
            iprintf("DNS(V4) Error: %d\r\n", status);
    }

    // Resolve IPv6 IP address
    status = GetHostByName6(name, &IpAddress6, IPADDR::NullIP(), TICKS_PER_SECOND * 5, DNS_AAAA, 0);
    if (status == DNS_OK) { iprintf("DNS(V6) succeeded. %s resolves to %I\r\n", name, IpAddress6); }
    else
    {
        if (status == DNS_TIMEOUT)
            iprintf("DNS(V6) Error: TIMEOUT\r\n");
        else if (status == DNS_NOSUCHNAME)
            iprintf("DNS(V6) Error: NO SUCH NAME\r\n");
        else
            iprintf("DNS(V6) Error: %d\r\n", status);
    }
}

/*-----------------------------------------------------------------------------
 * Verify device IP settings are set so DNS can be used. The device must have
 * an active link, DNS IP address, and network Gateway.
 * ---------------------------------------------------------------------------*/
bool CanInterfaceDoDNS(int ifNumber)
{
    if (InterfaceLinkActive(ifNumber) && (InterfaceDNS(ifNumber).NotNull()) && (InterfaceGate(ifNumber).NotNull())) return true;
    return false;
}

/*-----------------------------------------------------------------------------
 * User Main
 *---------------------------------------------------------------------------*/
void UserMain(void *pd)
{
    init();                                        // Initialize network
    WaitForActiveNetwork(TICKS_PER_SECOND * 10);   // Wait for link on at least one interface

    while (1)
    {
        int ifNumber = GetFirstInterface();
        while (ifNumber)
        {
            iprintf("Interface[%s]: \r\n", InterfaceName(ifNumber));
            if (InterfaceLinkActive(ifNumber))
                iprintf("Link UP\r\n");
            else
                iprintf("Link DOWN                       <---Problem\r\n");

            if (InterfaceDNS(ifNumber).NotNull())
                iprintf("Has a DNS address (%hI)\r\n", InterfaceDNS(ifNumber));
            else
                iprintf("Does not have a DNS address     <---Problem\r\n");

            if (InterfaceGate(ifNumber).NotNull())
                iprintf("Has a Gateway address(%hI)\r\n", InterfaceGate(ifNumber));
            else
                iprintf("Does not have a Gateway address <---Problem\r\n");

            if (CanInterfaceDoDNS(ifNumber))
            {
                iprintf("This interface is capable of doing DNS!\r\n");
                break;
            }

            ifNumber = GetNextInterface(ifNumber);
        }

        if (CanInterfaceDoDNS(ifNumber))
        {
            char name[128];
            iprintf("\r\nYou are going to be prompted for a name to look up\r\n");
            iprintf("Some interesting names to try:\r\n");
            iprintf("google.com              Will resolve both ipv4 and ipv6 addresses.\r\n");
            iprintf("ipv6.vm1.test-ipv6.com  Will only resolve IPV6 address\r\n");
            iprintf("www.netburner.com       Our personal favorite\r\n");

            while (1)
            {
                iprintf("\r\n\nEnter name to resolve (eg. www.netburner.com): \r\n");
                fgets(name, 128, stdin);
                if (name[0]) ExecuteDNSRequest(name);
            }
        }
        else
        {
            iprintf("Found no interface capable of doing DNS\r\nRequires Link, DNS IP address and Gateway IP address\r\n");
            iprintf("Hit enter to try again\r\n");
            getchar();
        }
    }
}
