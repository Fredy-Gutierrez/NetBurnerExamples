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

#include <ctype.h>
#include <init.h>
#include <nbrtos.h>
#include <netinterface.h>
#include <nettypes.h>
#include <predef.h>
#include <stdio.h>

const char *AppName = "Device Name";

/*-----------------------------------------------------------------------------
 * Create a device name using the MAC address.
 *----------------------------------------------------------------------------*/
void CreateDnsDeviceName(char *deviceName, uint32_t maxLen)
{
    // First interface is normally Ethernet 0. If you want to use a different interface
    // use the GetNextInterface() function to locate it.
    int ifNumber = GetFirstInterface();
    InterfaceBlock *ifBlock = GetInterfaceBlock(ifNumber);

    // Display information to serial port
    iprintf("Interface %d, %s, MAC:", ifNumber, ifBlock->GetInterfaceName());
    InterfaceMAC(ifNumber).print();
    iprintf("\r\n");

    // Display the device name currently stored in the flash configuration
    iprintf("Current device name from flash configuration: \"%s\"\r\n", ifBlock->device_name.c_str());

    // Get the MAC address and create the device name
    MACADR macAddress = InterfaceMAC(ifNumber);
    sniprintf(deviceName, maxLen, "NetBurner-%02X%02X", macAddress.GetByte(4), macAddress.GetByte(5));

    if (strncmp(deviceName, ifBlock->device_name.c_str(), 80) != 0)
    {
        iprintf("Assigning device name: %s\r\n", deviceName);
        ifBlock->device_name = deviceName;
        SaveConfigToStorage();

        iprintf("Reboot device for new device name to take effect\r\n");
    }
    else
    {
        iprintf("New name already matches configuration\r\n");
    }
}

/*-----------------------------------------------------------------------------
 * UserMain
 *----------------------------------------------------------------------------*/
void UserMain(void *pd)
{
    const uint32_t maxLen = 32;   // Max length of device name

    init();
    StartHttp();

    // Create a DHCP device name using the last 2 octets of the MAC address
    char deviceName[maxLen];
    CreateDnsDeviceName(deviceName, maxLen);

    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND);
    }
}
