#ifndef  __HD44780_H
#define  __HD44780_H
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

class HD44780_LCD_Bus {
    volatile uint8_t bus[2];
    class reg {
        uint8_t addr;
        volatile uint8_t * lcd;
public:
        reg(uint8_t addr, volatile uint8_t * lcd) : addr(addr), lcd(lcd) {}
        operator uint8_t() { return read(); }
        uint8_t operator=(uint8_t val) { write(val); return val; }

        uint8_t read() { return lcd[addr]; }
        void write(uint8_t val)
        {
            while (lcd[0] & 0x80) { asm("dsb"); }
            lcd[addr] = val;
        }
    };
public:
    HD44780_LCD_Bus();

    void init(uint8_t cols, uint8_t rows);
    void putchar(char c);
    void putstr(const char *str);
    void setCursor(int row, int col);
    void clr();

    reg operator[] (int i) { return reg(i & 0x1, bus); }
    friend class HD44780_LCD;
};

class HD44780_LCD {
public:
    enum cursorDisp_t {
        CURSOR_OFF  = 0,
        CURSOR_ON   = 2,
        CURSOR_BLINK= 3
    };

private:
    HD44780_LCD_Bus &bus;
    uint8_t cols;
    uint8_t rows;
    cursorDisp_t cursorDisp;
    bool dispState;
public:
    HD44780_LCD(HD44780_LCD_Bus &bus, uint8_t cols, uint8_t rows);

    void init() { bus.init(cols, rows); }
    void putchar(char c) { bus.putchar(c); }
    void putstr(const char *str) { bus.putstr(str); }
    void setCursor(int row, int col) { bus.setCursor(row, col); }
    void enableCursor(cursorDisp_t disp);
    void enableDisplay(bool enable);
    void clr();

    HD44780_LCD_Bus::reg operator[] (int i) { return bus[i]; }
};



#endif   /* ----- #ifndef __HD44780_H  ----- */
