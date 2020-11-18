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
 * This code module handles the web page form input to change the
 * NETBIOS name.
 *-------------------------------------------------------------------*/
#include <predef.h>
#include <basictypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <constants.h>
#include <string.h>
#include <nbrtos.h>
#include <http.h>
#include <system.h>

extern "C"
{
   void webNetBiosName( int sock, PCSTR url );
}

extern const char *deviceName;

/*-------------------------------------------------------------------
 * FUNCTIONCALL function to display the "name" structure member
 * on the web page.
 *-----------------------------------------------------------------*/
void webNetBiosName( int sock, PCSTR url )
{
      writesafestring( sock, deviceName );  //pData->name );
}

