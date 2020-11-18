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
#include <ebi.h>
#include "ebi_pager.h"
#include <bsp.h>

extern "C"
{
   void UserMain( void *pd );
};

const char *AppName = "SAME70 EBI Paging";

PinIO pagePins[8] = {
P2[24], P2[20], P2[19], P2[17], P2[23], P2[18], P2[16], P2[15]
};

/**
 * The following are linker generated symbols which land at the base address of each of the EBI Chip Selects.
 *
 */
extern volatile uint8_t ebi_0_base[];
extern volatile uint8_t ebi_1_base[];
extern volatile uint8_t ebi_2_base[];
extern volatile uint8_t ebi_3_base[];

/**
 * @brief Enables automatic Page Fault handling and configures a group of GPIOs
 *  to act as Page Select address lines. The GPIOs will automatically update on
 *  page faults such that page faults are invisible to normal code, allowing
 *  normal code to treat the paged area as a contiguous segment.
 *
 * @param csNum The EBI chip select to configure page handling for.
 * @param pageSize The size in bytes of each page attached to the given chip select.
 * @param pageSelectPins An array of PinIOs for the GPIO page select pins.
 *  Note: this array is least significant address line first.
 * @param pageSelectPinCount How many pins are in the pageSelectPins array
 * */
void ConfigAddressPager(uint32_t csNum, uint32_t pageSize,
            PinIO *pageSelectPins, uint32_t pageSelectPinCount);

EBI_CS_cfg_t cs0_cfg
{
    25, 15, 25, 15, 30, 65, 30, 65, 90, 90, 0,
    EBI_BUS_WIDTH_8,
    EBI_BYTE_ACCESS_SELECT,
    EBI_NWAIT_DISABLED,
    EBI_WRITE_MODE_NCS,
    EBI_READ_MODE_NCS
};

void UserMain(void *pd)
{
    init();
    EnableSmartTraps();

    // Configured the Page Select pins.
    for (int i = 0; i < 8; i++) {
        pagePins[i] = 0;
        pagePins[i].function(PIN_GPIO_OUT);
    }
    // Configure the EBI Pager to handle Page Faults for us
    ConfigAddressPager(0, 256, pagePins, 8);
    // Configure the EBI Chip Select to enable access to the bus
    ConfigureEBI_CS(0, cs0_cfg);

    // Enable the on-module bus buffers
    EnableExtBusBuff(true);

    while (1)
    {
        for (uint32_t page = 0; page < (256); page++)
        {
            iprintf("Page: %lu - %p\n", page, ebi_0_base+(page << 8));
            ebi_0_base[page << 8] = 0xFF;
            asm("dsb");
            OSTimeDly(1);
        }
    }
}
