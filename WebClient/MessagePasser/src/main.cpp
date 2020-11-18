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
#include <netinterface.h>

const char * AppName = "MessagePasser";

//const char * Url ="http://66.27.60.212:9090/"
const char * Url ="http://burnedcloud.herokuapp.com/messagehandler";

void SendManualRxMessage(const char * msg)
{
    ParsedJsonDataSet JsonResult;

    static char JsonOut[256];
    MACADR ma = InterfaceMAC(GetFirstInterface());
    sniprintf(JsonOut, 256, "{\"DEVICEID\":\"%02X%02X%02X%02X%02X%02X\",\"UPTIME\":%ld,\"MESSAGE\":\"%s\"}"
             ,ma.GetByte(0)
             ,ma.GetByte(1)
             ,ma.GetByte(2)
             ,ma.GetByte(3)
             ,ma.GetByte(4)
             ,ma.GetByte(5)
             ,Secs
             ,msg);

    bool result = DoJsonPost(Url, JsonOut, JsonResult, NULL, 10 * TICKS_PER_SECOND);
    if (result)
    {
        iprintf("Success is %d\r\n", JsonResult.FindFullNameBoolean("SUCCESS"));
        iprintf("Message back is [%s]\r\n", JsonResult.FindFullNameString("MESSAGE"));
    }
    else
    {
        iprintf("Result failed\r\n");
    }
}




void SendRxMessage(const char * msg)
{
    ParsedJsonDataSet JsonOutObject;
    ParsedJsonDataSet JsonInObject;

    JsonOutObject.StartBuilding();
    JsonOutObject.AddMyMac("DEVICEID");
    JsonOutObject.Add("UPTIME", Secs);
    JsonOutObject.Add("MESSAGE", msg);
    JsonOutObject.DoneBuilding();

    //OutBoundJson.PrintObject(true);

    bool result = DoJsonPost(Url, JsonOutObject, JsonInObject, NULL, 10 * TICKS_PER_SECOND);

    if (result)
    {
        iprintf("Success is %d\r\n", JsonInObject.FindFullNameBoolean("SUCCESS"));
        iprintf("Message back is [%s]\r\n", JsonInObject.FindFullNameString("MESSAGE"));
    }
    else
    {
        iprintf("Result failed\r\n");
    }
}


void UserMain(void * pd)
{
    init();
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

    iprintf("Application started\n");
    while (1)
    {
        static char message[80];
        sniprintf(message, 80, "At the tone the time will be: %ld", Secs);

        SendRxMessage(message);
        OSTimeDly(TICKS_PER_SECOND * 10);
    }
}
