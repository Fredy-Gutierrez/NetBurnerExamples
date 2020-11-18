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
#include <sim.h>
#include <same70.h>
#include <pins.h>
#include <defer.h>
#include <hal.h>
#include "ebi_pager.h"

PinVector<32>   EBI_Page[4];
uint32_t        EBI_Page_Siz[4];

extern volatile uint8_t ebi_0_base[];
extern volatile uint8_t ebi_1_base[];
extern volatile uint8_t ebi_2_base[];
extern volatile uint8_t ebi_3_base[];

void ChangeMPUPage(uint32_t region, uint32_t addr)
{
    MPU->RBAR = (addr & 0xFFFFFFE0) | MPU_RBAR_VALID_Msk | (region & 0xF);
}

extern void (*Default_Handler) ();
extern "C"
void MemManage_Handler()
{
    // If we reenter the the handler while handling, we've managed to double
    //  fault, so give up and reboot.
    IF_REENTERED(ForceReboot());
    uint32_t faultAddr;
    do {
    if (!(SCB->CFSR & SCB_CFSR_DACCVIOL_Msk)) {break;}

    faultAddr = SCB->MMFAR;
    // Clear the MemManage fault bits
    SCB->CFSR = 0xff;
    if (faultAddr < (uint32_t)ebi_0_base)               { break; }
    if (faultAddr >= ((uint32_t)(ebi_3_base + 0x1000000))){ break; }

    uint32_t csNum;
    uint32_t baseAddr;
    if (faultAddr < (uint32_t)ebi_1_base) {
        csNum = 0;
        baseAddr = (uint32_t)&ebi_0_base;
    }
    else if (faultAddr < (uint32_t)ebi_2_base) {
        csNum = 1;
        baseAddr = (uint32_t)&ebi_1_base;
    }
    else if (faultAddr < (uint32_t)ebi_3_base) {
        csNum = 2;
        baseAddr = (uint32_t)&ebi_2_base;
    }
    else {
        csNum = 3;
        baseAddr = (uint32_t)&ebi_3_base;
    }

    if (!EBI_Page_Siz[csNum]) { break; }

    uint32_t pageNumber = (faultAddr - baseAddr)/EBI_Page_Siz[csNum];

    ChangeMPUPage(MPU_REGION_EBI_PAGING + csNum,
            baseAddr+(pageNumber*EBI_Page_Siz[csNum]));

    EBI_Page[csNum] = pageNumber;
    asm ("dsb");

    return;

    } while (0);


    if (Default_Handler)
    {
        Default_Handler();
    }
}

void ConfigAddressPager(uint32_t csNum, uint32_t pageSize,
            PinIO *pageSelectPins, uint32_t pageSelectPinCount)
{
    if (csNum > 3) { return; }
    if (pageSize < 32) { return; }
    iprintf("Configuring Pins\n");
    // Build up the pins list
    EBI_Page[csNum].config(pageSelectPins, pageSelectPinCount);

    // Store the page size
    EBI_Page_Siz[csNum] = pageSize;

    MPU->RNR = MPU_REGION_EBI_WT;
    MPU->RBAR = ((uint32_t)ebi_0_base) | MPU_RBAR_VALID_Msk | MPU_REGION_EBI_WT;
    MPU->RASR = MPU_RASR_ENABLE_Msk | ( 26 << MPU_RASR_SIZE_Pos)
                | MPU_RASR_B_Msk | MPU_RASR_XN_Msk;

    MPU->RBAR = (((uint32_t)ebi_0_base) + (csNum << 24)) | MPU_RBAR_VALID_Msk
                | MPU_REGION_EBI_PAGING + csNum;

    // MPU Size field is 2^(siz+1) bytes
    // AP of 0b011 is 'All Read/Write Allowed'
    uint32_t mpuSiz = 30 - __CLZ(pageSize);
    MPU->RASR = MPU_RASR_ENABLE_Msk | (mpuSiz << MPU_RASR_SIZE_Pos)
                | MPU_RASR_B_Msk | (0x3 << MPU_RASR_AP_Pos);

    EBI_Page[csNum] = 0;
}

