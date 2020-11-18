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
#include <constants.h>
#include <bsp.h>
#include <cpu_hal.h>
#include <ebi.h>
#include <nbrtos.h>
#include "hd44780.h"

#define DLY(x) { for (volatile int i = 0; i < (x); i++) {} }

HD44780_LCD_Bus::HD44780_LCD_Bus()
{
    if ((((uint32_t)this) < 0x60000000) || (((uint32_t)this) >= 0x64000000))
    {
        return;
    }

    // Get the base address of the LCD buffer
    uint8_t csNum = (((uint32_t)(this)) & (0x3 << 28)) >> 28;

    // nCS_1 is the same as SDRAM CS, which the system already uses...
    if (csNum == 1) { return; }

    EnablePeriphClock(ID_SMC);

    SMC->SMC_CS_NUMBER[csNum].SMC_SETUP = SMC_SETUP_NCS_RD_SETUP(25) | SMC_SETUP_NRD_SETUP(15)
        | SMC_SETUP_NCS_WR_SETUP(25) | SMC_SETUP_NWE_SETUP(15);

    SMC->SMC_CS_NUMBER[csNum].SMC_PULSE = SMC_PULSE_NCS_RD_PULSE(30) | SMC_PULSE_NRD_PULSE(65)
        | SMC_PULSE_NCS_WR_PULSE(30) | SMC_PULSE_NWE_PULSE(65);
    SMC->SMC_CS_NUMBER[csNum].SMC_CYCLE = SMC_CYCLE_NRD_CYCLE(90) | SMC_CYCLE_NWE_CYCLE(90);

    SMC->SMC_CS_NUMBER[csNum].SMC_MODE = SMC_MODE_DBW_8_BIT;;

    ConfigureEBI_CSPin(csNum);
    ConfigureEBI_NWRPin();
    ConfigureEBI_NRDPin();
}

void HD44780_LCD_Bus::init(uint8_t cols, uint8_t rows)
{
    ExtBusEnableCtx busEnable;
    bus[0]=0x30;
    DLY(30000);
    bus[0]=0x30;
    DLY(30000);
    bus[0]=0x30;
    DLY(30000);

    bus[0]=0x30 | ((rows > 0) ? 0x8 : 0x0);
    DLY(30000);

    bus[0]=0x80; DLY(3000);
    bus[0]=0x0C; DLY(3000);
    bus[0]=0x06; DLY(3000);

    for (uint8_t row = 0; row < rows; row++) {
        setCursor(row, 0);
        for (uint8_t col = 0; col < cols; cols++) {
            this->putchar(' ');
        }
    }
}

void HD44780_LCD_Bus::putchar(char c)
{
    ExtBusEnableCtx busEnable;
    bus[1]=(uint8_t)c; DLY(3000)
}

void HD44780_LCD_Bus::putstr(const char * str)
{
    ExtBusEnableCtx busEnable;
    char c;
    while ((c = *str++)) {
        this->putchar(c);
    }
}

void HD44780_LCD_Bus::clr()
{
    ExtBusEnableCtx busEnable;
    bus[0]=0x01; DLY(30000);
}

void HD44780_LCD_Bus::setCursor(int row, int col)
{
    ExtBusEnableCtx busEnable;
    reg(0, bus) = 0x80 | ((row & 0x1)*0x40 + (col & 0x27));
    DLY(2000)
}

HD44780_LCD::HD44780_LCD(HD44780_LCD_Bus &bus, uint8_t cols, uint8_t rows)
    : bus(bus), cols(cols), rows(rows), cursorDisp(CURSOR_OFF),
        dispState(true)
{}

void HD44780_LCD::clr()
{
    ExtBusEnableCtx busEnable;
    for (uint8_t row = 0; row < rows; row++) {
        setCursor(row, 0);
        for (uint8_t col = 0; col < cols; col++) {
            this->putchar(' ');
        }
    }
}

void HD44780_LCD::enableCursor(cursorDisp_t disp)
{
    ExtBusEnableCtx busEnable;
    cursorDisp = (cursorDisp_t)(disp & CURSOR_BLINK);
    bus[0] = 0x08 | (dispState << 2) | cursorDisp;
}

void HD44780_LCD::enableDisplay(bool enable)
{
    ExtBusEnableCtx busEnable;
    dispState = enable;
    bus[0] = 0x08 | (dispState << 2) | cursorDisp;
}
