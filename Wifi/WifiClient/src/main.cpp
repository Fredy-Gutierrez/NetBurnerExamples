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
#include <stdio.h>
#include <ctype.h>
#include <startnet.h>
#include <init.h>
#include <wifi/wifi.h>
#include <wifi/wifiDriver.h>
#include <qspi.h>

extern "C" {
    void UserMain(void * pd);
}

const char * AppName="NBWIFIIN Wifi Client";

/*-------------------------------------------------------------------
 * MCF 5270 and 5234 platforms should enable spread spectrum mode
 * to reduce the possibiltiy of interference with the wifi radio.
 *------------------------------------------------------------------*/
#if (defined MCF5270 || defined MCF5234)
#include <sim.h>
/**************************************************************/
// The Following section of code turns on the spread spectrum
//   for the PLL and also reduces pin drive strengh
// This should be used to reduce EMI emissions
/**************************************************************/
   #define FM_RATE (0x1000)   // fref/40
   //#define FM_RATE (0)         // fref/80

   //#define FM_DEPTH   (0)         // FM disabled
   #define FM_DEPTH   (0x400)      // 1% of fsys\2
   //#define FM_DEPTH   (0x800)      // 2% of fsys\2

   #define FM_EXP (128)

   #define SYNSR_LOCK_BIT 0x00000008


/*-----------------------------------------------------------------------------
 * Enable Spread Spectrum
 *----------------------------------------------------------------------------*/
void EnableSpreadSpectrum()
{
   //sim.clock.syncr |= 0x8000;
   sim.gpio.dscr_eim = 0;
   sim.gpio.dscr_feci2c = 0;
   sim.gpio.dscr_uart = 0;
   sim.gpio.dscr_qspi = 0;
   sim.gpio.dscr_timer = 0;

   //syncr factory setting = 0x03000000, MFD = 011, RFD = 000
   sim.clock.syncr |= FM_EXP; // set to calculated EXP
   sim.clock.syncr &= ~0x00000C00; // Clear Depth fields to disable modulation
   while((sim.clock.synsr & SYNSR_LOCK_BIT) != SYNSR_LOCK_BIT);  //wait to lock

   sim.clock.syncr |= 0x00080000; //MFD = 011, RFD = 001
   sim.clock.syncr |= ( FM_RATE | FM_DEPTH);
   while((sim.clock.synsr & SYNSR_LOCK_BIT) != SYNSR_LOCK_BIT);  //wait to lock

   sim.clock.syncr &= ~0x00080000; //MFD = 011, RFD = 000

   while((sim.clock.synsr & 2) != 2);  // Wait for calibration to be done

/********************************************************************/
// EMI reduction code done
/**************************************************************/

   OSTimeDly(2);

   if((sim.clock.synsr & 1) == 1)
      iprintf("Frequency Modulation of Clock Successfully Calibrated!\r\n");
   else
      iprintf("Frequency Modulation of Clock FAILED Calibration!\r\n");
}
#endif


void DisplaySecurity( int security )
{
    const char *valueLabel;

    switch ( security )
    {
        case SEC_VALUE_OPEN:    { valueLabel = SEC_LABEL_OPEN; break; }
        case SEC_VALUE_WEP:     { valueLabel = SEC_LABEL_WEP;  break; }
        case SEC_VALUE_WPA:     { valueLabel = SEC_LABEL_WPA;  break; }
        case SEC_VALUE_WPA2:    { valueLabel = SEC_LABEL_WPA2; break; }
        case SEC_VALUE_WPS:     { valueLabel = SEC_LABEL_WPS;  break; }
        case SEC_VALUE_UNKNOWN:
        default:                { valueLabel = SEC_LABEL_UNKNOWN; break; }
    }
    iprintf("%s", valueLabel);
}


void DisplayCipher( int cipher )
{
    const char *valueLabel;

    switch ( cipher )
    {
        case CIPH_VALUE_NONE:   { valueLabel = CIPH_LABEL_NONE;  break; }
        case CIPH_VALUE_TKIP:   { valueLabel = CIPH_LABEL_TKIP;  break; }
        case CIPH_VALUE_AES:    { valueLabel = CIPH_LABEL_AES;   break; }
        case CIPH_VALUE_MIXED:  { valueLabel = CIPH_LABEL_MIXED; break; }
        default:                { valueLabel = CIPH_LABEL_UNKNOWN; break; } 
    }
    iprintf("%s", valueLabel);
}


/*-----------------------------------------------------------------------------
 * Display Wifi Status Information
 *----------------------------------------------------------------------------*/
void DisplayWifiStatus( NB::Wifi *pNBWifiObject )
{
    const int bufSize = 80;

    iprintf("\r\nETHERNET RUNTIME IP SETTINGS\r\n");

    // This example is for the development kit, which has an Ethernet interface
    // as its first interface. This information is here for completeness and to
    // show the difference in settings between Ethernet and Wifi.
    int ifNumber = GetFirstInterface();
    iprintf("IP Address: "); ShowIP( InterfaceIP( ifNumber ) ); iprintf("\r\n");
    iprintf("IP Mask:    "); ShowIP( InterfaceMASK( ifNumber ) ); iprintf("\r\n");
    iprintf("IP Gateway: "); ShowIP( InterfaceGate( ifNumber ) ); iprintf("\r\n");
    iprintf("IP DNS:     "); ShowIP( InterfaceDNS( ifNumber ) ); iprintf("\r\n");
    MACADR macAddress = InterfaceMAC( ifNumber );
    iprintf("MAC Address: "); ShowMac( &macAddress ); iprintf("\r\n");

    ifNumber = pNBWifiObject->GetSystemInterfaceNumber();
    iprintf("\r\nWIFI RUNTIME IP SETTINGS\r\n");
    if (ifNumber <= 0)
    {
        iprintf("Wifi interface not initialized, interface number = %d\r\n", ifNumber);
        return;
    }
    iprintf("Interface Number: %d\r\n", ifNumber);
    iprintf("IP Address: "); ShowIP( InterfaceIP( ifNumber ) ); iprintf("\r\n");
    iprintf("IP Mask:    "); ShowIP( InterfaceMASK( ifNumber ) ); iprintf("\r\n");
    iprintf("IP Gateway: "); ShowIP( InterfaceGate( ifNumber ) ); iprintf("\r\n");
    iprintf("IP DNS:     "); ShowIP( InterfaceDNS( ifNumber ) ); iprintf("\r\n");
    macAddress = InterfaceMAC( ifNumber );
    iprintf("MAC Address: "); ShowMac( &macAddress ); iprintf("\r\n");


    iprintf("\r\nWIFI STATUS FUNCTIONS\r\n");

    char buf[bufSize];
    for ( int i = 0; i < bufSize; i++ )
        buf[i] = '\0';

    // Service Set Id (SSID) of AP we are trying to connect to. This is the name of the network.
    int len = pNBWifiObject->GetCurSSID( buf, bufSize );
    iprintf("SSID: \"%s\", len = %d\r\n", buf, len );

    // Basic Service Set Id (BSSID). This is the MAC address of the AP.
    MACADR bssidMacAddr  = pNBWifiObject->GetCurBSSID();
    iprintf("BSSID: ");
    ShowMac( &bssidMacAddr);
    iprintf("\r\n");

    iprintf("RSSI: %d\r\n", pNBWifiObject->GetSignalStrength());
    iprintf("Channel: %d\r\n", pNBWifiObject->GetCurChannel());

    iprintf("Security: ");
    int security = pNBWifiObject->GetSecurity();
    DisplaySecurity( security );
    iprintf("\r\n");

    iprintf("Cipher: ");
    int cipher = pNBWifiObject->GetCipher();
    DisplayCipher( cipher );
    iprintf("\r\n");

    if (pNBWifiObject->Connected())
    	iprintf("Connected\r\n");
    else
    	iprintf("Not Connected\r\n");


    // To access the Wifi device information structure we need to call the GetDeviceInformation() function
    // of the Wifi object, so we use the pointer to the object: pNBWifiObject. The NBWifi:: is needed
    // because the Wifi driver source code uses a C++ namespace called NBWifi. The GetDeviceInformation()
    // function uses a system buffer pointed to by a PoolPtr to store the results. This buffer must be
    // freed by your application when you are done using it by calling FreeBuffer();


    NB::nbWifiDeviceInfo devInfo;
    pNBWifiObject->GetDeviceInformation( &devInfo );
    iprintf("Hardware Rev: %d.%d\r\n", devInfo.hardwareMajorRev, devInfo.hardwareMinorRev);
    iprintf("Wifi Firmware Rev: %d.%d\r\n", devInfo.softwareMajorRev, devInfo.softwareMinorRev );
}


void DisplayWifiConfigRecordParameters( NB::Wifi *pNBWifiObject )
{
    // Make sure that we were able to initiate the wifi object
    if( pNBWifiObject == nullptr )
    {
        // Try to get the wifi interface if we don't already have it
        int ifnumWifi = InitWifi_SPI();
        if( ifnumWifi > 0 )
    {
            pNBWifiObject = NB::Wifi::GetDriverByInterfaceNumber( ifnumWifi );
        }

        // If we still don't have it, bail out
        if( pNBWifiObject == nullptr || !pNBWifiObject->Connected() )
        {
            iprintf( "Failed to connect to wifi interface\r\n" );
            return;
        }
    }

    /* The total length of SSID and the key must not exceed 68 characters.
    * It is stored as a concatenated null terminate string for example:
    * "SSID:NetBurner,WPA:XJrLy4GbZe1/3wORx5ja4bBC74284OqbhWwVa3+-"
    */
    char configSSID[ SYSTEM_CONFIG_RECORD_M_FILENAME_SIZE ];
    char configPassPhrase[ SYSTEM_CONFIG_RECORD_M_FILENAME_SIZE ];

    for( int i = 0; i < SYSTEM_CONFIG_RECORD_M_FILENAME_SIZE; i++ )
    {
        configSSID[ i ] = '\0';
        configPassPhrase[ i ] = '\0';
    }

    pNBWifiObject->GetSSIDFromConfig( configSSID, SYSTEM_CONFIG_RECORD_M_FILENAME_SIZE );
    pNBWifiObject->GetKeyFromConfig( configPassPhrase, SYSTEM_CONFIG_RECORD_M_FILENAME_SIZE );
    iprintf( "Configured SSID: \"%s\"\r\n", configSSID );
    iprintf( "Pass Phrase: \"%s\"\r\n", configPassPhrase );
}


/*-----------------------------------------------------------------------------
 * UserMain
 * - This example will provide a menu through the serial debug port
 *   that enables you to set/clear static IP settings, and start/stop
 *   the DHCP Client service.
 *----------------------------------------------------------------------------*/
void UserMain(void * pd)
{
    init();

#if (defined MCF5270 || defined MCF5234)
    iprintf("Spread Spectrum mode enabled\r\n");
    EnableSpreadSpectrum();
#endif

    // Scan for available networks and print the results on stdout
    WifiInitScanAndShow_SPI();

    // You can also get the list of results via WifiInitScan_SPI or WifiInitScan_Serial
    // This list will automatically deallocate when the next scan occurs
    // nbwifiScanResult *scanResults = WifiInitScan_SPI();

    // Start the Wifi interface and connect via it's SPI interface, with no SSID argument,
    // the driver will load the SSID and password from the config record, which can be set
    // using IPSetup.
    // int InitWifi_SPI(
    //            const char * SSID       = "",
    //            const char * password   = "",
    //            int irqNum              = 3,
    //            int moduleNum           = 1,
    //            int csNum               = NBWIFI_DEFAULT_CSNUM,
    //            int connectorNum        = NBWIFI_DEFAULT_CONNUM,
    //            int gpioPinNum          = NBWIFI_DEFAULT_PINNUM,
    //            int resetPinNum         = 42
    //            );
    int ifnumWifi = InitWifi_SPI();
    OSTimeDly(TICKS_PER_SECOND);

    // NBWifi is the Wifi driver namespace, and Master is the name of the class within the namespace.
    // We need to obtain a pointer to the already created Wifi object so we can call the object's
    // member functions.
    NB::Wifi *pNBWifiObject;
    pNBWifiObject = NB::Wifi::GetDriverByInterfaceNumber( ifnumWifi );

    bool connected = pNBWifiObject->Connected();

    if ( connected )
    {
        DisplayWifiStatus( pNBWifiObject );
        DisplayWifiConfigRecordParameters( pNBWifiObject );
    }
    else
    {
        iprintf("Not connected to Access Point, cannot display status\r\n");
        iprintf("\r\nConfigured Parameters: \r\n");
        DisplayWifiConfigRecordParameters( pNBWifiObject );
    }


    while (1)
    {
    	getchar();
    	DisplayWifiStatus(pNBWifiObject);
    }
}
