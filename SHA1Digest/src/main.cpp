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

/****************************************************************************************
 * Calculate the SHA1 128-bit digest of the application and display it through the serial port.
 *
 * To calculate and display the digest of the application when building the application,
 * go to the project properties and add the -D option to the compcode flags.
 ****************************************************************************************/
#include <predef.h>
#include <stdio.h>
#include <ctype.h>
#include <init.h>
#include <nbrtos.h>
#include <sha1.h>

const char * AppName = "SHA1 App Digest";

// Application information structure. Precedes the application in memory.
typedef struct
{
      unsigned long dwBlockRamStart ;
      unsigned long dwExecutionAddr ;
      unsigned long dwBlockSize     ;
      unsigned long dwSrcBlockSize  ;
      unsigned long dwBlockSum      ;  
      unsigned long dwStructSum     ;
}StartUpStruct ;


extern uint32_t FlashAppBase;      // Location of the start of the application and startup struct

// Calculate the SHA1 digest of the application
void CalcCodeSha1(unsigned char * digest)
{
    SHA1_CTX ctx;
    int len = 0;
    unsigned char * pData;

    StartUpStruct * pSS = (StartUpStruct *) &FlashAppBase;
    len = sizeof(StartUpStruct) + pSS->dwSrcBlockSize;

    pData = (unsigned char *) (&FlashAppBase);

    SHA1Init(&ctx);
    SHA1Update(&ctx, pData, len);
    SHA1Final(digest, &ctx);
}


void UserMain(void * pd)
{
   init();
   iprintf("Application started\n");

   unsigned char Digest[20];
   CalcCodeSha1(Digest);
   iprintf("SHA1 Code Result = ");
   for(int i = 0; i < 5; i++)
   {
       iprintf("%08lX ",((uint32_t *)Digest)[i]);
   }
   iprintf("\r\n");

   while (1)
  {
      OSTimeDly(TICKS_PER_SECOND * 1);
  }
}
