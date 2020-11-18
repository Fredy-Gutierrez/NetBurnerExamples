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
#include <i2c.h>
#include <logme.h>
#include "wm8904.h"



WM8904::WM8904(I2C &module)
    : twi(module)
{
}

void WM8904::WriteReg(WM8904::Reg::RegAddr_t reg, uint16_t dat)
{
    dat = HTONS(dat);
    I2C::Result_t sr;
    int no_block = 0;
    do {
        sr = twi.writeRegN(WM8904_I2C_ID, reg, (uint8_t *)(&dat), 2);
        if ((sr == I2C::I2C_RES_ARB_LST) ||
                ((no_block > 2) &&(sr == I2C::I2C_RES_NACK)))
        {
            twi.resetBus();
        }
        no_block++;
    } while ((sr != I2C::I2C_RES_ACK) && (no_block < 10));
}

uint16_t WM8904::ReadReg(WM8904::Reg::RegAddr_t reg)
{
    uint16_t ret;
    I2C::Result_t sr;
    int no_block = 0;
    do {
        sr = twi.readRegN(WM8904_I2C_ID, reg, (uint8_t *)(&ret), 2);
        if (sr == I2C::I2C_RES_ARB_LST)
        {
            twi.resetBus();
        }
        no_block++;
    } while (!((sr == I2C::I2C_RES_ACK) || (sr == I2C::I2C_RES_NACK))
            && (no_block < 10));
    return HTONS(ret);
}


/************************************************************/
/************************************************************/
/*              INITIALIZATION COMMANDS                     */
/************************************************************/
/************************************************************/

/************************************************************/
/*   COMMAND BLOCK: 0                                       */
/************************************************************/
WM8904::Reg::cmd_t WM8904::Reg::init_cmds_0[] = {
    { CLK_RATES2,
        CLK_SYSCLK_SRC_MCLK | CLK_CLK_SYS_ENA | CLK_CLK_DSP_ENA
        | CLK_OPCLK_ENA | CLK_TOCLK_ENA,
        3 },
    { SWRST_ID, 0xFFFF, TICKS_PER_SECOND/4 },
    { BIAS_CTL0, BIAS_CTL0_ISEL_HI | 0x0010, 1 },
    { VMID_CTL0,
        VMID_CTL0_BUF_ENA | VMID_CTL0_RES_FAST | VMID_CTL0_ENA | 0x0010, 1 },
    { VMID_CTL0,
        VMID_CTL0_BUF_ENA | VMID_CTL0_RES_FAST | VMID_CTL0_ENA, 1 },
    { BIAS_CTL0, BIAS_CTL0_ISEL_HI | BIAS_CTL0_BIAS_ENA, 1 },
    { PWR_MGMNT2, PWR_MGMNT2_HPL_PGA_ENA | PWR_MGMNT2_HPR_PGA_ENA, 1 },
    { CHARGE_PUMP0, CHARGE_PUMP0_CP_ENA, 1},
    { CLASS_W0, 0x1, 1},
    { DGTL_PULLS, DGTL_PULLS_LRCLK_PU | DGTL_PULLS_BCLK_PU, 1 }
};

/************************************************************/
/*   COMMAND BLOCK: 1                                       */
/************************************************************/
WM8904::Reg::cmd_t WM8904::Reg::init_cmds_1[] = {
    { PWR_MGMNT3, PWR_MGMNT2_HPL_PGA_ENA | PWR_MGMNT2_HPR_PGA_ENA, 1 },
    { PWR_MGMNT6, PWR_MGMNT6_DACL_ENA | PWR_MGMNT6_DACR_ENA, 1},
    { ANA_HP0, ANA_HP0_HPL_ENA | ANA_HP0_HPR_ENA, 1},
    { ANA_LINEOUT0, ANA_HP0_HPL_ENA | ANA_HP0_HPR_ENA, 1},
    { ANA_HP0,
        ANA_HP0_HPL_ENA | ANA_HP0_HPR_ENA
        | ANA_HP0_HPL_ENA_DLY | ANA_HP0_HPR_ENA_DLY,
        1},
    { ANA_LINEOUT0,
        ANA_HP0_HPL_ENA | ANA_HP0_HPR_ENA
        | ANA_HP0_HPL_ENA_DLY | ANA_HP0_HPR_ENA_DLY,
        1},
    { DC_SERVO0,
        DC_SERVO0_ENA_LNL | DC_SERVO0_ENA_LNR
        | DC_SERVO0_ENA_HPL | DC_SERVO0_ENA_HPR,
        1},
    { DC_SERVO1,
        DC_SERVO1_TRIG_START_LNL | DC_SERVO1_TRIG_START_LNR
        | DC_SERVO1_TRIG_START_HPL | DC_SERVO1_TRIG_START_HPR,
        TICKS_PER_SECOND/4},
    { ANA_HP0,
        ANA_HP0_HPL_ENA_OUTP | ANA_HP0_HPL_ENA_DLY | ANA_HP0_HPL_ENA
        | ANA_HP0_HPR_ENA_OUTP | ANA_HP0_HPR_ENA_DLY | ANA_HP0_HPR_ENA,
        1},
    { ANA_LINEOUT0,
        ANA_HP0_HPL_ENA_OUTP | ANA_HP0_HPL_ENA_DLY | ANA_HP0_HPL_ENA
        | ANA_HP0_HPR_ENA_OUTP | ANA_HP0_HPR_ENA_DLY | ANA_HP0_HPR_ENA,
        1},
    { ANA_HP0,
        ANA_HP0_HPL_RM_SHORT | ANA_HP0_HPL_ENA_OUTP
        | ANA_HP0_HPL_ENA_DLY | ANA_HP0_HPL_ENA
        | ANA_HP0_HPR_RM_SHORT | ANA_HP0_HPR_ENA_OUTP
        | ANA_HP0_HPR_ENA_DLY | ANA_HP0_HPR_ENA,
        1},
    { ANA_LINEOUT0,
        ANA_HP0_HPL_RM_SHORT | ANA_HP0_HPL_ENA_OUTP
        | ANA_HP0_HPL_ENA_DLY | ANA_HP0_HPL_ENA
        | ANA_HP0_HPR_RM_SHORT | ANA_HP0_HPR_ENA_OUTP
        | ANA_HP0_HPR_ENA_DLY | ANA_HP0_HPR_ENA,
        1}
};

/************************************************************/
/*   COMMAND BLOCK: 2                                       */
/************************************************************/
WM8904::Reg::cmd_t WM8904::Reg::init_cmds_2[] = {
    { CLK_RATES1, CLK_SYS_RATE_256 | CLK_SAMPLE_RATE_48K, 1 },
    { CLK_RATES2,
        CLK_SYSCLK_SRC_MCLK | CLK_CLK_SYS_ENA | CLK_CLK_DSP_ENA
            | CLK_OPCLK_ENA | CLK_TOCLK_ENA,
        1 },
};
/************************************************************/
/*   COMMAND BLOCK: 3                                       */
/************************************************************/
WM8904::Reg::cmd_t WM8904::Reg::init_cmds_3[] = {
    { ANA_OUT1_L, ANA_OUT_VOLUME(0x0D), 1 },
    { ANA_OUT1_R, ANA_OUT_VOLUME(0x0D), 1 },
    { ANA_OUT1_L, ANA_OUT_VOL_UPDATE | ANA_OUT_VOLUME(0x0D), 1 },
    { ANA_OUT1_R, ANA_OUT_VOL_UPDATE | ANA_OUT_VOLUME(0x0D), 1 }
};

/************************************************************/
/*   COMMAND BLOCK: 4                                       */
/************************************************************/
WM8904::Reg::cmd_t WM8904::Reg::init_cmds_4[] = {
    { PWR_MGMNT6,
        PWR_MGMNT6_DACL_ENA | PWR_MGMNT6_DACR_ENA
        | PWR_MGMNT6_ADCL_ENA | PWR_MGMNT6_ADCR_ENA,
        1},
    { ANA_L_IN1, 0x54, 1},
    { ANA_R_IN1, 0x54, 1},
    { PWR_MGMNT0, PWR_MGMNT0_INL_ENA | PWR_MGMNT0_INR_ENA, 1},
    { AUDIO_INTF0,
        AUDIO_INTF0_DAC_SRCR| AUDIO_INTF0_ADC_SRCR,
        TICKS_PER_SECOND/4},
};

/************************************************************/
/*   COMMAND BLOCK: 5                                       */
/************************************************************/
WM8904::Reg::cmd_t WM8904::Reg::init_cmds_5[] = {
    { CLK_RATES2, CLK_SYSCLK_SRC_FLL, 1 },
    { FLL_CTL1, 0, 1 },
    { FLL_CTL2, FLL_OUTDIV(8), 1 },
    { FLL_CTL3, FLL_K(0xc49c), 1 },
    { FLL_CTL4, FLL_N(32), 1 },
    { FLL_CTL5, FLL_CLK_REF_DIV_1, 1 },
    { FLL_CTL1, FLL_FRACN_ENA_FRAC, 2 },
    { FLL_CTL1, FLL_FRACN_ENA_FRAC | FLL_ENA_EN, TICKS_PER_SECOND/4 },
};

/************************************************************/
/*   COMMAND BLOCK: 6                                       */
/************************************************************/
WM8904::Reg::cmd_t WM8904::Reg::init_cmds_6[] = {
    { DAC_DGTL1, 0x0, 1}, // WE MUST SET THE UN-MUTE IF WE WANT OUTPUT
    { CLK_RATES2,
        CLK_SYSCLK_SRC_FLL | CLK_CLK_SYS_ENA | CLK_CLK_DSP_ENA
        | CLK_OPCLK_ENA | CLK_TOCLK_ENA,
        1 },
};

void WM8904::SendCmd(WM8904::Reg::cmd_t cmd)
{
    WriteReg(cmd.reg,cmd.val);
    OSTimeDly(cmd.dly);
}

void WM8904::SendCmdList(Reg::cmd_t *cmds, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++)
    {
        SendCmd(cmds[i]);
    }
}

void WM8904::UpdateCmd(Reg::cmd_t cmd, uint16_t updateMask)
{
    uint16_t reg = ReadReg(cmd.reg) & (~updateMask);
    cmd.val = (cmd.val & updateMask) | reg;
    SendCmd(cmd);
}

void WM8904::InitInput(const cfg_t &cfg)
{
    Reg::cmd_t cmds[7] =
    {
    { Reg::PWR_MGMNT6,
        PWR_MGMNT6_DACL_ENA | PWR_MGMNT6_DACR_ENA
        | PWR_MGMNT6_ADCL_ENA | PWR_MGMNT6_ADCR_ENA,
        1},
    { Reg::ANA_L_IN1, (uint16_t)(((uint16_t)0x44U)|(uint16_t)(cfg.inSrc-1)), 1},
    { Reg::ANA_R_IN1, (uint16_t)(((uint16_t)0x44U)|(uint16_t)(cfg.inSrc-1)), 1},
    { Reg::ANA_L_IN0, ANA_IN_VOLUME(cfg.initMicGain), 1},
    { Reg::ANA_R_IN0, ANA_IN_VOLUME(cfg.initMicGain), 1},
    { Reg::PWR_MGMNT0, PWR_MGMNT0_INL_ENA | PWR_MGMNT0_INR_ENA, 1},
    { Reg::AUDIO_INTF0,
        AUDIO_INTF0_DAC_SRCR| AUDIO_INTF0_ADC_SRCR,
        TICKS_PER_SECOND/4},
    };

    SendCmdList(cmds,
            (sizeof(cmds)/sizeof(WM8904::Reg::cmd_t)));
}

static uint32_t fracNDiv(uint32_t dividend, uint32_t divisor)
{
    uint16_t quotient_N;
    uint16_t quotient_K = 0;

    uint32_t remainder;

    quotient_N = dividend/divisor;
    remainder = dividend - (divisor*quotient_N);
    divisor >>= 1;
    for (int shifts = 16; (divisor && remainder && shifts); shifts--)
    {
        if (remainder > divisor) {
            quotient_K |= 0x1 << (shifts-1);
            remainder -= divisor;
        }
        divisor >>= 1;
    }
    // round up any remaining remainder
    if ((remainder && divisor) && (remainder >= (divisor >> 1)))
    {
        quotient_K++;
    }
    return (((uint32_t)quotient_N) << 16) | ((uint32_t)quotient_K);
}

void WM8904::ConfigFLL(const cfg_t &cfg)
{
    uint32_t outDiv = 1;
    uint32_t fvco;
    do {
        fvco = (cfg.sampleRate*256*outDiv);
    } while ((fvco < 90000000) && (++outDiv < 0x3F));

    uint16_t fllRatio;
    if      (cfg.refClkRate >= 1000000) { fllRatio = FLL_CLK_REF_DIV_1; }
    else if (cfg.refClkRate >=  256000) { fllRatio = FLL_CLK_REF_DIV_2; }
    else if (cfg.refClkRate >=  128000) { fllRatio = FLL_CLK_REF_DIV_4; }
    else if (cfg.refClkRate >=   64000) { fllRatio = FLL_CLK_REF_DIV_8; }
    else                                { fllRatio = FLL_CLK_REF_DIV_16; }

    uint32_t fll_mult = fracNDiv(fvco, cfg.refClkRate);
    uint16_t fll_n = fll_mult >> 16;
    uint16_t fll_k = fll_mult & 0xFFFF;

    Reg::cmd_t cmds[8] =
    {
        { Reg::CLK_RATES2, CLK_SYSCLK_SRC_FLL, 1 },
        { Reg::FLL_CTL1, 0, 1 },
        { Reg::FLL_CTL2, FLL_OUTDIV(outDiv), 1 },
        { Reg::FLL_CTL3, FLL_K(fll_k), 1 },
        { Reg::FLL_CTL4, FLL_N(fll_n), 1 },
        { Reg::FLL_CTL5, fllRatio, 1 },
        { Reg::FLL_CTL1,
            (uint16_t)(fll_k ? FLL_FRACN_ENA_FRAC : 0), 2 },
        { Reg::FLL_CTL1,
            (uint16_t)((fll_k ? FLL_FRACN_ENA_FRAC : 0) | FLL_ENA_EN),
            TICKS_PER_SECOND/4 },
    };

    SendCmdList(cmds,
            (sizeof(cmds)/sizeof(WM8904::Reg::cmd_t)));
}

void WM8904::Init(const cfg_t &cfg, const SSC_cfg_t &ssc_cfg)
{
    ssc.Init(ssc_cfg);
    P2[26].function(PINP2_26_TWD2);
    P2[23].function(PINP2_23_TWCK2);
    P2[26].PullUp(true);
    P2[23].PullUp(true);
    twi.setup(30000);

    // If CLK_SYS_ENA was set, but MCLK disappeared due to a reset,
    //  we need to access Clock Rates 2 and clear CLK_SYS_ENA before we do
    //  *anything else*, otherwise the codec's control interface will hang.
    P2[24].function(PINP2_24_OUT);
    WriteReg(WM8904::Reg::CLK_RATES2,
                    CLK_SYSCLK_SRC_MCLK | CLK_CLK_SYS_ENA | CLK_OPCLK_ENA );
    P2[24].function(PINP2_24_PCK2);
    asm ("dsb");
    iprintf("Reading SWRST_ID\n");
    uint16_t reg = ReadReg(WM8904::Reg::SWRST_ID);
    iprintf("SWRST_ID: %#04x\n", reg);

    uint16_t lrRate;
    uint16_t fmt;
    switch(cfg.dataFormat) {
        case DATA_FMT_16_L_JUSTIFIED:
            fmt = AUDIO_INTF1_FMT_LEFT | AUDIO_INTF1_WL_16BIT;
            lrRate = 32;
            break;
        case DATA_FMT_32_L_JUSTIFIED:
            fmt = AUDIO_INTF1_FMT_LEFT | AUDIO_INTF1_WL_24BIT;
            lrRate = 64;
            break;
        case DATA_FMT_16_I2S:
            fmt = AUDIO_INTF1_FMT_I2S | AUDIO_INTF1_WL_16BIT;
            lrRate = 64;
            break;
        case DATA_FMT_32_I2S:
            fmt =  AUDIO_INTF1_FMT_I2S | AUDIO_INTF1_WL_24BIT;
            lrRate = 64;
            break;
        default:
            return;
    }
    uint32_t BClk = cfg.sampleRate*lrRate;
    uint32_t sysClk = cfg.sampleRate * 256;
    uint16_t BClkRate;
    switch (sysClk/BClk)
    {
    case 2:     BClkRate = AUDIO_INTF2_BCLK_DIV_2; break;
    case 8:     BClkRate = AUDIO_INTF2_BCLK_DIV_8; break;
    case 16:    BClkRate = AUDIO_INTF2_BCLK_DIV_16; break;
    case 4:     BClkRate = AUDIO_INTF2_BCLK_DIV_4; break;
    default:    BClkRate = AUDIO_INTF2_BCLK_DIV_4; break;
    }

    SendCmdList(WM8904::Reg::init_cmds_0,
                (sizeof(WM8904::Reg::init_cmds_0)/sizeof(WM8904::Reg::cmd_t)));

    SendCmdList(WM8904::Reg::init_cmds_1,
                (sizeof(WM8904::Reg::init_cmds_1)/sizeof(WM8904::Reg::cmd_t)));

    SendCmdList(WM8904::Reg::init_cmds_2,
                (sizeof(WM8904::Reg::init_cmds_2)/sizeof(WM8904::Reg::cmd_t)));

    {
        WM8904::Reg::cmd_t bitSettings[] =
        {
            { Reg::AUDIO_INTF2, BClkRate, 1 },
            { Reg::AUDIO_INTF3,
                (uint16_t)(AUDIO_INTF3_LRCLK_DIR_OUT | AUDIO_INTF3_LRCLK_RATE(lrRate)),
                1 },
            { Reg::AUDIO_INTF1,
                (uint16_t)(AUDIO_INTF1_BCLK_DIR_OUT | fmt | 0x4000), 1 },
        };
        SendCmdList(bitSettings,
                (sizeof(bitSettings)/sizeof(WM8904::Reg::cmd_t)));
    }

    SetVolume(cfg.outSrc, cfg.outCh, cfg.initOutVol);

    if (cfg.inSrc != AUDIO_IN_NONE) {
        InitInput(cfg);
    }

    this->ConfigFLL(cfg);

    SendCmdList(WM8904::Reg::init_cmds_6,
                (sizeof(WM8904::Reg::init_cmds_6)/sizeof(WM8904::Reg::cmd_t)));

    reg = ReadReg(WM8904::Reg::IRQ_STAT);
    iprintf("IRQ_STAT: %#04x\n", reg);

    reg = ReadReg(WM8904::Reg::GPIO_CTL1);
    iprintf("GPIO_CTL1: %#04x\n", reg);
}

void WM8904::SetVolume(WM8904::AudioOutSelect_t out,
                        WM8904::AudioChSelect_t channel,
                        uint8_t volume)
{
    Reg::cmd_t cmds[4];
    int idx = 0;
    Reg::RegAddr_t lCh;
    Reg::RegAddr_t rCh;
    switch (out)
    {
    case AUDIO_OUT_HP:
        lCh = Reg::ANA_OUT1_L; rCh = Reg::ANA_OUT1_R; break;
    case AUDIO_OUT_LINE:
        lCh = Reg::ANA_OUT2_L; rCh = Reg::ANA_OUT2_R; break;
    default:
        return;
    };
    switch (channel)
    {
    case CH_SELECT_LEFT:
        cmds[idx++] = { lCh, ANA_OUT_VOLUME(volume), 1 };
        cmds[idx++] = { lCh,
                        (uint16_t)(ANA_OUT_VOL_UPDATE | ANA_OUT_VOLUME(volume)),
                        1 };
        break;
    case CH_SELECT_RIGHT:
        cmds[idx++] = { rCh, ANA_OUT_VOLUME(volume), 1 };
        cmds[idx++] = { rCh,
                        (uint16_t)(ANA_OUT_VOL_UPDATE | ANA_OUT_VOLUME(volume)),
                        1 };
        break;
    case CH_SELECT_LEFT_RIGHT:
        cmds[idx++] = { lCh, ANA_OUT_VOLUME(volume), 1 };
        cmds[idx++] = { rCh, ANA_OUT_VOLUME(volume), 1 };
        cmds[idx++] = { lCh,
                        (uint16_t)(ANA_OUT_VOL_UPDATE | ANA_OUT_VOLUME(volume)),
                        1 };
        cmds[idx++] = { rCh,
                        (uint16_t)(ANA_OUT_VOL_UPDATE | ANA_OUT_VOLUME(volume)),
                        1 };
        break;
    default:
        return;
    }

    SendCmdList(cmds, idx-1);
}

uint8_t WM8904::GetVolume(AudioOutSelect_t out, AudioChSelect_t channel)
{
    Reg::RegAddr_t lCh;
    Reg::RegAddr_t rCh;
    switch (out)
    {
    case AUDIO_OUT_HP:
        lCh = Reg::ANA_OUT1_L; rCh = Reg::ANA_OUT1_R; break;
    case AUDIO_OUT_LINE:
        lCh = Reg::ANA_OUT2_L; rCh = Reg::ANA_OUT2_R; break;
    default:
        return 0x0000;
    };

    return ReadReg((channel == CH_SELECT_RIGHT) ? rCh : lCh) & 0x3F;
}

void WM8904::Mute(AudioOutSelect_t out, AudioChSelect_t channel, bool mute)
{
    Reg::RegAddr_t lCh;
    Reg::RegAddr_t rCh;
    uint16_t reg;
    switch (out)
    {
    case AUDIO_OUT_HP:
        lCh = Reg::ANA_OUT1_L; rCh = Reg::ANA_OUT1_R; break;
    case AUDIO_OUT_LINE:
        lCh = Reg::ANA_OUT2_L; rCh = Reg::ANA_OUT2_R; break;
    default:
        return;
    };
    Reg::cmd_t cmd{lCh, 0, 1};

    if (mute)   { cmd.val = ANA_OUT_MUTE; }
    else        { cmd.val = 0; }

    switch (channel)
    {
        case CH_SELECT_LEFT:
        {
            cmd.reg = lCh;
            UpdateCmd(cmd, ~ANA_OUT_MUTE);
        }
        break;
        case CH_SELECT_RIGHT:
        {
            cmd.reg = rCh;
            UpdateCmd(cmd, ~ANA_OUT_MUTE);
        }
        break;
        case CH_SELECT_LEFT_RIGHT:
        default:
        {
            cmd.reg = lCh;
            UpdateCmd(cmd, ~ANA_OUT_MUTE);
            cmd.reg = rCh;
            UpdateCmd(cmd, ~ANA_OUT_MUTE);
        }
        break;
    }
}

void WM8904::SetMicGain(WM8904::AudioChSelect_t channel, uint8_t gain)
{
    Reg::cmd_t cmds[2];
    int idx = 0;

    switch (channel)
    {
    case CH_SELECT_LEFT:
        cmds[idx++] = { Reg::ANA_L_IN0, ANA_IN_VOLUME(gain), 1};
        break;
    case CH_SELECT_RIGHT:
        cmds[idx++] = { Reg::ANA_R_IN0, ANA_IN_VOLUME(gain), 1};
        break;
    case CH_SELECT_LEFT_RIGHT:
        cmds[idx++] = { Reg::ANA_L_IN0, ANA_IN_VOLUME(gain), 1};
        cmds[idx++] = { Reg::ANA_R_IN0, ANA_IN_VOLUME(gain), 1};
        break;
    default:
        return;
    }

    SendCmdList(cmds, idx-1);
}

uint8_t WM8904::GetMicGain(AudioChSelect_t channel)
{
    return ReadReg((channel == CH_SELECT_RIGHT) ? Reg::ANA_R_IN0 : Reg::ANA_L_IN0) & 0x1F;
}

void WM8904::MuteMic(AudioChSelect_t channel, bool mute)
{
    Reg::cmd_t cmd{Reg::ANA_L_IN0, 0, 1};
    if (mute)   { cmd.val = ANA_IN_MUTE; }
    else        { cmd.val = 0; }

    switch (channel)
    {
        case CH_SELECT_LEFT:
        {
            cmd.reg = Reg::ANA_L_IN0;
            UpdateCmd(cmd, ~ANA_IN_MUTE);
        }
        break;
        case CH_SELECT_RIGHT:
        {
            cmd.reg = Reg::ANA_R_IN0;
            UpdateCmd(cmd, ~ANA_IN_MUTE);
        }
        break;
        case CH_SELECT_LEFT_RIGHT:
        default:
        {
            cmd.reg = Reg::ANA_L_IN0;
            UpdateCmd(cmd, ~ANA_IN_MUTE);

            cmd.reg = Reg::ANA_R_IN0;
            UpdateCmd(cmd, ~ANA_IN_MUTE);
        }
        break;
    }
}

int WM8904::TransmitBuffer(void *buffer, uint32_t bufferLen, bool waitIfNeeded)
{
    return ssc.TransmitBuffer(buffer, bufferLen, waitIfNeeded);
}

int WM8904::ReadyReceiveBuffer(void *buffer, uint32_t bufferLen, bool waitIfNeeded)
{
    return ssc.ReadyReceiveBuffer(buffer, bufferLen, waitIfNeeded);
}

void WM8904::RegisterTxBufferDoneCB(SSC_BufferDoneFn_t cb)
{
    ssc.RegisterTxBufferDoneCB(cb);
}

void WM8904::RegisterRxBufferDoneCB(SSC_BufferDoneFn_t cb)
{
    ssc.RegisterRxBufferDoneCB(cb);
}

