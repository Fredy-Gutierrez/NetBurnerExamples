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

/*-------------------------------------------------------------------
  This code module contains the web functions for the send mail
  example program.
 -------------------------------------------------------------------*/
#include <predef.h>
#include <stdio.h>
#include <stdlib.h>
#include <basictypes.h>
#include <system.h>
#include <ip.h>
#include <iosys.h>
#include <fdprintf.h>
#include <string.h>
#include <httppost.h>
#include "formtools.h"
#include "nvsettings.h"


//----- Function Prototypes -----
extern "C"
{
    void webServerListenPort( int sock, PCSTR url );
    void webClientTimeout( int sock, PCSTR url );
    void webClientOverrideTimeout( int sock, PCSTR url );
    void webBaudRate( int sock, PCSTR url );
    void webFlowControl( int sock, PCSTR url );
}

extern void InitializeSerialPorts();
extern void ConfigureSerialFlowControl();

bool bChangeServerListenPort;       // Set true if the listen port was changed on the web page
NV_SettingsStruct NV_Settings;      // Non-volatile settings to store in flash memory



void webServerListenPort( int sock, PCSTR url )
{
   fdprintf(sock, "%d", NV_Settings.ServerListenPort);
}

void webClientTimeout( int sock, PCSTR url )
{
    fdprintf(sock, "%d", NV_Settings.ClientTimeout );
}

void webClientOverrideTimeout( int sock, PCSTR url )
{
    fdprintf(sock, "%d", NV_Settings.ClientOverrideTimeout);
}


const char *listBaudRates[80] = {
        "115200", "57600", "38400", "19200", "9600", "4800", "2400", "1200", ""
};

void webBaudRate( int sock, PCSTR url )
{
   FormOutputSelect( sock, "BaudRate", 1, listBaudRates );
}


const char *listFlowControl[80] = {
        "None", "Software", ""
};

void webFlowControl( int sock, PCSTR url )
{
   FormOutputSelect( sock, "FlowControl", 1, listFlowControl );
}



/*-----------------------------------------------------------------------------------
 * Process for post variables
 *-----------------------------------------------------------------------------------*/
void processPostVariables( const char *pName, const char *pValue )
{
    //iprintf("Processing: %s\r\n", pName);

    if( strcmp(pName, "ServerListenPort") == 0 )
    {
        if (NV_Settings.ServerListenPort != (uint16_t)atoi(pValue))
        {
            bChangeServerListenPort = TRUE;
            iprintf("Changing Server Listen Port\r\n");
        }
        NV_Settings.ServerListenPort = (uint16_t)atoi(pValue);
    }
    else if( strcmp( pName, "ClientTimeout") == 0 )
    {
        NV_Settings.ClientTimeout = (uint16_t)atoi( pValue );
    }
    else if( strcmp( pName, "ClientOverrideTimeout") == 0 )
    {
        NV_Settings.ClientTimeout = (uint16_t)atoi( pValue );
    }
    else if( strcmp( pName, "BaudRate") == 0 )
    {
        // Check if current baud rate is different than new one
        if (NV_Settings.DataBaudRate != (uint32_t)atol(pValue))
        {
            NV_Settings.DataBaudRate = (uint32_t)atol(pValue);
            InitializeSerialPorts();
        }
    }
    else if( strcmp( pName, "SerialDataFlowControl") == 0 )
    {
        // Check if current value is different than new one
        if (NV_Settings.SerialDataFlowControl != (uint16_t)atoi(pValue))
        {
            NV_Settings.SerialDataFlowControl = (uint16_t)atoi(pValue);
            ConfigureSerialFlowControl();
        }
    }
}

/*-----------------------------------------------------------------------------------
 * Form post callback fuction
 *-----------------------------------------------------------------------------------*/
void PostCallBack(int sock, PostEvents event, const char * pName, const char * pValue)
{
    // Received a call back with an event, check for event type
    switch (event)
    {
    case eStartingPost:     // Called at the beginning of the post before any data is sent
        break;

    case eVariable:     // Called once for each variable in the form
        processPostVariables(pName, pValue);
        break;

    //Called back with a file handle if the post had a file
    case eFile:
        break; //No file type here so we do nothing

    // Called back when the post is complete. You should send your response here.
    case eEndOfPost:
        {
            // Display new data for debugging
            iprintf("New User Settings:\r\n");
            iprintf("Server Port: %d\r\n", NV_Settings.ServerListenPort);
            iprintf("Client Timeout: %d\r\n", NV_Settings.ClientTimeout);
            iprintf("Client Override Timeout: %d\r\n", NV_Settings.ClientOverrideTimeout);
            iprintf("Data Baud Rate: %ld\r\n", NV_Settings.DataBaudRate);
            iprintf("Serial Data Flow Control: %d\r\n", NV_Settings.SerialDataFlowControl);

            // Now to store it in flash. WARNING: If new settings are added, remember to add them to
            // NV Settings default initialization
            NV_Settings.VerifyKey = VERIFY_KEY;
            if( SaveUserParameters( &NV_Settings, sizeof( NV_Settings ) ) != 0 )
            {
                iprintf( "New Settings Saved\r\n" );
            }
            else
            {
                iprintf("ERROR: Could not save new settings\r\n");
            }

            RedirectResponse(sock, "INDEX.HTML");  // Our response is to redirect to the index page
        }
        break;

    } //Switch
}


// Create a global static post handeling object that responds to the specified html page.
// A separate post handler can be created for each form in your application.
HtmlPostVariableListCallback poster("index.html",PostCallBack);







