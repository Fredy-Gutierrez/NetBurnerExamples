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

const char *AppName = "SystemParamsModifyEthernet";

// This is the function that displays the config tree structure
void ShowTree();

// This external global variable exposes the boot config values
extern MonitorRecord monitor_config;

/**
 *  UpdateConfigString
 *
 *  Takes a config_string, prints the current value, and updates it from
 *  user input.
 */
void UpdateConfigString(config_string &confStr)
{
    // Display the old value and get a new value from user input
    NBString confName;
    confStr.GetNameValue(confName);
    iprintf("%s old value: %s\r\n", confName.c_str(), confStr.c_str());
    char newVal[80];
    iprintf("   Enter new value: ");
    fgets(newVal, 80, stdin);
    newVal[strlen(newVal) - 1] = '\0';   // Replace return character with null
    iprintf("\r\n");

    // Assign the string value
    confStr = newVal;
    iprintf("%s new value: %s\r\n", confName.c_str(), confStr.c_str());

    // Now force it to save
    SaveConfigToStorage();
}

/**
 *  UpdateConfigInt
 *
 *  Takes a config_int, prints the current value, and updates it from
 *  user input.
 */
void UpdateConfigInt(config_int &confInt)
{
    // Display the old value and get a new value from user input
    NBString confName;
    confInt.GetNameValue(confName);
    iprintf("%s old value: %d\r\n", confName.c_str(), int(confInt));
    char newVal[25];
    iprintf("     Enter new value: ");
    fgets(newVal, 25, stdin);
    iprintf("\r\n");

    // Assign the int value
    confInt = atoi(newVal);
    iprintf("%s new value: %d\r\n", confName.c_str(), int(confInt));

    // Now force it to save
    SaveConfigToStorage();
}

/**
 *  UpdateConfigIpaddr
 *
 *  Takes a config_IPADDR4, prints the current value, and updates it from
 *  user input.
 */
void UpdateConfigIpaddr(config_IPADDR4 &confIp)
{
    // Display the old value and get a new value from user input
    NBString confName;
    NBString confVal;
    confIp.GetNameValue(confName);
    confIp.GetTextValue(confVal);
    iprintf("%s old value: %s\r\n", confName.c_str(), confVal.c_str());

    // Get the new value from user input
    char newVal[80];
    iprintf("   Enter new value: ");
    fgets(newVal, 80, stdin);
    newVal[strlen(newVal) - 1] = '\0';   // Replace return character with null
    iprintf("\r\n");
    confIp = AsciiToIp4(newVal);

    // Assign the IP address value
    confIp.GetTextValue(confVal);
    iprintf("%s new value: %s\r\n", confName.c_str(), confVal.c_str());

    // Now force it to save
    SaveConfigToStorage();
}

/**
 *  UpdateConfigChooser
 *
 *  Takes a config_chooser, prints the current value, and updates it from
 *  user input.
 */
void UpdateConfigChooser(config_chooser &confChooser)
{
    // Display the old value and get a new value from user input
    NBString confName;
    confChooser.GetNameValue(confName);
    iprintf("%s old value: %s\r\n", confName.c_str(), NBString(confChooser).c_str());

    // Get the new value from user input
    char newVal[80];
    iprintf("   Enter new value. Choices are {%s}: ", confChooser.GetChoices().c_str());
    fgets(newVal, 80, stdin);
    newVal[strlen(newVal) - 1] = '\0';   // Replace return character with null
    iprintf("\r\n");
    confChooser = newVal;

    // Assign the IP address value
    iprintf("%s new value: %s\r\n", confName.c_str(), NBString(confChooser).c_str());

    // Now force it to save
    SaveConfigToStorage();
}

/**
 *  ShowCommandList
 *
 *  Prints out a list of all available commands
 */
void ShowCommandList()
{
    iprintf("\r\nCommand List\r\n");
    iprintf("------------\r\n");
    iprintf("   A) Show and Set IPv4 Static IP Address\r\n");
    iprintf("   C) Show Config Tree\r\n");
    iprintf("   G) Show and Set IPv4 Static IP Gateway\r\n");
    iprintf("   M) Show and Set IPv4 Static IP Mask\r\n");
    iprintf("   N) Show and Set Device Name\r\n");
    iprintf("   O) Show and Set IPv4 Mode\r\n");
    iprintf("   U) Show and Set Discover Server URL\r\n");
    iprintf("   ?) Show Commands\r\n");
}

/**
 *  UserMain
 *
 *  Main entry point for the example
 */
void UserMain(void *pd)
{
    // Setup everything the device needs to get going
    init();                   // Initialize network stack
    WaitForActiveNetwork();   // Wait for DHCP address

    // We need the fd of our serial port in order to print the config records to it, so close it and
    // reopen it.
    SerialClose(0);
    SimpleOpenSerial(0, 115200);

    iprintf("Config Tree Demo built at %s on %s\r\n'?' for commands\r\n", __TIME__, __DATE__);
    iprintf("IP:            %hI\r\n", InterfaceIP(GetFirstInterface()));
    iprintf("AutoIP :       %hI\r\n", InterfaceAutoIP(GetFirstInterface()));
    iprintf("Gateway:       %hI\r\n", InterfaceGate(GetFirstInterface()));

    // Get the first interface block available (typically Ethernet0)
    InterfaceBlock *firstInterface = GetInterfaceBlock(GetFirstInterface());

    ShowCommandList();

    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND * 1);

        if (charavail())
        {
            char c = getchar();
            iprintf("Read char %c\r\n", c);

            if (c == 'A')
            {
                // Display and set the IPv4 static address
                UpdateConfigIpaddr(firstInterface->ip4.addr);
            }

            if (c == 'C') { ShowTree(); }

            if (c == 'G')
            {
                // Display and set the IPv4 static gateway
                UpdateConfigIpaddr(firstInterface->ip4.gate);
            }

            if (c == 'M')
            {
                // Display and set the IPv4 static mask
                UpdateConfigIpaddr(firstInterface->ip4.mask);
            }

            if (c == 'N')
            {
                // Display and set the device name
                UpdateConfigString(firstInterface->device_name);
            }

            if (c == 'O')
            {
                // Display and set the IPv4 mode
                UpdateConfigChooser(firstInterface->ip4.mode);
            }

            if (c == 'U')
            {
                // Display and set the discover server URL
                UpdateConfigString(firstInterface->discovery_server);
            }

            if (c == '?') { ShowCommandList(); }
        }
    }
}
