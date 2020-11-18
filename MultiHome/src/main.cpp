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

// NB Definitions
#include <predef.h>

// NB Libs
#include <ctype.h>
#include <init.h>
#include <multihome.h>
#include <netinterface.h>
#include <stdio.h>
#include <tcp.h>
#include <fdprintf.h>
// Check for Multihome options and display compiler warnings if not enabled

#ifndef MULTIHOME
#error This example requires that MULTIHOME be uncommented in the header      \
         file nburn\nbrtos\include\predef.h, and that the system files be rebuilt.
#endif

// App name for IPSetup
const char *AppName = "Multihome Example";

extern "C"
{
    void WebDisplayIpSettings(int sock, PCSTR url);
}

void ShowIPOnFd(int fd, IPADDR ia);

InterfaceBlock *DhcpIb, *StaticIb, *MultiHome1Ib;

/*-------------------------------------------------------------------
 Display IP setting on web page
 -------------------------------------------------------------------*/
void WebDisplayIpSettings(int sock, PCSTR url)
{
    // Show header message indicating source and destination
    IPADDR remoteIP = GetSocketRemoteAddr(sock);
    remoteIP.fdprint(sock);
    IPADDR localIP = GetSocketLocalAddr(sock);
    localIP.fdprint(sock);
    fdprintf(sock, "Received request from %I on IP address: %I \r\n<br><br>",remoteIP,localIP);

    // Show IP address for multihome
    writestring(sock, "Valid IP Address for this device:<br>\r\n");
    fdprintf(sock, "DHCP: %hI <br>\r\n",(IPADDR4)DhcpIb->ip4.cur_addr);

    fdprintf(sock, "Multihome: %hI <br>\r\n",(IPADDR4)MultiHome1Ib->ip4.cur_addr);

}

/*-------------------------------------------------------------------
 UserMain
 -------------------------------------------------------------------*/
void UserMain(void *pd)
{
    init();
    StartHttp();

    // Add the multihome interface at fixed ip address, IP, Mask, Gateway
    AddInterface(AsciiToIp4("10.1.1.240"), AsciiToIp4("255.255.255.0"), IPADDR4::NullIP());

    int DhcpInterface = GetFirstInterface();     // Get first interface identifier
    DhcpIb = GetInterfaceBlock(DhcpInterface);   // Get interface data
    iprintf("DHCP IP Address: %hI r\n",(IPADDR4)DhcpIb->ip4.cur_addr);

    int StaticInterface = GetNextInterface(DhcpInterface);   // Get next interface identifier
    StaticIb = GetInterfaceBlock(StaticInterface);           // Get interface data
    iprintf("AutoIP Address: %hI\r\n",(IPADDR4)StaticIb->ip4.cur_addr);

    int MultiHome1Interface = GetNextInterface(StaticInterface);   // Get next interface identifier
    MultiHome1Ib = GetInterfaceBlock(MultiHome1Interface);         // Get interface data
    iprintf("Multihome 1 IP Address: %hI\r\n",(IPADDR4)MultiHome1Ib->ip4.cur_addr);

    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND * 5);
    }
}
