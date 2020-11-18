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

const char *AppName = "SystemParamsModifyBoot";

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
 *  UpdateConfigBool
 *
 *  Takes a config_bool, prints the current value, and updates it from
 *  user input.
 */
void UpdateConfigBool(config_bool &confBool)
{
    // Display the old value and get a new value from user input
    NBString confName;
    confBool.GetNameValue(confName);
    iprintf("%s old value: %d\r\n", confName.c_str(), int(confBool));
    char newVal[25];
    iprintf("     Enter new value [T\\F]: ");
    fgets(newVal, 25, stdin);
    iprintf("\r\n");

    // Assign the int value
    confBool = (newVal[0] == 'T' || newVal[0] == 't');
    iprintf("%s new value: %d\r\n", confName.c_str(), int(confBool));

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
    iprintf("   A) Show and Set Abort Command\r\n");
    iprintf("   B) Show and Set Baud Rate\r\n");
    iprintf("   C) Show Config Tree\r\n");
    iprintf("   D) Show and Set Boot Delay\r\n");
    iprintf("   Q) Show and Set Quiet Boot\r\n");
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
                // Display and set the abort string
                UpdateConfigString(monitor_config.abortbootcmd);
            }

            if (c == 'B')
            {
                // Display and set baud rate
                UpdateConfigInt(monitor_config.Baud);
            }

            if (c == 'C') { ShowTree(); }

            if (c == 'D')
            {
                // Display and set boot delay
                UpdateConfigInt(monitor_config.BootDelay);
            }

            if (c == 'Q')
            {
                // Display and set quiet boot
                UpdateConfigBool(monitor_config.Quiet);
            }

            if (c == '?') { ShowCommandList(); }
        }
    }
}
