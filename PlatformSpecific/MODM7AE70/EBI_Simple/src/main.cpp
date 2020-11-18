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
#include <bsp.h>

extern "C"
{
   void UserMain( void *pd );
};

const char *AppName = "SAME70 EBI Simple";

// The following two variables are the address and data used for the bus test
const uint32_t EBITestAddressOffset = 0xA5A5;
const uint16_t EBITestData16 = 0x5A5A;
const uint8_t EBITestData8 = 0x5A;

/**
 * The following are linker generated symbols which land at the base address of each of the EBI Chip Selects.
 *
 */
extern volatile uint8_t ebi_0_base[];
extern volatile uint8_t ebi_1_base[];
extern volatile uint8_t ebi_2_base[];
extern volatile uint8_t ebi_3_base[];

/**
 * struct EBI_CS_cfg_t
 * Configuration structure for an EBI chip select.
 *
 * EBI_CS_cfg_t::ncs_rd_setup
 *  NCS setup length = (128* ncs_rd_setup[5] + ncs_rd_setup[4:0]) clock cycles
 * EBI_CS_cfg_t::nrd_setup
 *  NRD setup length = (128* nrd_setup[5] + nrd_setup[4:0]) clock cycles
 * EBI_CS_cfg_t::ncs_wr_setup
 *  NCS setup length = (128* ncs_wr_setup[5] + ncs_wr_setup[4:0]) clock cycles
 * EBI_CS_cfg_t::nwe_setup
 *  NWE setup length = (128* nwe_setup[5] + nwe_setup[4:0]) clock cycles
 *
 * EBI_CS_cfg_t::ncs_rd_pulse
 *  NCS pulse length = (256* ncs_rd_pulse[6] + ncs_rd_pulse[5:0]) clock cycles
 * EBI_CS_cfg_t::nrd_pulse
 *  NRD pulse length = (256* nrd_pulse[6] + nrd_pulse[5:0]) clock cycles
 * EBI_CS_cfg_t::ncs_wr_pulse
 *  NCS pulse length = (256* ncs_wr_pulse[6] + ncs_wr_pulse[5:0]) clock cycles
 * EBI_CS_cfg_t::nwe_pulse
 *  NWE pulse length = (256* nwe_pulse[6] + nwe_pulse[5:0]) clock cycles
 *
 * EBI_CS_cfg_t::nrd_cycles
 *  The total read cycle length is the total duration in clock cycles of the
 *  read cycle. It is equal to the sum of the setup, pulse and hold steps of
 *  the NRD and NCS signals. It is defined as:
 *  Read cycle length = (nrd_cycles[8:7]*256 + nrd_cycles[6:0]) clock cycles
 * EBI_CS_cfg_t::nwe_cycles
 *  The total write cycle length is the total duration in clock cycles of the
 *  write cycle. It is equal to the sum of the setup, pulse and hold steps of
 *  the NWE and NCS signals. It is defined as:
 *  Write cycle length = (nwe_cycles[8:7]*256 + nwe_cycles[6:0]) clock cycles
 *
 * EBI_CS_cfg_t::tdf_cycles
 *  The number of clock cycles required by the external device to release
 *  the data after the rising edge of the read controlling signal.
 *  The SMC always provide one full cycle of bus turnaround after the
 *  TDF_CYCLES period.
 *  Note: Valid range => 0-15
 * EBI_CS_cfg_t::busWidth
 *  Data bus width
 * EBI_CS_cfg_t::byteAccess
 *  Byte Access mode, only used for 16-bit bus
 * EBI_CS_cfg_t::nWait
 *  NWAIT signal mode. Used to extend read/write pulse by the bus device.
 * EBI_CS_cfg_t::wrMode
 *  Configures which signal is used to signal a bus write.
 * EBI_CS_cfg_t::rdMode
 *  Configures which signal is used to signal a bus read.
 * */

// Example EBI configuration
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

    // Configure the EBI Chip Select to enable access to the bus
    ConfigureEBI_CS(0, cs0_cfg);

    // Enable the on-module bus buffers
    EnableExtBusBuff(true);

    while (1)
    {
        iprintf("\r\nEnter (R) for bus read or (W) for bus write:");
        char c = getchar();
        if( toupper(c) == 'W' )
        {
        	ebi_0_base[EBITestAddressOffset] = EBITestData8;
        	iprintf("\r\nAddress 0x%08X written with 0x%04X\r\n", &ebi_0_base[EBITestAddressOffset], EBITestData8 );
        }
        else if( toupper(c) == 'R' )
        {
        	uint32_t EBIdata = ebi_0_base[EBITestAddressOffset];
        	iprintf("\r\nRead 0x%04X from Address 0x%08X\r\n", EBIdata,  &ebi_0_base[EBITestAddressOffset] );
        }
    }
}
