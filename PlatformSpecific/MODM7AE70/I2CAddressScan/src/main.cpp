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
#include <pins.h>
#include <smarttrap.h>
#include <nbrtos.h>
#include <i2c.h>


const char *AppName = "SAME70 I2C Address Scan";

#define I2C_MODULE_0        (0)          // use I2C0/TW0
#define I2C_BUS_SPEED       (100000)     // bus speed of 100kHz

int8_t InitI2CModule(uint8_t moduleNum)
{
    if( (moduleNum < 0) || (moduleNum > 2) ) 
    {
        iprintf("Error: Invalid module number\r\n");
        return -1;
    }

    switch(moduleNum)
    {
        case 0:
            P2[39].function(PINP2_39_TWD0);
            P2[42].function(PINP2_42_TWCK0);
            i2c[0].setup(I2C_BUS_SPEED);
            break;
        case 1:
            P2[22].function(PINP2_22_TWD1);
            P2[12].function(PINP2_12_TWCK1);
            i2c[1].setup(I2C_BUS_SPEED);
            break;
        case 2:
            P2[26].function(PINP2_26_TWD2);
            P2[23].function(PINP2_23_TWCK2);
            i2c[2].setup(I2C_BUS_SPEED);
            break;
        default:
            iprintf("invalid moduleNum\r\n");
            break;
    }

    return 0;
}

int8_t ScanI2CBus( uint8_t moduleNum )
{
    if( (moduleNum < 0) || (moduleNum > 2) )
    {
        iprintf("Error: Invalid module number\r\n");
        return -1;
    }

    uint8_t readData = 0;
    uint8_t ret = 0;
    uint32_t deviceCount = 0;

    iprintf("Scanning I2C bus for devices...\r\n");

    // Scan address range 0 through 127 for devices
    for(uint8_t addr = 0; addr < 128; addr++)
    {
        ret = i2c[moduleNum].readReg8(addr, 0x0 /* register */, readData);
        if(ret == I2C::I2C_RES_ACK)
        {
            // if we received an ACK, there is a device at this address
            iprintf("Found a device at address 0x%02x\r\n", addr);
            deviceCount++;
        }
    }
    iprintf("Done. Found a total of %d device(s) on the bus.\r\n", deviceCount);

    return 0;
}

void UserMain(void *pd)
{
    init();
    EnableSmartTraps();

    InitI2CModule( I2C_MODULE_0 );
    ScanI2CBus( I2C_MODULE_0 );

    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND * 1);
    }
}
