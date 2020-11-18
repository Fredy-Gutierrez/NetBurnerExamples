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

//* NB Library Definitions */
#include <http.h>
#include <httppost.h>
#include <netbios.h>

/* Product Definitions */
#include "nbfactory.h"

/* Ethernet to Serial Application Definitions */
#include "serialburnerdata.h"

#ifdef DIAG_LOG
#include "log.h"
#endif /* #ifdef DIAG_LOG */

/* Close down system before updating configuration */
extern void CloseEverything(void);

/* Verify entered password with user parameter settings password */
extern int TestPassword(const char *name, const char *passwd);

/* certificatekey.cpp routines */
extern void CaCertPost(int sock, PostEvents event, const char *pName, const char *pValue);
extern void HttpsPost(int sock, PostEvents event, const char *pName, const char *pValue);
extern void CaDelPost(int sock, PostEvents event, const char *pName, const char *pValue);

// Register a call back for the web page...
HtmlPostVariableListCallback gHttpsPostCallback("HTTPSPOST.HTML", HttpsPost);
HtmlPostVariableListCallback gCaCertPostCallback("CACERTPOST.HTML", CaCertPost);
HtmlPostVariableListCallback gCaDelPostCallback("CADELPOST.HTML", CaDelPost);

/**
 * @brief Checks for changes with user parameters.
 */
void CheckForChanges()
{
    // Check for changes
    if (gChangedUserParameters == true)
    {
        OSTimeDly(1);

        // Save changes
        if (gChangedUserParameters == true)
        {
            debug_iprintf("Saving User Parameters, size = %ld\r\n", sizeof(NV_Settings));
            SaveUserParameters(&gNV_SettingsChangeCopy, sizeof(NV_Settings));
            NV_SettingsStruct *NV_SettingsStructPtr = (NV_SettingsStruct *)GetUserParameters();
            NV_Settings = *NV_SettingsStructPtr;
            gChangedUserParameters = false;
        }
    }
}
