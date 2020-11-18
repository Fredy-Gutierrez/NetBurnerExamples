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
#include <smarttrap.h>
#include <taskmon.h>
#include <dhcpd.h>
#include <wifi/wifi.h>

extern "C" {
void UserMain(void * pd);
}


// NB is the NetBurner namespace, and Wifi is the name of the class within the namespace.
// We need to obtain a pointer to the already created Wifi object so we can call the object's
// member functions.
NB::Wifi *pNBWifiObject = nullptr;
int ifnumWifi = 0;

extern DHCP::Server MyServer;
void PrintDhcpClients( DHCP::Server &server );

const char * AppName="wifiAP";

#if (defined MCF5270 || defined MCF5234)
#include <sim.h>
/**************************************************************/
// The Following section of code turns on the spread spectrum
//   for the PLL and also reduces pin drive strength
// This should be used to reduce EMI emissions
/**************************************************************/
   #define FM_RATE (0x1000)   // fref/40
   //#define FM_RATE (0)         // fref/80

   //#define FM_DEPTH   (0)         // FM disabled
   #define FM_DEPTH   (0x400)      // 1% of fsys\2
   //#define FM_DEPTH   (0x800)      // 2% of fsys\2

   #define FM_EXP (128)

   #define SYNSR_LOCK_BIT 0x00000008

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
      printf("Frequency Modulation of Clock Successfully Calibrated!\r\n");
   else
      printf("Frequency Modulation of Clock FAILED Calibration!\r\n");
}
#endif


void UserMain(void * pd)
{
    init();


#if (defined MCF5270 || defined MCF5234)
    EnableSpreadSpectrum();
#endif


    // Start the wifi device in Access Point mode. This allows another device to connect
    // directly to this module. If you do not pass an SSID and password as function parameters,
    // the SSID/password from the config record will be used. These can be set using IPSetup.
    // The full call is:
    // int InitAP_SPI(
    //          const char * SSID       = "",
    //          const char * password   = "", /* Password must be between 8 and 64 chars */
    //          uint8_t      channel    = 6, /* 802.11 channel to setup the AP on */
    //          int irqNum              = 3,
    //          int moduleNum           = 1,
    //          int csNum               = NBWIFI_DEFAULT_CSNUM,
    //          int connectorNum        = NBWIFI_DEFAULT_CONNUM,
    //          int gpioPinNum          = NBWIFI_DEFAULT_PINNUM,
    //          int resetPinNum         = 42
    //          );
    //
    int rv = InitAP_SPI("NetBurnerAP", "password");
    iprintf("InitAP_SPI returned: %d\r\n", rv );

    /* DHCP server will assign IP addresses to devices connected to the AP starting with this address */
    //IPADDR4 startAddr(192, 168, 0, 2);	// works in tools 2.8.5 and above
    IPADDR4 startAddr = AsciiToIp4("192.168.0.2");

    // Start DHCPD Server on the Wifi interface
    iprintf("Starting Access Point DHCP server... ");
    if (AddStandardDHCPServer(theWifiIntf->GetSystemInterfaceNumber(), startAddr))
        iprintf("Success\r\n");
    else
        iprintf("Error: another server exists\r\n");

    iprintf("Any key to display clients\r\n");

    while (1)
    {
    	getchar();
        PrintDhcpClients( MyServer );
    }
}
