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
#include <ipshow.h>
#include <config_server.h>

/**
 *  The use of config objects should be done at a global scope.  They can contain any number of member,
 *  which should be given a default value and an name used as an identifier.  The name value for each
 *  member variable should be unique, otherwise it can lead to issues inside the config tree.
 */
class Thermostat1 : public config_obj
{
   public:
    config_int m_maxTemp{100, "MaxTemp"};
    config_int m_setTemp{30, "SetTemp"};
    config_int m_minTemp{10, "MinTemp"};
    config_int m_powerMax{200, "PowerMax"};
    config_bool m_active{true, "Active"};
    ConfigEndMarker;   // No new data members below this line

    explicit Thermostat1(const char *name, const char *desc) : config_obj(name, desc){};
    Thermostat1(config_obj &owner, const char *name, const char *desc) : config_obj(owner, name, desc){};
};

class Thermostat2 : public Thermostat1
{
   public:
    config_int m_tempRec1{101, "TempRecord_1"};
    config_int m_tempRec2{102, "TempRecord_2"};
    ConfigEndMarker;   // No new data members below this line

    explicit Thermostat2(const char *name, const char *desc) : Thermostat1(name, desc){};
    Thermostat2(config_obj &owner, const char *name, const char *desc) : Thermostat1(owner, name, desc){};
};

class EmailNotify : public config_obj
{
   public:
    config_string m_normal{"Bob@someplace.com", "Normal"};
    config_string m_emergency{"Fred@someplace.com", "Emergency"};
    config_chooser m_choose{"Actions", "Alert", "Alert,TurnOff,Crash"};
    ConfigEndMarker;   // No new data members below this line

    explicit EmailNotify(const char *name, const char *desc) : config_obj(name, desc){};
    EmailNotify(config_obj &owner, const char *name, const char *desc) : config_obj(owner, name, desc){};
};

/**
 *  Create the actual objects that will be stored in the config record.  Generally speaking, they should
 *  be attached to the appdata config object.  Note that each object has a different name, as mentioned above.
 *  Failure to give every object a unique name can (and probably will) cause issues inside the config tree.
 */
static Thermostat1 thermo1(appdata, "Thermostat1", "Our first thermostat.");
static Thermostat2 thermo2(appdata, "Thermostat2", "Our second thermostat.");
static EmailNotify emailNot(appdata, "Email", "Email to send notifications to.");

static config_string gSingleString{appdata, "MyString", "SName"};
config_int gMyOwnVal{appdata, 199, "MyOwnValue"};

const char *AppName = "CustomWebConfig";

int fd; 	// serial port file descriptor

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
    iprintf("   E) Show Email Notify\r\n");
    iprintf("   F) Show and Set Email Normal Contact Address\r\n");
    iprintf("   N) Show IP for First Interface\r\n");
    iprintf("   S) Show and Set gSingleString\r\n");
    iprintf("   T) Show Thermostat 1\r\n");
    iprintf("   V) Show and Set gMyOwnVal\r\n");
    iprintf("   2) Show Thermostat 2\r\n");
    iprintf("   6) Show Interface Values\r\n");
    iprintf("   ?) Show Command List\r\n");
}

/**
 *  ProcessCommand
 *
 *  Process serial port menu commands
 */
void ProcessCommand( char cmd )
{
	switch ( toupper( cmd ) )
	{
		case 'C':
        {
            ShowTree();
            break;
        }

		case 'E':
		{
            emailNot.RenderToFd(fd, true);
			break;
		}

		case 'F':
        {
			// Show and update the value
			UpdateConfigString(emailNot.m_normal);
			break;
		}

		case 'N':
		{
            InterfaceIP(GetFirstInterface());
		    break;
		}

		case 'S':
        {
            UpdateConfigString(gSingleString);
            break;
        }

		case 'T':
		{
            thermo1.RenderToFd(fd, true);
			break;
		}

		case 'V':
        {
            UpdateConfigInt(gMyOwnVal);
            break;
        }

		case '2':
		{
            thermo2.RenderToFd(fd, true);
			break;
		}

		case '6':
        {
            showIpAddresses();
            iprintf("Time = %ld\r\n", Secs);
            break;
        }

		default:
			ShowCommandList();
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
    init();                                       // Initialize network stack
    EnableConfigMirror();                         // Calling this function forces it to get linked
                                                  // during compilation, which will enable the config mirror. 

    StartHttp();                                  // Start web server, default port 80
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

    // We need the fd of our serial port in order to print the config records to it, so close it and
    // reopen it.
    SerialClose(0);
    fd = SimpleOpenSerial(0, 115200);

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
