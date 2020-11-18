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

#include <predef.h>
#include <stdio.h>
#include <init.h>
#include <sim.h>           /*on-chip register definitions*/
#include <pins.h>
#include <nbrtos.h>
#include "SimpleAD.h"

extern "C"
{
   void UserMain( void *pd );
};

extern unsigned long CPU_CLOCK;

const char *AppName = "Simple SAME70 A/D Example";

struct ch_t {
    uint8_t module;
    uint8_t ch;
};


#define ADC_CH_COUNT (8)
ch_t ch[ADC_CH_COUNT] = {
    {0, 0}, {0, 1}, {0, 2}, {0, 5}, {0, 6}, {0, 10}, {0, 11}, {1, 3}
};

void UserMain(void *pd)
{
    init();
    InitSingleEndAD();

    for (int i = 0; i < ADC_CH_COUNT; i++)
        ADConfigCh(ch[i].module, ch[i].ch, true);
    while (1)
    {
        StartAD();
        while (!ADDone())
            asm("nop");
        for (int i = 0; i < ADC_CH_COUNT; i++)
            printf("%#03x,", GetADResult(ch[i].module, ch[i].ch)); //Get the AD Result
        printf("\r\n");
        for (int i = 0; i < ADC_CH_COUNT; i++)
            printf("%6.4f,", ((double) GetADResult(ch[i].module, ch[i].ch)) * 3.3 / (4096.0));

        printf("\r\n\r\n");
    }
}
