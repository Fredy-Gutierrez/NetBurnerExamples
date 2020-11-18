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
#include <dhcpclient.h>
#include <netinterface.h>
#include <qspi.h>
#include <hal.h>
#include <serial.h>
#include <taskmon.h>
#include <sim.h>
#include <NetworkDebug.h>
#include <smarttrap.h>
#include <dhcpd.h>
#include <wifi/wifi.h>
#include <wifi/wifiBsp.h>

const char * AppName="NBWIFIIN Wifi Configuration AP";

/******************************************************************************
 * We can override the library defaults by creating and settings these        *
 * variables. They are provided as Weak References in the library, so are not *
 * required to created by the user.                                           *
 ******************************************************************************/
//const IPADDR4 configAP_LeaseStart(192, 168, 0, 2);
//const IPADDR4 configAP_IPAddr(192, 168, 0, 1);
//const NB::SSID_t default_SSID_Prefix = { 4, "FOO_" };

// NBWifi is the Wifi driver namespace, and Master is the name of the class within the namespace.
// We need to obtain a pointer to the already created Wifi object so we can call the object's
// member functions.
NB::Wifi *pNBWifiObject = NULL;
int ifnumWifi = 0;

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
    iprintf("Security: %s\r\n", valueLabel);
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
    iprintf("Cipher:  %s\r\n", valueLabel);
}



/*-----------------------------------------------------------------------------
 * Show scan results
 * You can get the list of results via WifiInitScan_SPI or WifiInitScan_Serial.
 * This list will automatically deallocate when the next scan occurs
 *----------------------------------------------------------------------------*/
void DoSyncScan()
{
    nbWifiScanResult *pScanResult;      // nbWifiScanResult structure is in wifiDriver.h

    iprintf("Starting scan.....");
    pScanResult = WifiInitScan_SPI();
    iprintf("complete\r\n");

    while ( pScanResult != NULL )
    {
        iprintf("BSSID: "); ShowMac( &pScanResult->bssid );
        iprintf("  SSID: \"%s\"\r\n", pScanResult->ssid );
        pScanResult = pScanResult->next;
    }
}


/*-----------------------------------------------------------------------------
 * Display Wifi Status Information
 *----------------------------------------------------------------------------*/
void DisplayWifiStatus( NB::Wifi *pNBWifiObject )
{
    const int bufSize = 80;

    // This example is for the development kit, which has an Ethernet interface
    // as its first interface. This information is here for completeness and to
    // show the difference in settings between Ethernet and Wifi.
    iprintf("\r\nETHERNET RUNTIME IP SETTINGS\r\n");
    int ifNumber = GetFirstInterface();
    iprintf("Interface Number: %d\r\n", ifNumber);
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

    iprintf("Connection Status: ");
    if ( pNBWifiObject->Connected() )
    {
        iprintf("Connected\r\n");

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
    }
    else
    {
        iprintf("Not Connected\r\n");
    }

    // To access the Wifi device information structure we need to call the GetDeviceInformation() function
    // of the Wifi object, so we use the pointer to the object: pNBWifiObject. The NB:: is needed
    // because the Wifi driver source code uses a C++ namespace called NB. The GetDeviceInformation()
    // function uses a system buffer pointed to by a PoolPtr to store the results. This buffer must be
    // freed by your application when you are done using it by calling FreeBuffer();
    NB::nbWifiDeviceInfo devInfo;
    pNBWifiObject->GetDeviceInformation( &devInfo );
    iprintf("Hardware Rev: %d.%d\r\n", devInfo.hardwareMajorRev, devInfo.hardwareMinorRev);
    iprintf("Wifi Firmware Rev: %d.%d\r\n", devInfo.softwareMajorRev, devInfo.softwareMinorRev );

    /*
    PoolPtr infoBuf = pNBWifiObject->GetDeviceInformation();
    NB::nbWifiDeviceInfo *devInfo = (NB::nbWifiDeviceInfo *)infoBuf->pData;
    iprintf("Hardware Rev: %d.%d\r\n", devInfo->hardwareMajorRev, devInfo->hardwareMinorRev);
    iprintf("Wifi Firmware Rev: %d.%d\r\n", devInfo->softwareMajorRev, devInfo->softwareMinorRev );
    FreeBuffer(infoBuf);
    */
}

void DisplayWifiConfigRecordParameters()
{
    // Make sure that we were able to initiate the wifi object
    if( pNBWifiObject == nullptr )
    {
        // Try to get the wifi interface if we don't already have it
        if( ifnumWifi == 0 )
        {
            ifnumWifi = InitWifi_SPI();
        }

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

    char configSSID[ SSID_MAX_LEN + 1 ];
    char configPassPhrase[ PASS_MAX_LEN + 1 ];

    memset(configSSID, 0, SSID_MAX_LEN + 1);
    memset(configPassPhrase, 0, PASS_MAX_LEN + 1);

    pNBWifiObject->GetSSIDFromConfig( configSSID, SSID_MAX_LEN );
    pNBWifiObject->GetKeyFromConfig( configPassPhrase, PASS_MAX_LEN );
    iprintf( "Configured SSID: \"%s\"\r\n", configSSID );
    iprintf( "Pass Phrase: \"%s\"\r\n", configPassPhrase );
}

void EraseWifiSSID()
{
    if (pNBWifiObject) {
        pNBWifiObject->StoreSSIDPWToConfig("", NULL);
    }
}

/*-----------------------------------------------------------------------------
 *
 *----------------------------------------------------------------------------*/
void DisplayMenu()
    {
        iprintf("\r\n");
        iprintf("C = Connect\r\n");
        iprintf("D = Disconnect\r\n");
        iprintf("R = Reset Stored Wifi SSID\r\n");
        iprintf("S = Scan\r\n");
        iprintf("W = Wifi Status\r\n");
        iprintf("X = Reset Wifi Module\r\n");
        iprintf("Z = System Reboot\r\n");
}


/*-----------------------------------------------------------------------------
 *
 *----------------------------------------------------------------------------*/
void ProcessCommand( char cmd )
{
    switch ( toupper(cmd))
        {
        case 'C':
                if ( pNBWifiObject != NULL)
                {
                    if (pNBWifiObject->Connected())
                    {
                        iprintf("Already Connected\r\n");
                        break;
                    }
                }

                iprintf("Connecting......\r\n");
                ifnumWifi = InitWifi_SPI();
                if (ifnumWifi > 0)
                {
                    pNBWifiObject = NB::Wifi::GetDriverByInterfaceNumber( ifnumWifi );
                    if (pNBWifiObject->Connected())
                    {
                        iprintf("Connected with interface number: %d\r\n", ifnumWifi);
                    }
                    else
                    {
                        iprintf("Failed to connect\r\n");
                    }
                }
                else
                {
                    iprintf("Failed to initialize wifi, interface = %d\r\n", ifnumWifi);
                }
                break;

        case 'D':
                if (pNBWifiObject->Connected())
                {
                    iprintf("Disconnecting.....");
                    pNBWifiObject->Disconnect();
                    OSTimeDly(TICKS_PER_SECOND * 2);
                    if (! pNBWifiObject->Connected())
                        iprintf("Success\r\n");
                    else
                        iprintf("Failed\r\n");
                }
                break;
        case 'R':
            EraseWifiSSID();
            break;

        case 'S':
        DoSyncScan();
            break;

        case 'W':
            if (pNBWifiObject != NULL)
                DisplayWifiStatus( pNBWifiObject );
            DisplayWifiConfigRecordParameters( );
            break;

    case 'X':
        NB::WifiBSP::WifiReset();
        break;

        case 'Z':
            iprintf("Rebooting System\r\n");
            OSTimeDly(TICKS_PER_SECOND);
            ForceReboot();
            break;

    default:
        DisplayMenu();
    }
}




/*-----------------------------------------------------------------------------
 * UserMain
 *----------------------------------------------------------------------------*/
void UserMain(void * pd)
{
    init();                                       // Initialize network stack
    StartHttp();                                  // Start web server, default port 80

    // IMPORTANT: Subsequent calls for scans and connections will rely on the interface number and
    // object initialization in these calls.
    iprintf("Initializing Wifi and scanning....\r\n");
    ifnumWifi = WifiInitScanAndShow_SPI();
    if (ifnumWifi > 0)
    {
        pNBWifiObject = NB::Wifi::GetDriverByInterfaceNumber( ifnumWifi );
    }
    else
    {
        iprintf("Failed to initialize wifi interface\r\n");
    }
    {
        char testSSID[SSID_MAX_LEN + 1];
        if (pNBWifiObject) {
            if (!pNBWifiObject->GetSSIDFromConfig(testSSID, SSID_MAX_LEN)) {
                int ret = pNBWifiObject->StartConfigAP();
                if (ret) {
                    iprintf("Failed to start config AP: %d\n", ret);
                }
                else {
                    iprintf("Config AP started\n");
                }
            }
            else {
                // run the connect command
                ProcessCommand( 'c' );
            }
        }

    }
    // ifnumWifi = InitWifi_Serial();
    // You can also run over the serial port, albeit much slower
    // ifnumWifi = InitWifi_Serial( "MyExampleSSID", "InsertPasswordHere" );
    // ifnumWifi = InitWifi_SPI();

    DisplayMenu();
    while (1)
    {
        char c = getchar();
        ProcessCommand( c );
    }
}
