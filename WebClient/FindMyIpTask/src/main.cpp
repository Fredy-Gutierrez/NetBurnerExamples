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
#include <ctype.h>
#include <buffers.h>
#include <json_lexer.h>
#include <webclient/http_funcs.h>
#include <webclient/web_client.h>


const char * url="http://httpbin.org/ip";



const char * AppName="FindMyIp";



void HandleErrorState(int state)
{
if(state==WEB_CLIENT_ERROR_NO_ERROR) return;
if ((state<=WEB_CLIENT_ERROR_LAST_STATE) &&( state>0)) iprintf("Set Error state [%d] [%s]\r\n",state,web_error_state_text[state]);
else
iprintf("Error state unknown %d\r\n",state);
}



bool DoActualClientRequest(ParsedURI &TheUri, uint16_t & next_time_delay)
{
ParsedJsonDataSet JsonResult;
bool result=DoGet(TheUri,JsonResult);
if(result)
{
    iprintf("My IP address is %s\r\n",JsonResult.FindGlobalString("origin"));
    next_time_delay=10*TICKS_PER_SECOND;
    return true;
}

    next_time_delay=10*TICKS_PER_SECOND;
    return false;
}



void UserMain(void * pd)
{


    init();                                       // Initialize network stack
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

   pWebErrorReporter=HandleErrorState;
   StartWebClient(MAIN_PRIO-2,url);


   iprintf("Application started\n");
   while (1)
  {
      OSTimeDly(TICKS_PER_SECOND * 1);
  }
}
