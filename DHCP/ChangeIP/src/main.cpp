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

// NB Constants
#include <constants.h>

// NB Libs
#include <hal.h>
#include <dhcpclient.h>
#include <init.h>
#include <ipshow.h>
#include <netinterface.h>
#include <string.h>
#include <system.h>
#include <utils.h>

const char *AppName = "Change IPv4 Example";

// Values used for static IP addresses. Change to suit your network environment
#define STATIC_IP "10.1.1.222"
#define STATIC_MASK "255.255.255.0"
#define STATIC_GATE "10.1.1.1"
#define STATIC_DNS1 "10.1.1.1"
#define STATIC_DNS2 "8.8.8.8"

// Ignore warnings for %HI and %I format parameters in fdprintf()
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wformat-extra-args"

/*-------------------------------------------------------------------
 * Display IPv4 Settings
 *-------------------------------------------------------------------*/
void DisplayIPv4Settings()
{
    iprintf("\r\n\r\n--- RUNTIME IPv4 SETTINGS ---\r\n");

    int ifNumber = GetFirstInterface();   // First interface is normally Ethernet 0

    while (ifNumber)
    {
        // Get the InterfaceBlock associated with the interface number
        InterfaceBlock *pIfBlock = GetInterfaceBlock(ifNumber);

        // Print MAC address and interface name
        iprintf("\r\nInteface %d, %s, MAC:", ifNumber, pIfBlock->GetInterfaceName());
        InterfaceMAC(ifNumber).print();
        iprintf("\r\n");

        iprintf("Device Name: \"%s\"\r\n", pIfBlock->device_name.c_str());
        iprintf("Application Name: \"%s\"\r\n", AppName);

        iprintf("\r\nIPv4 Active Values:\r\n");
        // Display DHCP mode. Note te mode is of type NBString, so to print use the c_str() member function
        NBString addressMode = pIfBlock->ip4.mode;
        iprintf("DHCP %s: %s\r\n", pIfBlock->ip4.mode.pName, addressMode.c_str());
        // If you wanted to print without assigning to a variable, printf() requires
        // that the variable be cast to a NBString.
        // iprintf("DHCP %s: %s\r\n", pIfBlock->ip4.mode.pName, ((NBString)pIfBlock->ip4.mode).c_str() );

        iprintf("%s: %HI\r\n", pIfBlock->ip4.cur_addr.pName, pIfBlock->ip4.cur_addr.i4);
        iprintf("%s: %HI\r\n", pIfBlock->ip4.cur_mask.pName, pIfBlock->ip4.cur_mask.i4);
        iprintf("%s: %HI\r\n", pIfBlock->ip4.cur_gate.pName, pIfBlock->ip4.cur_gate.i4);
        iprintf("%s: %HI\r\n", pIfBlock->ip4.cur_dns1.pName, pIfBlock->ip4.cur_dns1.i4);
        iprintf("%s: %HI\r\n", pIfBlock->ip4.cur_dns2.pName, pIfBlock->ip4.cur_dns2.i4);

        iprintf("\r\nIPv4 Static Values:\r\n");
        IPADDR4 ipAddr4 = pIfBlock->ip4.addr;
        iprintf("%s: %HI\r\n", pIfBlock->ip4.addr.pName, ipAddr4);
        ipAddr4 = pIfBlock->ip4.mask;
        iprintf("%s: %HI\r\n", pIfBlock->ip4.addr.pName, ipAddr4);
        ipAddr4 = pIfBlock->ip4.gate;
        iprintf("%s: %HI\r\n", pIfBlock->ip4.addr.pName, ipAddr4);
        ipAddr4 = pIfBlock->ip4.dns1;
        iprintf("%s: %HI\r\n", pIfBlock->ip4.addr.pName, ipAddr4);
        ipAddr4 = pIfBlock->ip4.dns2;
        iprintf("%s: %HI\r\n", pIfBlock->ip4.addr.pName, ipAddr4);

        iprintf("\r\n");
        ifNumber = GetNextInterface(ifNumber);
    }
}

/*-------------------------------------------------------------------
 * Change to static IP settings
 *-------------------------------------------------------------------*/
void ChangeToStaticMode()
{
    int ifNumber = GetFirstInterface();   // First interface is normally Ethernet 0

    InterfaceBlock *pIfBlock = GetInterfaceBlock(ifNumber);

    // Enable this if you want to check for the mode first
    //    NBString addressMode = pIfBlock->ip4.mode;
    //    if ( addressMode == "Static")
    //    {
    //        iprintf("\r\nAlready in static IP address mode\r\n");
    //        return;
    //    }

    pIfBlock->ip4.mode = "Static";

    pIfBlock->ip4.addr = AsciiToIp4(STATIC_IP);
    pIfBlock->ip4.mask = AsciiToIp4(STATIC_MASK);
    pIfBlock->ip4.gate = AsciiToIp4(STATIC_GATE);
    pIfBlock->ip4.dns1 = AsciiToIp4(STATIC_DNS1);
    pIfBlock->ip4.dns2 = AsciiToIp4(STATIC_DNS2);

    iprintf("Setting static IP values:\r\n");
    IPADDR4 ipAddr4 = pIfBlock->ip4.addr;
    iprintf("%s: %HI\r\n", pIfBlock->ip4.addr.pName, ipAddr4);
    ipAddr4 = pIfBlock->ip4.mask;
    iprintf("%s: %HI\r\n", pIfBlock->ip4.addr.pName, ipAddr4);
    ipAddr4 = pIfBlock->ip4.gate;
    iprintf("%s: %HI\r\n", pIfBlock->ip4.addr.pName, ipAddr4);
    ipAddr4 = pIfBlock->ip4.dns1;
    iprintf("%s: %HI\r\n", pIfBlock->ip4.addr.pName, ipAddr4);
    ipAddr4 = pIfBlock->ip4.dns2;
    iprintf("%s: %HI\r\n", pIfBlock->ip4.addr.pName, ipAddr4);

    SaveConfigToStorage();
    iprintf("Rebooting....\r\n");
    OSTimeDly(TICKS_PER_SECOND * 2);
    ForceReboot();
}

/*-------------------------------------------------------------------
 * Change to static IP settings
 *-------------------------------------------------------------------*/
void ChangeToDhcpMode()
{
    int ifNumber = GetFirstInterface();   // First interface is normally Ethernet 0

    InterfaceBlock *pIfBlock = GetInterfaceBlock(ifNumber);

    // Enable this if you want to check for the mode first
    //    NBString addressMode = pIfBlock->ip4.mode;
    //    if ( addressMode == "DHCP")
    //    {
    //        iprintf("\r\nAlready in DHCP IP address mode\r\n");
    //        return;
    //    }

    pIfBlock->ip4.mode = "DHCP";

    SaveConfigToStorage();
    iprintf("Rebooting....\r\n");
    OSTimeDly(TICKS_PER_SECOND * 2);
    ForceReboot();
}

/*-------------------------------------------------------------------
 DisplayUserMenu
 Displays menu through the serial port to interact with program.
 -------------------------------------------------------------------*/
void DisplayUserMenu()
{
    iprintf("\r\n--- Main Menu ---\r\n");
    iprintf("1. Display Runtime Settings\r\n");
    iprintf("2. Change to static address mode\r\n");
    iprintf("3. Change to DHCP address moder\n");
}

/*-------------------------------------------------------------------
 UserMain task
 -------------------------------------------------------------------*/
void UserMain(void *pd)
{
    init();
    WaitForActiveNetwork();

    showIpAddresses();
    DisplayIPv4Settings();

    while (1)
    {
        char c = getchar();

        switch (toupper(c))
        {
            case '1': DisplayIPv4Settings(); break;

            case '2': ChangeToStaticMode(); break;

            case '3': ChangeToDhcpMode(); break;

            default: DisplayUserMenu();
        }
    }
}

#pragma GCC diagnostic pop
