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
#include <http.h>
#include <init.h>
#include <netinterface.h>
#include <serial.h>
#include <stdlib.h>
#include <ipshow.h>		// Utility functions to display IP address information
#include <config_server.h>

const char *AppName = "BasicWebConfig";

void ShowTree();		// Utility function to display configuration tree to stdout serial port

/**
 * ShowCommandList
 *
 * Prints out a list of all available commands
 */
void ShowCommandList()
{
    iprintf("\r\nCommand List\r\n");
    iprintf("------------\r\n");
    iprintf("   C) Show Config Tree\r\n");
    iprintf("   N) Show IP for First Interface\r\n");
    iprintf("   6) Show Interface Values\r\n");
    iprintf("   ?) Show Command List\r\n");
}


/**
 * ProcessCommand
 *
 * Process a serial port menu command
 */
void ProcessCommand( char cmd )
{
	switch ( toupper(cmd)) {
		case 'C':
            ShowTree();
            break;

		case 'N':
            InterfaceIP(GetFirstInterface());;
		    break;

		case '6':
            showIpAddresses();
            iprintf("Time = %ld\r\n", Secs);
            break;

		default:
            ShowCommandList();
	}
}

/**
 * UserMain
 *
 * Main entry point for the example
 */
void UserMain(void *pd)
{
    // Setup everything the device needs to get going
    init();                                       // Initialize system and network
    EnableConfigMirror();                         // Calling this function forces it to get linked
                                                  // during compilation, which will enable the config mirror. 

    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address
    StartHttp();                                  // Start web server, default port 80

    iprintf("Config Tree Demo built at %s on %s\r\n'?' for commands\r\n", __TIME__, __DATE__);
    iprintf("IP:            %hI\r\n", InterfaceIP(GetFirstInterface()));
    iprintf("AutoIP :       %hI\r\n", InterfaceAutoIP(GetFirstInterface()));
    iprintf("Gateway:       %hI\r\n", InterfaceGate(GetFirstInterface()));

    ShowCommandList();

    while (1)
    {
        if (charavail())
        {
            char c = getchar();
            ProcessCommand(c);
        }
    }
}
