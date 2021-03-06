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

const char * url="http://earthquake.usgs.gov/earthquakes/feed/v1.0/summary/4.5_day.geojson";



const char * AppName="EarthQuake";



void UserMain(void * pd)
{

     init();                                       // Initialize network stack
     WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

   iprintf("Application started\n");

   ParsedJsonDataSet JsonResult;
   bool result=DoGet(url,JsonResult);
   if(result)
   {
    printf("In the last Day we have had %g Earthquakes > 4.5\r\n",JsonResult.FindFullNameNumber("metadata.count"));
    while(JsonResult.FindElementAfterName("title")==STRING)
    {
        iprintf("%s\r\n",JsonResult.CurrentString());
    }
   JsonResult.PrintObject(true);
   }
   else
   {
    iprintf("Failed to contact server\r\n");
   }

   while (1)
  {
      OSTimeDly(TICKS_PER_SECOND * 1);
  }
}
