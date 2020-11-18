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


#include "nvsettings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iosys.h>
#include <ctype.h>
#include <system.h>
#include <utils.h>
#include <netinterface.h>
#include "sshuser.h"

// Allocate non-volatile user settings runtime storage. This structure
// will be written to Flash for non-volatile storage.
NV_SettingsStruct NV_Settings;


/*-------------------------------------------------------------------
 * Verify user parameters, if invalid reset to defaults
 *------------------------------------------------------------------*/
void CheckNVSettings( BOOL returnToFactory )
{
   NV_SettingsStruct* pData = (NV_SettingsStruct*)GetUserParameters();
   NV_Settings = *pData;

   if ( ( NV_Settings.VerifyKey != NB_FACTORY_VERIFY_KEY ) || ( returnToFactory == TRUE ) )
   {
      // Set device name
	  int ifNumber = GetFirstInterface();
	  MACADR MacAddr = InterfaceMAC(ifNumber);
	  puint8_t lpb = (puint8_t)&MacAddr;
      sniprintf( NV_Settings.DeviceName, DEVICE_NAME_LENGTH, "%s-%02X%02X", NB_FACTORY_MODULE_BASE_NAME,
         (int)lpb[4], (int)lpb[5] );

      (void)strcpy( (char*)NV_Settings.NTPName, "pool.ntp.org" );

      SshUserSetDefault();

      NV_Settings.VerifyKey = NB_FACTORY_VERIFY_KEY;

      SaveUserParameters( &NV_Settings, sizeof(NV_Settings) );

      iprintf("Setting up new default storage info\r\n");
   }

   /* SSH Keys */
   SshUserRetrieveKeys();

   return;
}

