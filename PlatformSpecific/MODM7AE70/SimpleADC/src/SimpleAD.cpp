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
#include <nbrtos.h>
#include <basictypes.h>
#include <sim.h>
#include <stdio.h>


extern volatile uint32_t PERIPH_CLOCK;
class AFECLock
{
    Afec &afec;
    static OS_CRIT lock[2];
public:
    AFECLock(uint32_t i) : afec((i) ? *AFEC1 : *AFEC0)
    {
        lock[&afec==AFEC1].Enter();
        afec.AFEC_WPMR = AFEC_WPMR_WPKEY_PASSWD;
    }
    ~AFECLock()
    {
        if (lock[&afec==AFEC1].OSCritDepthCount == 1)
        {
            afec.AFEC_WPMR = AFEC_WPMR_WPKEY_PASSWD | AFEC_WPMR_WPEN;
        }
        lock[&afec==AFEC1].Leave();
    }
};
OS_CRIT AFECLock::lock[2];

void InitSingleEndAD()
{
    // Enable the Peripheral Clock to both AFECs
    PMC->PMC_PCR = PMC_PCR_CMD | PMC_PCR_EN | PMC_PCR_PID(ID_AFEC0);
    PMC->PMC_PCR = PMC_PCR_CMD | PMC_PCR_EN | PMC_PCR_PID(ID_AFEC1);

    {
        AFECLock lock0(0), lock1(1);

        AFEC0->AFEC_CR = AFEC_CR_SWRST;
        AFEC1->AFEC_CR = AFEC_CR_SWRST;
        asm ("dsb");

        uint32_t reg;
        reg =  AFEC_MR_TRGEN_DIS // No hardware trigger
            | AFEC_MR_SLEEP_SLEEP // Sleep the AFEC when not in use
            | AFEC_MR_PRESCAL((PERIPH_CLOCK/25000000)-1) // AFEC clk of 25MHz
            | AFEC_MR_STARTUP_SUT112 // Startup dly of 4.48uS @ 25MHz AFEC clk
            | AFEC_MR_TRACKTIM(2) // Zero additional tracking cyles, good up to 148k input impedance
            | AFEC_MR_TRANSFER(1)
            | 1 << 23
            ;
        // Ready AFEC0 and AFEC1
        AFEC0->AFEC_MR = reg;
        AFEC1->AFEC_MR = reg;

        reg = AFEC_EMR_RES_NO_AVERAGE | AFEC_EMR_TAG | AFEC_EMR_STM
            | AFEC_EMR_SIGNMODE_SE_UNSG_DF_SIGN;
        AFEC0->AFEC_EMR = reg;
        AFEC1->AFEC_EMR = reg;
        reg = AFEC_ACR_IBCTL(2) | AFEC_ACR_PGA0EN | AFEC_ACR_PGA1EN;
        AFEC0->AFEC_ACR = reg;
        AFEC1->AFEC_ACR = reg;
        for (int i = 0; i < 12; i++)
        {
            AFEC0->AFEC_CSELR = i;
            AFEC1->AFEC_CSELR = i;
            asm ("dsb");
            AFEC0->AFEC_COCR = 0x200;
            AFEC1->AFEC_COCR = 0x200;
        }
    }
}

// Most channels are consumed by the DRAM, this is the bitmask of what's left
const uint16_t chAvail[2] = { 0b1100'0110'0111, 0b0000'0000'1000 };

struct ADpiomap_t {
    uint8_t pio : 3;
    uint8_t ch : 5;
};

// Map from AFEC module number and channel to PIO instance and PIO channel
static const ADpiomap_t piomap[2][12] = {
{
    {3, 30}, {0, 21}, {1,  3}, {4,  5}, {4,  4}, {1,  2},
    {0, 17}, {0, 18}, {0, 19}, {0, 20}, {1,  0}, {7, 31}
},
{
    {1,  1}, {2, 13}, {2, 15}, {2, 12}, {2, 29}, {2, 30},
    {2, 31}, {2, 26}, {2, 27}, {2,  0}, {4,  3}, {4,  0}
}
};


void ADConfigCh(unsigned nAfec, unsigned ch, bool enable)
{
    if ((nAfec > 1) || !(chAvail[nAfec] & (1 << ch))) { return; }

    AFECLock lock(nAfec);
    Afec &afec = (nAfec) ? *AFEC1 : *AFEC0;
    if (enable) { afec.AFEC_CHER = 1 << ch; }
    else        { afec.AFEC_CHDR = 1 << ch; }

    // If the channel is external (aka, not the internal temperature sensor)...
    if (piomap[nAfec][ch].pio <= 4)
    {
        // Disable the internal Pull Up and Pull Down for the channel's IO pin
        Pio &pio = PIOA[piomap[nAfec][ch].pio];
        pio.PIO_PPDDR = 1 << piomap[nAfec][ch].ch;
        pio.PIO_PUDR  = 1 << piomap[nAfec][ch].ch;
    }
}

void StartAD()
{
    AFECLock lock0(0), lock1(1);
    AFEC0->AFEC_CR = AFEC_CR_START;
    AFEC1->AFEC_CR = AFEC_CR_START;
}

bool ADDone()
{
    uint32_t chEn[2];
    chEn[0] = AFEC0->AFEC_CHSR;
    chEn[1] = AFEC1->AFEC_CHSR;
    return ((AFEC0->AFEC_ISR & chEn[0]) == chEn[0])
            && ((AFEC1->AFEC_ISR & chEn[1]) == chEn[1]);
}

uint16_t GetADResult(unsigned nAfec, unsigned ch) //Get the AD Result
{
    AFECLock lock(nAfec);
    if ((nAfec > 1) || (ch > 11)) { return 0; }
    Afec &afec = (nAfec) ? *AFEC1 : *AFEC0;

    afec.AFEC_CSELR = ch;
    asm ("dsb");
    uint16_t ret = afec.AFEC_CDR;
    return ret;
}

