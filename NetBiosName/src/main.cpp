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
#include <netbios.h>
#include <stdio.h>
#include <string.h>

const char *AppName = "NetBios/DNS Name Example";
const char *deviceName = "Bob-1234";

/*-------------------------------------------------------------------
 * UserMain
 * ----------------------------------------------------------------*/
void UserMain(void *pd)
{
    init();
    WaitForActiveNetwork();
    StartHttp();

    // Must come after DHCP because we need an IP address
    iprintf("Requested device name: \"%s\" with NetBIOS\r\n", deviceName);
    NetbiosEnableNameService(deviceName, TRUE);

    iprintf("If you have selected a valid NetBios name, you can type\r\n");
    iprintf(" \"//%s\" in your web browser to access the device by name\r\n", deviceName);

    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND);
    }
}
