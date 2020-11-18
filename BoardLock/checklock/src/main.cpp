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

const char * AppName="checklock";

/*Your keblob should be :*/
uint32_t ctx_state[4]={2106921824u,3945495657u,2391356351u,2780313164u};
uint32_t ctx_count[2]={512u,0u};
unsigned char buf[64];

#ifdef SSL_TLS_SUPPORT
unsigned char inbuf[64];
unsigned char outbuf[64];
#endif

MD5_CTX YourCompanySecret(ctx_state,ctx_count,buf
#ifdef SSL_TLS_SUPPORT
 ,inbuf,outbuf
#endif
);


bool CheckBoardLock()
{
MD5_CTX TheContext=YourCompanySecret;
unsigned char digest[16];
MACADR ma=InterfaceMAC(GetFirstInterface());
MD5Update(&TheContext,(const uint8_t *) &ma,6);
MD5Final(digest,&TheContext);

/* We are getting the board lock value from  the userpaqram space.
If you want to store the 16 byte value some where else, feel free to do so */
unsigned char * checkvalue=(unsigned char *)GetUserParameters();

for(int i=0; i<16; i++) if(checkvalue[i]!=digest[i]) return false;
return true;
}


void UserMain(void * pd)
{
    init();                                       // Initialize network stack
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

    iprintf("Application started\n");
    if(CheckBoardLock())
    	iprintf("Board checks ok\r\n");
    else
    	iprintf("Board fails lock check\r\n");

    while (1)
    {
    	OSTimeDly(TICKS_PER_SECOND * 1);
    }
}
