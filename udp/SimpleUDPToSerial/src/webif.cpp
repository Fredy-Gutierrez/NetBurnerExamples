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

/**
 *  @file webif.cpp
 *  @brief This module handles the web page interface to the UDP to Serial
 *  program example.
 **/

#include <basictypes.h>
#include <http.h>
#include <iosys.h>
#include <stdlib.h>
#include <string.h>
#include <system.h>
#include <httppost.h>
#include "webif.h"

//----- function prototypes -----
extern "C"
{
    void WebLocalPort( int sock, PCSTR url );
    void WebDestPort( int sock, PCSTR url );
    void WebDestIp( int sock, PCSTR url );
}

//----- Global Vars -----
char gPostBuf[ POST_BUFSIZE ];
NV_SettingsStruct gNvSettings;
bool gChangedLocalPort;
bool gChangedDest;
bool gWriteToFlash;

/**
 *  Check NV Settings. This function will check the flash memory user
 *  parameter storage area for valid stored values. If the values are
 *  invalid (VerifyKey), it will assign default values.
 **/
void CheckNVSettings()
{
    NV_SettingsStruct *pData = (NV_SettingsStruct *)GetUserParameters();
    gNvSettings.VerifyKey = pData->VerifyKey;

    if( gNvSettings.VerifyKey != VERIFY_KEY )
    {
        iprintf( "Resetting flash memory parameters to default\r\n" );
        gNvSettings.VerifyKey = VERIFY_KEY;
        gNvSettings.nLocalPort = LOCAL_PORT;
        gNvSettings.nDestPort = DEST_PORT;
        strncpy( gNvSettings.szDestIpAddr, DEST_IPADDR, MAX_IPADDR_LEN );

        SaveUserParameters( &gNvSettings, sizeof( gNvSettings ) );
    }
    else
    {
        iprintf( "Read stored parameters from flash memory\r\n" );
        gNvSettings.nDestPort = pData->nDestPort;
        gNvSettings.nLocalPort = pData->nLocalPort;
        strncpy( gNvSettings.szDestIpAddr, pData->szDestIpAddr, MAX_IPADDR_LEN );
    }
}


/**
 *  Function to display destination port number
 **/
void WebLocalPort( int sock, PCSTR url )
{
    char buf[ 80 ];

    sniprintf( buf, 80, "VALUE=\"%d\" ", gNvSettings.nLocalPort );
    writestring( sock, buf );
}


/**
 * Function to display destination port number
 **/
void WebDestPort( int sock, PCSTR url )
{
    char buf[ 80 ];

    sniprintf( buf, 80, "VALUE=\"%d\" ", gNvSettings.nDestPort );
    writestring( sock, buf );
}


/**
 *  Function to display current destination IP address
 */
void WebDestIp( int sock, PCSTR url )
{
    char buf[ 80 ];

    sniprintf( buf, 80, "VALUE=\"%s\" ", gNvSettings.szDestIpAddr );
    writestring( sock, buf );
}




void processPostVariables( const char *pName, const char *pValue )
{
    iprintf("Processing: %s\r\n", pName);

    if( strcmp(pName, "tfLocalPort") == 0 )
    {
        gNvSettings.nLocalPort = (uint16_t)atoi( pValue );
        iprintf( "Local port set to: %d\r\n", gNvSettings.nLocalPort );
        gChangedLocalPort = true;
        gWriteToFlash = true;
    }
    else if( strcmp( pName, "tfDestPort") == 0 )
    {
        gNvSettings.nDestPort = (uint16_t)atoi( pValue );
        iprintf( "Dest port set to: %d\r\n", gNvSettings.nDestPort );
        gChangedDest = true;
        gWriteToFlash = true;
    }
    else if( strcmp( pName, "tfDestIpAddr") == 0 )
    {
        iprintf( "DestIpAddr set to: %s\r\n", pValue );
        strncpy( gNvSettings.szDestIpAddr, pValue, MAX_IPADDR_LEN );
        gChangedDest = true;
        gWriteToFlash = true;
    }
    else
    {
        iprintf("Error processing %s\r\n", pName);
    }
}


void PostCallBack(int sock, PostEvents event, const char * pName, const char * pValue)
{
    // Received a call back with an event, check for event type
    switch (event)
    {
    case eStartingPost:     // Called at the beginning of the post before any data is sent
        gWriteToFlash = false;
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
            if ( gWriteToFlash )
            {
                // Save new parameters in flash memory. WARNING: If new settings are added,
                // remember to add them to NV Settings default initialization.
                gNvSettings.VerifyKey = VERIFY_KEY;
                if( SaveUserParameters( &gNvSettings, sizeof( gNvSettings ) ) != 0 )
                {
                    iprintf( "New Settings Saved\r\n" );
                }
                else
                {
                    iprintf( "ERROR: Could not save new settings\r\n" );
                }
            }

            RedirectResponse(sock, "INDEX.HTML");  // Our response is to redirect to the index page
        }
        break;

    } //Switch
}


// Create a global static post handeling object that responds to the specified html page.
// A separate post handler can be created for each form in your application.
HtmlPostVariableListCallback poster("index.html",PostCallBack);





