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

#include <http.h>
#include <httppass.h>   // Needed for HTML passwords
#include <init.h>
#include <nbrtos.h>
#include <string.h>
#include <system.h>

const char *AppName = "HTML Simple Password Example";

HTTP_ACCESS CheckHttpAccess(int sock, int access_level, HTTP_Request & Req)
{
    /* access level is set in the webpage. */
    if (access_level == 0) return HTTP_OK_TO_SERVE;


    char *pPass;
    char *pUser;


    if (Req.ExtractAuthentication(&pPass, &pUser))
    {
        iprintf("Testing User[%s] and Password[%s]\r\m", pUser, pPass);
        //Return ok as long are  equal
        if (strcmp(pPass, pUser) != 0) return HTTP_OK_TO_SERVE;
    }

    return HTTP_NEED_PASSWORD;

}



void UserMain(void *pd)
{
    init();                                       // Initialize network stack
    StartHttp();                                  // Start web server, default port 80
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

    iprintf("Application: %s\r\nNNDK Revision: %s\r\n", AppName, GetReleaseTag());

    // Register a new GET handler

    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND);
    }
}
