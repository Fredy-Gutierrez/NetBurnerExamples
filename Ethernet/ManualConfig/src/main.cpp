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

/*------------------------------------------------------------------------------
 * Ethernet manual configuration test
 *----------------------------------------------------------------------------*/
#include <predef.h>
#include <stdio.h>
#include <ctype.h>
#include <init.h>
#include <ethernet.h>
#include <ethervars.h>
#include <nbrtos.h>
#include "ip_util.h"

const char *AppName = "Ethernet Speed/Duplex Test";

// External reference to Ethernet variables and functions
extern EthernetVars enet;

// Diagnostic functions used in the command dispatcher
void dEnet(void);
void ShowArp(void);
void ShowCounters(void);
void DumpTcpDebug(void);
void EnableTcpDebug(uint16_t);


/*-----------------------------------------------------------------------------
 * This function pings the address given in the buffer.
 *-----------------------------------------------------------------------------*/
void processPing(char *strIpAddr)
{
    IPADDR  destIpAddr;
    char    *cp = strIpAddr;

    while (*cp && isspace(*cp)) { cp++; }       // Trim leading white space

    // Get the address or use the default
    if (cp[0])
    {
        destIpAddr = AsciiToIp(cp);
    }
    else
    {
        int ifNumber = GetFirstInterface();  // First interface is normally Ethernet 0
        iprintf("Gateway: %HI\r\n", InterfaceGate(ifNumber));
        destIpAddr = InterfaceGate(ifNumber);
    }

    iprintf("Pinging: %I\r\n", destIpAddr);

    int rv = Ping(destIpAddr, 1 /* ID */, 1 /* Seq */, 100 /* Max ticks */);

    if (rv == -1)
        iprintf("Failed!\r\n");
    else
        iprintf("Response took %d ticks.\r\n", rv);
}



/*-----------------------------------------------------------------------------
 * Display debug menu
 *-----------------------------------------------------------------------------*/
void displayMenu(void)
{
    iprintf("\r\n----- Main Menu -----\r\n");
    iprintf("1. Set Autonegotiate mode\r\n");
    iprintf("2. Set 100Mbps Full Duplex\r\n");
    iprintf("3. Set 100Mbps Half Duplex\r\n");
    iprintf("4. Set  10Mbps Full Duplex\r\n");
    iprintf("5. Set  10Mbps Half Duplex\r\n");

    iprintf("A - Show ARP Cache\r\n");
    iprintf("C - Show Counters\r\n");
    iprintf("E - Show Ethernet Registers\r\n");
    iprintf("P - Ping (Example: \"P 192.168.1.1\")\r\n");
    iprintf("W - Show OS Seconds Counter\r\n");
    iprintf("? - Display Menu\r\n");

    showLinkStatus();
}

/*-----------------------------------------------------------------------------
 * Set the Ethernet port speed and duplex.
 * The 5441x products use a class method since three can be more than one
 * Ethernet port. All platforms prior to the 5441x devices use a standard C
 * function call. This example will only modify the primary port of an 5441x
 * multi Ethernet port device to keep things simple.
 *----------------------------------------------------------------------------*/
void SetSpeedAndDuplex(bool speed100, bool fullDuplex, bool autoNeg)
{
#ifndef _DEBUG      // cannot use manual config if debugging

    int ifNumber = GetFirstInterface();
    InterfaceBlock * ifBlock = GetInterfaceBlock(ifNumber);
    iprintf("Interface %d, %s:\r\n", ifNumber, ifBlock->GetInterfaceName());

    #if (defined MCF5441X)
        enet0.ManualEthernetConfig(speed100, fullDuplex, autoNeg);
        //enet1.ManualEthernetConfig(speed100, fullDuplex, autoNeg);  // example for multi port devices
    #else

        enet0.ManualEthernetConfig(speed100, fullDuplex, autoNeg);
    #endif

#else
    iprintf("Cannot manually configure in debug mode\r\n");
#endif
}

/*-----------------------------------------------------------------------------
 * Handle commands with a simple command dispatcher
 *----------------------------------------------------------------------------*/
void processCommand(char *buffer)
{
    switch ( toupper(buffer[0]) )
    {
        case '1':
            iprintf("Setting auto negotiate\r\n");
            //      (speed100, fullDuplex, autoNegotiate
            SetSpeedAndDuplex(false, false, true);
            break;

        case '2':
            iprintf("Setting 100Mbps, Full Duplex\r\n");
            SetSpeedAndDuplex(true, true, false);
            break;

        case '3':
            iprintf("Setting 100Mbps, Half Duplex\r\n");
            SetSpeedAndDuplex(true, false, false);
            break;

        case '4':
            iprintf("Setting 10Mbps, Full Duplex\r\n");
            SetSpeedAndDuplex(false, true, false);
            break;

        case '5':
            iprintf("Setting 10Mbps, Half Duplex\r\n");
            SetSpeedAndDuplex(false, false, false);
            break;

        case 'A':
            ShowArp();
            break;

        case 'C':
            ShowCounters();
            break;

        case 'E':
            dEnet();
            break;

        case 'P':
            processPing(buffer + 1);
            break;

        case 'W':
            iprintf("Tick Count = 0x%lX = %ld (%ld seconds)\r\n", TimeTick, TimeTick, TimeTick / TICKS_PER_SECOND);
            break;

        default: // '?'
            displayMenu();
    }
}


/*-----------------------------------------------------------------------------
 * The main task
 *-----------------------------------------------------------------------------*/
void UserMain(void *pd)
{
    init();

    char buffer[255];
    buffer[0] = '\0';

    displayMenu();
    while (1)
    {
        if (charavail())
        {
            gets(buffer);
            processCommand(buffer);
            buffer[0] = '\0';
        }
    }
}
