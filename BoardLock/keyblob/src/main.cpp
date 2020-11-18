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
#include <md5.h>
#include <netinterface.h>


const char * AppName="keyblob";


const char * YourSecretSigningText ="This should be your company  secret message";

static MD5_CTX TheContext;


void UserMain(void * pd)
{
   init();                                       // Initialize network stack
   WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

   iprintf("Sign board Application started\n");

   MD5Init(&TheContext);
   int len=strlen(YourSecretSigningText);
   const unsigned char * cp=(const unsigned char *)YourSecretSigningText;
   unsigned char tBuffer[64];

   while (len>64)
       {
        MD5Update(&TheContext,cp,64);
        cp+=64;
        len-=64;
       }
   for(int i=0; i<64; i++)
   {
    if(len) {tBuffer[i]=*cp++; len--;}
    else
        tBuffer[i]=i;
   }

   MD5Update(&TheContext,tBuffer,64);

   iprintf("/*Your keblob should be :*/\r\n");

   iprintf("uint32_t ctx_state[4]={%luu,%luu,%luu,%luu};\r\n",TheContext.state[0],TheContext.state[1],TheContext.state[2],TheContext.state[3]);
   iprintf("uint32_t ctx_count[2]={%luu,%luu};\r\n",TheContext.count[0],TheContext.count[1]);

iprintf("unsigned char buf[64];\r\n");
iprintf("#ifdef SSL_TLS_SUPPORT\r\n");
iprintf("unsigned char inbuf[64];\r\n");
iprintf("unsigned char outbuf[64];\r\n");
iprintf("#endif\r\n");
iprintf("MD5_CTX YourCompanySecret(ctx_state,ctx_count,buf\r\n");
iprintf("#ifdef SSL_TLS_SUPPORT\r\n");
iprintf(" ,inbuf,outbuf\r\n");
iprintf("#endif\r\n");
iprintf(");\r\n");


   while (1)
  {
      OSTimeDly(TICKS_PER_SECOND * 1);
  }
}
