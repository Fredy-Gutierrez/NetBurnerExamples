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

#include <predef.h>
#include <constants.h>
#include <ctype.h>
#include <nbrtos.h>
#include <system.h>
#include <init.h>
#include <smarttrap.h>
#include <wifi/wifi.h>
#include <wifi/wifiBsp.h>


extern const unsigned long nbWifiImageLen;
extern const unsigned char nbWifiImage[];

const char *AppName = "NBWIFI-IN FirmwareUpdate";

extern "C" { void UserMain( void * pd ); }
extern void WifiReset(int resetPinNum, int connectorNum);




/*-----------------------------------------------------------------------------
 * Display Wifi Status Information
 *----------------------------------------------------------------------------*/
void DisplayWifiStatus( NB::Wifi *pNBWifiObject )
{
    NB::nbWifiDeviceInfo devInfo;
    pNBWifiObject->GetDeviceInformation( &devInfo );
    iprintf("Hardware Rev: %d.%d\r\n", devInfo.hardwareMajorRev, devInfo.hardwareMinorRev);
    iprintf("Wifi Firmware Rev: %d.%d\r\n", devInfo.softwareMajorRev, devInfo.softwareMinorRev );
}



/*-----------------------------------------------------------------------------
 * UserMain
 *----------------------------------------------------------------------------*/
void UserMain( void * pd )
{
    init();                                       // Initialize network stack
    StartHttp();                                  // Start web server, default port 80
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

    iprintf("Application: %s\r\nNNDK Revision: %s\r\n", AppName, GetReleaseTag());

    if (theWifiIntf == nullptr)
    {
        if(wifiInit.GetDriver != nullptr)
        {
            theWifiIntf = wifiInit.GetDriver();
            theWifiIntf->APIStart();
            theWifiIntf->Start();
            SetWifiSPISpeed(15000000);
        }
    }

    if (theWifiIntf == NULL)
    {
        iprintf("*** ERROR: Could not talk to Wifi module. Program Halted.\r\n");
        while (1)
            OSTimeDly(TICKS_PER_SECOND);
    }

    iprintf("\r\nCurrent Wifi Module Information:\r\n");
    DisplayWifiStatus(theWifiIntf);

    iprintf("Press any key to update firmware:\r\n");
    getchar();
    iprintf("Writing new firmware... ");

    bool ret = theWifiIntf->UpdateSlaveFirmware( nbWifiImageLen, nbWifiImage );
    iprintf("%s\n", ret ? "Done" : "FAILURE");

    iprintf("\r\nWifi Module Information:\r\n");
    DisplayWifiStatus(theWifiIntf);

    while(1) {
        OSTimeDly(TICKS_PER_SECOND);
    }
}
