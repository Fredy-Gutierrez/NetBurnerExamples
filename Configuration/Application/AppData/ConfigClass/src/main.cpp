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

/**
 *  The use of config objects should be done at a global scope.  They can contain any number of member,
 *  which should be given a default value and an name used as an identifier.  The name value for each
 *  member variable should be unique, otherwise it can lead to issues inside the config tree.
 */
class TempRange : public config_obj
{
   public:
    config_int m_maxTemp{100, "MaxTemp"};
    config_int m_setTemp{30, "SetTemp"};
    config_int m_minTemp{10, "MinTemp"};
    ConfigEndMarker;   // No new data members below this line

    TempRange(const char *name, const char *desc = nullptr) : config_obj(name, desc){};
    TempRange(config_obj &owner, const char *name, const char *desc = nullptr) : config_obj(owner, name, desc){};
};

class Thermostat : public TempRange
{
   public:
    config_int m_tempRec1{101, "TempRecord_1"};
    config_int m_tempRec2{102, "TempRecord_2"};
    config_bool m_active{true, "Active"};
    config_string m_location{"Warehouse 1", "Thermostat Location"};
    config_chooser m_tempScale{"Temperature Scale", "Fahrenheit", "Fahrenheit,Celsius,Kelvin"};
    ConfigEndMarker;   // No new data members below this line

    Thermostat(const char *name, const char *desc = nullptr) : TempRange(name, desc){};
    Thermostat(config_obj &owner, const char *name, const char *desc = nullptr) : TempRange(owner, name, desc){};
};

/**
 *  Create the actual objects that will be stored in the config record.  Generally speaking, they should
 *  be attached to the appdata config object.  Note that each object has a different name, as mentioned above.
 *  Failure to give every object a unique name can (and probably will) cause issues inside the config tree.
 */
static Thermostat thermo(appdata, "Thermostat", "The primary thermostat");

const char *AppName = "ConfigClass";

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
 *  ShowCommandList
 *
 *  Prints out a list of all available commands
 */
void ShowCommandList()
{
    iprintf("\r\nCommand List\r\n");
    iprintf("------------\r\n");
    iprintf("   C) Show Config Tree\r\n");
    iprintf("   L) Show and Set Thermostat Location\r\n");
    iprintf("   M) Show and Set Thermostat Max Temp\r\n");
    iprintf("   N) Show and Set Thermostat Min Temp\r\n");
    iprintf("   S) Show and Set Thermostat Set Temp\r\n");
    iprintf("   T) Show Thermostat Recorded Temps\r\n");
    iprintf("   ?) Show Command List\r\n");
}

/**
*  ProcessCommand
*
*  Process serial port menu commands
*/
void ProcessCommand(char cmd)
{
    switch (toupper(cmd))
    {
        case 'C':
        {
            ShowTree();
            break;
        }

        case 'L':
        {
            // Display and set the location for thermostat
            UpdateConfigString(thermo.m_location);
            break;
        }

        case 'M':
        {
            // Display and set the max temperatures
            UpdateConfigInt(thermo.m_maxTemp);
            break;
        }

        case 'N':
        {
            // Display and set the min temperatures
            UpdateConfigInt(thermo.m_minTemp);
            break;
        }

        case 'S':
        {
            // Display and set the set temperatures
            UpdateConfigInt(thermo.m_setTemp);
            break;
        }

        case 'T':
        {
            iprintf("Thermostat Temp Records:\r\n");
            iprintf("     Temp Record 1: %d\r\n", int(thermo.m_tempRec1));
            iprintf("     Temp Record 2: %d\r\n", int(thermo.m_tempRec2));
            break;
        }

        case '?':
        default:
        {
            ShowCommandList();
            break;
        }
    }
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
            ProcessCommand(c);
        }
    }
}
