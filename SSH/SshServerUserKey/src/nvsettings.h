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


#ifndef NVSETTINGS_H_
#define NVSETTINGS_H_

#include <basictypes.h>
#include "nbfactory.h"


// Configuration verify key (increment if data changed, added, reorganized)
#define NB_FACTORY_VERIFY_KEY    ( 0x5e545064 )
#define NTP_NAME_LENGTH          ( 35 )
#define DEVICE_NAME_LENGTH       ( 15 )


struct NV_SettingsStruct
{
   char DeviceName[ ( DEVICE_NAME_LENGTH + 1 ) ];
   char NTPName[ NTP_NAME_LENGTH+ 1 ];
   //IPADDR NTP_Addr;

   /* SSH key source and lengths (default and user installed) */
   uint8_t SshKeyRsaSource;   // Library default, app default, or user installed
   uint16_t SshKeyRsaLength;
   uint8_t SshKeyDsaSource;   // Library default, app default, or user installed
   uint16_t SshKeyDsaLength;

   /* Version verification key */
   uint32_t VerifyKey;
};


extern void CheckNVSettings( BOOL returnToFactory );


#endif /* NVSETTINGS_H_ */




