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
 * Helper functions to display IP address information on a web page and
 * the serial port
 */
#include <predef.h>
#include <constants.h>
#include <ctype.h>
#include <nbrtos.h>
#include <system.h>
#include <init.h>
#include <utils.h>
#include <ipv6/ipv6_interface.h>
#include <netinterface.h>
#include <tcp.h>
#include <fdprintf.h>
#include <http.h>
#include "ip_util.h"

/**
 * @brief Source names for IPv6 assigned addresses
 */
const char *SourceName(ePrefixSource e)
{
    switch (e)
    {
        case eLinkLocal: return "Link Local";
        case eRouter: return "Router";
        case eDHCP: return "DHCP";
        case eStatic: return "Static";
        case eUnknown:
        default: return "Unknown";
    }
}

// Ignore warnings for %HI and %I format parameters in fdprintf()
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wformat-extra-args"

/**
 * @brief Convert a number of seconds into a string with format HH:MM:SS
 */
void getTimeStringFromSeconds(uint32_t seconds, char *strTime, uint32_t maxLen)
{
    uint32_t hour = 0;
    uint32_t min = 0;
    uint32_t sec = 0;
    uint32_t time = 0;

    time = seconds;
    hour = time / 3600;
    time = time % 3600;

    min = time / 60;
    time = time % 60;

    sec = time;

    snprintf(strTime, maxLen, "%02ld:%02ld:%02ld", hour, min, sec);
}

/**
 * @brief Create a string with the current time and DHCP lease time information
 */
void getLeaseTimeInfo(IPV6_PREFIX *pPrefix, char *timeStr, uint32_t maxTimeStrLen)
{
    const uint32_t maxLen = 80;
    char strCurrentTime[maxLen];
    char strRenewTime[maxLen];
    char strRebindTime[maxLen];

    timeStr[0] = '\0';
    uint32_t seconds = TimeTick / TICKS_PER_SECOND;
    getTimeStringFromSeconds(seconds, strCurrentTime, maxLen);

    // Use these for a countdown mode instead of absolute system time
    //    seconds = (pPrefix->pDHCPD->m_renewTick - TimeTick) / TICKS_PER_SECOND;
    //    getTimeStringFromSeconds(seconds, strRenewTime, maxLen);
    //
    //    seconds = (pPrefix->pDHCPD->m_rebindTick - TimeTick) / TICKS_PER_SECOND;
    //    getTimeStringFromSeconds(seconds, strRebindTime, maxLen);

    seconds = (pPrefix->pDHCPD->m_renewTick) / TICKS_PER_SECOND;
    getTimeStringFromSeconds(seconds, strRenewTime, maxLen);

    seconds = (pPrefix->pDHCPD->m_rebindTick) / TICKS_PER_SECOND;
    getTimeStringFromSeconds(seconds, strRebindTime, maxLen);

    sniprintf(timeStr, maxTimeStrLen, "Time Since Boot: %s, Renew: %s, Rebind: %s", strCurrentTime, strRenewTime, strRebindTime);
}

/**
 * @brief Web page function callback to display links to IPv4 and IPv6 addresses.
 *
 * Note that IPv6 prefix addresses may take a few seconds to be assigned.
 * In this example a HTML table is created to format the information. An
 * alternative would be to use the FUNCTIONCALL, CPPCALL or VARIABLE HTML
 * tags to provide just the addresses information in your web page,
 * and handle the formating in your HTML file.
 */
void showIpAddressesWeb(int socket, char *url)
{
    IPADDR localIP = GetSocketLocalAddr(socket);
    uint32_t remotePort = GetSocketRemotePort(socket);

    fdprintf(socket, "<br>Web browser request came from:");
    localIP.fdprint(socket);
    fdprintf(socket, ": %d<br>\r\n", remotePort);
    fdprintf(socket, "<br><strong>Device Interfaces:</strong><br>");
    int ifNumber = GetFirstInterface();

    while (ifNumber)
    {
        InterfaceBlock *ifBlock = GetInterfaceBlock(ifNumber);

        fdprintf(socket, "<hr>\r\n");
        fdprintf(socket, "<strong>Interface # %d, %s, MAC: ", ifNumber, ifBlock->GetInterfaceName());
        MACADR MacAddr = InterfaceMAC(ifNumber);
        fdShowMac(socket, &MacAddr);
        fdprintf(socket, "</strong><br>");

        //----- Display IPv4 information an link -----
        fdprintf(socket, "<br><strong>IPV4 Addresses: </strong><br>");
        fdprintf(socket, "<table border=0>\r\n");
        fdprintf(socket, "<tr><td>IP:   </td> <td>%HI</td></tr>\r\n", InterfaceIP(ifNumber));
        fdprintf(socket, "<tr><td>Mask: </td> <td>%HI</td></tr>\r\n", InterfaceMASK(ifNumber));
        fdprintf(socket, "<tr><td>DNS1: </td> <td>%HI</td></tr>\r\n", InterfaceDNS(ifNumber));
        fdprintf(socket, "<tr><td>DNS2: </td> <td>%HI</td></tr>\r\n", InterfaceDNS2(ifNumber));
        fdprintf(socket, "<tr><td>Gateway: </td> <td>%HI</td></tr>\r\n", InterfaceGate(ifNumber));
        fdprintf(socket, "<tr><td>AutoIP: </td> <td>%HI</td></tr>\r\n", InterfaceAutoIP(ifNumber));
        fdprintf(socket, "</table>\r\n");
        writestring(socket, "<br>");

        //----- Display IPv6 information an link -----
        if (GetInterfaceLink(ifNumber))   // Create link for IPv4 address
        {
            fdprintf(socket, "Make an IPv4 request: <a href=\"http://");
            InterfaceIP(ifNumber).fdprint(socket);
            fdprintf(socket, "\">");
            InterfaceIP(ifNumber).fdprint(socket);
            fdprintf(socket, "</a><br>\r\n");
        }

        //----- Display IPv6 information an link -----
        IPv6Interface *pIPv6If = IPv6Interface::GetInterfaceN(ifNumber);
        if (pIPv6If)
        {
            fdprintf(socket, "<br><strong>IPV6 Addresses:</strong><br>\r\n");

            IPV6_PREFIX *pPrefix = pIPv6If->FirstPrefix();

            fdprintf(socket, "<table border=0 cellpadding=5>\r\n");
            while (pPrefix)
            {
                if (pPrefix->bValidForInterface)   // Verify prefix is valid, and create link for IPv6 address
                {
                    ePrefixSource pSrc = pPrefix->Source();
                    fdprintf(socket, "<tr> <td><a href=\"http://[%I]\">%I</a></td> <td>Assigned by %s", pPrefix->m_IPAddress,
                             pPrefix->m_IPAddress, SourceName(pSrc));

                    if (pPrefix->pDHCPD)   // if DHCP, show lease information
                    {
                        char buf[120];
                        getLeaseTimeInfo(pPrefix, buf, 120);
                        fdiprintf(socket, ": %s", buf);
                    }
                    fdprintf(socket, "</td></tr>\r\n");
                }
                pPrefix = pPrefix->GetNext();
            }
            fdprintf(socket, "</table>\r\n");

            //----- Display DNS information -----
            IPV6_DNS *pDNS = pIPv6If->FirstDNS();
            if (pDNS) { fdiprintf(socket, "<br><strong>DNS:</strong><br>"); }
            while (pDNS)
            {
                fdiprintf(socket, "<div style=\"padding-left: 30px\"> %I - Source: %s</div>", pDNS->m_IPAddress,
                          (pDNS->pRouter) ? "Router" : "DHCP");
                pDNS = pDNS->GetNext();
            }
        }

        ifNumber = GetNextInterface(ifNumber);
    }
    fdprintf(socket, "<hr>\r\n");
}

/**
 * @brief Print IP addresses to serial port
 */
void showIpAddressesSerial()
{
    int ifNumber = GetFirstInterface();   // First interface is normally Ethernet 0

    while (ifNumber)
    {
        InterfaceBlock *ifBlock = GetInterfaceBlock(ifNumber);

        iprintf("\r\nInteface %d, %s, MAC:", ifNumber, ifBlock->GetInterfaceName());
        InterfaceMAC(ifNumber).print();
        iprintf("\r\n\r\n");

        if( !ifBlock->dhcpClient.ValidDhcpLease() )
        {
            iprintf("Device does not currently have valid DCHP lease.\r\n");
        }

        iprintf("IPv4 Addresses:\r\n");
        iprintf("IP:      %HI\r\n", InterfaceIP(ifNumber));
        iprintf("Mask:    %HI\r\n", InterfaceMASK(ifNumber));
        iprintf("DNS:     %HI\r\n", InterfaceDNS(ifNumber));
        iprintf("DNS2:    %HI\r\n", InterfaceDNS2(ifNumber));
        iprintf("Gateway: %HI\r\n", InterfaceGate(ifNumber));
        iprintf("Auto IP: %HI\r\n", InterfaceAutoIP(ifNumber));
        iprintf("\r\n");

        iprintf("IPv6 Addresses:\r\n");
        IPv6Interface *pIPv6If = IPv6Interface::GetInterfaceN(ifNumber);
        if (pIPv6If)
        {
            IPV6_PREFIX *pPrefix = pIPv6If->FirstPrefix();
            while (pPrefix)
            {
                // Check that the prefix is actually valid to use as an address for our device
                if (pPrefix->bValidForInterface && pPrefix->AgeStillValidTest())
                {
                    ePrefixSource pSrc = pPrefix->Source();
                    iprintf("%-10s: ", SourceName(pSrc));
                    pPrefix->m_IPAddress.print();
                    iprintf("/%d\r\n", pPrefix->PrefixLen);
                    //                    iprintf("/%d :%s\r\n", pPrefix->PrefixLen, SourceName(pSrc));
                    if (pPrefix->pDHCPD)   // display lease time information
                    {
                        char buf[120];
                        getLeaseTimeInfo(pPrefix, buf, 120);
                        iprintf("            %s\r\n", buf);
                    }
                }
                pPrefix = pPrefix->GetNext();
            }

            IPV6_DNS *pDNS = pIPv6If->FirstDNS();
            if (pDNS) { iprintf("\r\nDNS:\r\n"); }

            while (pDNS)
            {
                iprintf("  %I - Source: %s\r\n", pDNS->m_IPAddress, (pDNS->pRouter) ? "Router" : "DHCP");
                pDNS = pDNS->GetNext();
            }
        }

        ifNumber = GetNextInterface(ifNumber);
    }
}

/**
 * @brief Show link status on Ethernet interfaces
 */
void showLinkStatus()
{
    int ifNumber = GetFirstInterface();

    while (ifNumber)
    {
        InterfaceBlock *ifBlock = GetInterfaceBlock(ifNumber);

        if (ifBlock)
        {
            iprintf("\r\nInterface %d, %s:\r\n", ifNumber, ifBlock->GetInterfaceName());
            if (ifBlock->LinkActive())
            {
                iprintf("Link Status: UP, %dMB, ", ifBlock->LinkSpeed());
                if (ifBlock->LinkDuplex())
                {
                    iprintf("Full Duplex\r\n");
                }
                else
                {
                    iprintf("Half Duplex\r\n");
                }
            }
            else
            {
                iprintf("Link Status: DOWN\r\n");
                iprintf("** Note: If your device just booted and it is set get an IP address from a DHCP server (the default), it is probably still waiting for an IP address assignment.\r\n");
            }

        }
        ifNumber = GetNextInterface(ifNumber);
    }
}

#pragma GCC diagnostic pop
