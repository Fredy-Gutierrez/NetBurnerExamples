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

#include <init.h>
#include <stdio.h>
#include <json_lexer.h>             // For the server transactions
#include <webclient/web_client.h>
#include <config_obj.h>             // So we can create our own config objects
#include <ipshow.h>                 // Show diagnostic IP addresses
#include <math.h>                   // Is nan
#include <utils.h>                  // PutLeds and GetSwitches

// This config object holds the address of the web page we will be accessing
static config_string Url(appdata, "http://class.netburner.com/post/", "MessageUrl");
static uint8_t SwitchPositions;

const char * AppName = "MessagePasserTask";

const char * StudentName = "Attendee Name";

// Process errors, webclient/web_client.h has a list of the potential errors
void HandleErrorState(int state)
{
    if (state == WEB_CLIENT_ERROR_NO_ERROR)
        return;
    if ((state <= WEB_CLIENT_ERROR_LAST_STATE) && (state > 0))
        iprintf("Set Error state [%d] [%s]\r\n", state, web_error_state_text[state]);
    else
        iprintf("Error state unknown %d\r\n", state);
}

// Do the actual server request
bool DoActualClientRequest(ParsedURI &TheUri, uint16_t & next_time_delay)
{
    next_time_delay = 10 * TICKS_PER_SECOND;
    static char message[80];
    sniprintf(message, 80, "At the tone the time will be %ld", Secs);
    ParsedJsonDataSet JsonOutObject;
    ParsedJsonDataSet JsonInObject;

    JsonOutObject.StartBuilding();
    JsonOutObject.AddMyMac("DEVICEID");
    JsonOutObject.Add("UPTIME", Secs);
    JsonOutObject.Add("MESSAGE", message);
    JsonOutObject.Add("STUDENTNAME", StudentName);
    JsonOutObject.Add("SWITCH", (int) SwitchPositions);
    JsonOutObject.DoneBuilding();

//JsonOutObject.PrintObject(true);

    bool result = DoJsonPost(TheUri, JsonOutObject, JsonInObject, NULL, 10 * TICKS_PER_SECOND);

    if (result)
    {
        bool bSuccess = JsonInObject.FindFullNamePermissiveBoolean("SUCCESS");
        double CurSwitch = JsonInObject.FindFullNameNumber("CURSWITCH");
        if (isnan(CurSwitch))
        {
            putleds(0);
            iprintf("Failed to get CURSWITCH value\r\n");
        }
        else
        {
            putleds((uint8_t) CurSwitch);
        }

        if (bSuccess)
        {
            iprintf("The server acknowledged our post with  SUCCESSflag=true  CS=%g\r\n", CurSwitch);
        }
        else
        {
            iprintf("The server did not respond with a SUCCESS flag\r\n");
        }

        iprintf("Server sent us a message [%s]\r\n", JsonInObject.FindFullNameString("MESSAGE"));
        return true;
    }
    else
    {
        putleds(0);
        iprintf("Result failed\r\n");
    }
    return false;
}

void UserMain(void * pd)
{
    init();
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address
    showIpAddresses();

    //  SetHttpDiag(true);          // Enable web client diagnostics

    pWebErrorReporter = HandleErrorState;

    StartWebClient(MAIN_PRIO - 2, Url);

    uint8_t LastSw;

    LastSw = SwitchPositions = getdipsw();

    iprintf("Application started\n");
    while (1)
    {
        SwitchPositions = getdipsw();
        if (LastSw != SwitchPositions)
            WebClientSem.Post();
        LastSw = SwitchPositions;
        OSTimeDly(1);
    }
}

