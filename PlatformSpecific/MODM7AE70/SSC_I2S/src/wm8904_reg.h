#ifndef  __WM8904_REG_H
#define  __WM8904_REG_H
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

#define WM8904_I2C_ID 0x1A

#define BIAS_CTL0_ISEL_HI       (0x2 << 2)
#define BIAS_CTL0_ISEL_LO       (0x0 << 2)
#define BIAS_CTL0_BIAS_ENA      (1 << 0)

#define VMID_CTL0_BUF_ENA       (1 << 6)
#define VMID_CTL0_RES_DIS       (0 << 1)
#define VMID_CTL0_RES_MID       (1 << 1)
#define VMID_CTL0_RES_SLOW      (2 << 1)
#define VMID_CTL0_RES_FAST      (3 << 1)
#define VMID_CTL0_ENA           (1 << 0)

#define PWR_MGMNT0_INL_ENA      (1 << 1)
#define PWR_MGMNT0_INR_ENA      (1 << 0)

#define PWR_MGMNT2_HPL_PGA_ENA  (1 << 1)
#define PWR_MGMNT2_HPR_PGA_ENA  (1 << 0)

#define PWR_MGMNT6_DACL_ENA     (1 << 3)
#define PWR_MGMNT6_DACR_ENA     (1 << 2)
#define PWR_MGMNT6_ADCL_ENA     (1 << 1)
#define PWR_MGMNT6_ADCR_ENA     (1 << 0)

#define CHARGE_PUMP0_CP_ENA     (1 << 0)

#define CLK_SYS_RATE_64         (0 << 10)
#define CLK_SYS_RATE_128        (1 << 10)
#define CLK_SYS_RATE_192        (2 << 10)
#define CLK_SYS_RATE_256        (3 << 10)
#define CLK_SYS_RATE_384        (4 << 10)
#define CLK_SYS_RATE_512        (5 << 10)
#define CLK_SYS_RATE_768        (6 << 10)
#define CLK_SYS_RATE_1024       (7 << 10)
#define CLK_SYS_RATE_1408       (8 << 10)
#define CLK_SYS_RATE_1536       (9 << 10)
#define CLK_SAMPLE_RATE_8K      (0 << 10)
#define CLK_SAMPLE_RATE_12K     (1 << 10)
#define CLK_SAMPLE_RATE_16K     (2 << 10)
#define CLK_SAMPLE_RATE_24K     (3 << 10)
#define CLK_SAMPLE_RATE_32K     (4 << 10)
#define CLK_SAMPLE_RATE_48K     (5 << 10)

#define CLK_MCLK_INV            (1 << 15)
#define CLK_SYSCLK_SRC_MCLK     (0 << 14)
#define CLK_SYSCLK_SRC_FLL      (1 << 14)
#define CLK_TOCLK_RATE_DIV2     (0 << 12)
#define CLK_TOCLK_RATE_DIV1     (1 << 12)
#define CLK_OPCLK_ENA           (1 <<  3)
#define CLK_CLK_SYS_ENA         (1 <<  2)
#define CLK_CLK_DSP_ENA         (1 <<  1)
#define CLK_TOCLK_ENA           (1 <<  0)

#define AUDIO_INTF0_DACL_DATINV     (1 << 12)
#define AUDIO_INTF0_DACR_DATINV     (1 << 11)
#define AUDIO_INTF0_DAC_BOOST(x)    ((x&3) << 9)
#define AUDIO_INTF0_LOOPBACK        (1 << 8)
#define AUDIO_INTF0_ADC_SRCL        (1 << 7)
#define AUDIO_INTF0_ADC_SRCR        (1 << 6)
#define AUDIO_INTF0_DAC_SRCL        (1 << 5)
#define AUDIO_INTF0_DAC_SRCR        (1 << 4)
#define AUDIO_INTF0_ADC_COMP        (1 << 3)
#define AUDIO_INTF0_ADC_COMPMODE    (1 << 2)
#define AUDIO_INTF0_DAC_COMP        (1 << 1)
#define AUDIO_INTF0_DAC_COMPMODE    (1 << 0)

#define AUDIO_INTF2_OPCLK_DIV_1     (0 << 8)
#define AUDIO_INTF2_OPCLK_DIV_2     (1 << 8)
#define AUDIO_INTF2_OPCLK_DIV_3     (2 << 8)
#define AUDIO_INTF2_OPCLK_DIV_4     (3 << 8)
#define AUDIO_INTF2_OPCLK_DIV_5_5   (4 << 8)
#define AUDIO_INTF2_OPCLK_DIV_6     (5 << 8)
#define AUDIO_INTF2_OPCLK_DIV_8     (6 << 8)
#define AUDIO_INTF2_OPCLK_DIV_12    (7 << 8)
#define AUDIO_INTF2_OPCLK_DIV_16    (8 << 8)
#define AUDIO_INTF2_BCLK_DIV_1      (0 << 0)
#define AUDIO_INTF2_BCLK_DIV_1_5    (1 << 0)
#define AUDIO_INTF2_BCLK_DIV_2      (2 << 0)
#define AUDIO_INTF2_BCLK_DIV_3      (3 << 0)
#define AUDIO_INTF2_BCLK_DIV_4      (4 << 0)
#define AUDIO_INTF2_BCLK_DIV_5      (5 << 0)
#define AUDIO_INTF2_BCLK_DIV_5_5    (6 << 0)
#define AUDIO_INTF2_BCLK_DIV_6      (7 << 0)
#define AUDIO_INTF2_BCLK_DIV_8      (8 << 0)
#define AUDIO_INTF2_BCLK_DIV_10     (9 << 0)
#define AUDIO_INTF2_BCLK_DIV_11     (10 << 0)
#define AUDIO_INTF2_BCLK_DIV_12     (11 << 0)
#define AUDIO_INTF2_BCLK_DIV_16     (12 << 0)
#define AUDIO_INTF2_BCLK_DIV_20     (13 << 0)
#define AUDIO_INTF2_BCLK_DIV_22     (14 << 0)
#define AUDIO_INTF2_BCLK_DIV_24     (15 << 0)
#define AUDIO_INTF2_BCLK_DIV_25     (16 << 0)
#define AUDIO_INTF2_BCLK_DIV_30     (17 << 0)
#define AUDIO_INTF2_BCLK_DIV_32     (18 << 0)
#define AUDIO_INTF2_BCLK_DIV_44     (19 << 0)
#define AUDIO_INTF2_BCLK_DIV_48     (20 << 0)

#define AUDIO_INTF1_AIFDAC_TDM_NORM (0 << 13)
#define AUDIO_INTF1_AIFDAC_TDM_TDM  (1 << 13)
#define AUDIO_INTF1_AIFDAC_TDM_CHAN0    (0 << 12)
#define AUDIO_INTF1_AIFDAC_TDM_CHAN1    (1 << 12)
#define AUDIO_INTF1_AIFADC_TDM_NORM (0 << 11)
#define AUDIO_INTF1_AIFADC_TDM_TDM  (1 << 11)
#define AUDIO_INTF1_AIFADC_TDM_CHAN0    (0 << 10)
#define AUDIO_INTF1_AIFADC_TDM_CHAN1    (1 << 10)
#define AUDIO_INTF1_AIF_TRIS        (1 << 8)
#define AUDIO_INTF1_BCLK_INV        (1 << 7)
#define AUDIO_INTF1_BCLK_DIR_IN     (0 << 6)
#define AUDIO_INTF1_BCLK_DIR_OUT    (1 << 6)
#define AUDIO_INTF1_LRCLK_INV       (1 << 6)
#define AUDIO_INTF1_WL_16BIT        (0 << 2)
#define AUDIO_INTF1_WL_20BIT        (1 << 2)
#define AUDIO_INTF1_WL_24BIT        (2 << 2)
#define AUDIO_INTF1_WL_32BIT        (3 << 2)
#define AUDIO_INTF1_FMT_RIGHT       (0 << 0)
#define AUDIO_INTF1_FMT_LEFT        (1 << 0)
#define AUDIO_INTF1_FMT_I2S         (2 << 0)
#define AUDIO_INTF1_FMT_DSP         (3 << 0)

#define AUDIO_INTF3_LRCLK_DIR_IN    (0 << 11)
#define AUDIO_INTF3_LRCLK_DIR_OUT   (1 << 11)
#define AUDIO_INTF3_LRCLK_RATE(x)   ((x)&0x7FF)

#define DC_SERVO0_ENA_LNR       (1 << 3)
#define DC_SERVO0_ENA_LNL       (1 << 2)
#define DC_SERVO0_ENA_HPR       (1 << 1)
#define DC_SERVO0_ENA_HPL       (1 << 0)

#define DC_SERVO1_TRIG_1_LNR    (1 << 15)
#define DC_SERVO1_TRIG_1_LNL    (1 << 14)
#define DC_SERVO1_TRIG_1_HPR    (1 << 13)
#define DC_SERVO1_TRIG_1_HPL    (1 << 12)
#define DC_SERVO1_TRIG_n_LNR    (1 << 11)
#define DC_SERVO1_TRIG_n_LNL    (1 << 10)
#define DC_SERVO1_TRIG_n_HPR    (1 << 9)
#define DC_SERVO1_TRIG_n_HPL    (1 << 8)
#define DC_SERVO1_TRIG_START_LNR    (1 << 7)
#define DC_SERVO1_TRIG_START_LNL    (1 << 6)
#define DC_SERVO1_TRIG_START_HPR    (1 << 5)
#define DC_SERVO1_TRIG_START_HPL    (1 << 4)
#define DC_SERVO1_TRIG_DAC_WR_LNR   (1 << 3)
#define DC_SERVO1_TRIG_DAC_WR_LNL   (1 << 2)
#define DC_SERVO1_TRIG_DAC_WR_HPR   (1 << 1)
#define DC_SERVO1_TRIG_DAC_WR_HPL   (1 << 0)

#define FLL_FRACN_ENA_INT       (0 << 2)
#define FLL_FRACN_ENA_FRAC      (1 << 2)
#define FLL_OSC_ENA_DIS         (0 << 1)
#define FLL_OSC_ENA_EN          (1 << 1)
#define FLL_ENA_DIS             (0 << 0)
#define FLL_ENA_EN              (1 << 0)

#define FLL_OUTDIV(x)           ((uint16_t)(((x-1)&0x3F)<<8))
#define FLL_CTRL_RATE_FVCO_1    (0 << 4)
#define FLL_CTRL_RATE_FVCO_2    (1 << 4)
#define FLL_CTRL_RATE_FVCO_3    (2 << 4)
#define FLL_CTRL_RATE_FVCO_4    (3 << 4)
#define FLL_CTRL_RATE_FVCO_5    (4 << 4)
#define FLL_CTRL_RATE_FVCO_6    (5 << 4)
#define FLL_CTRL_RATE_FVCO_7    (6 << 4)
#define FLL_CTRL_RATE_FVCO_8    (7 << 4)
#define FLL_FRATIO_DIV_1        (0 << 0)
#define FLL_FRATIO_DIV_2        (1 << 0)
#define FLL_FRATIO_DIV_4        (2 << 0)
#define FLL_FRATIO_DIV_8        (3 << 0)
#define FLL_FRATIO_DIV_16       (7 << 0)

#define FLL_K(x)        ((uint16_t)(((x)&0xFFFF)<<0))

#define FLL_N(x)        ((uint16_t)(((x)&0x3FF)<<5))
#define FLL_GAIN_1      (0 << 0)
#define FLL_GAIN_2      (1 << 0)
#define FLL_GAIN_4      (2 << 0)
#define FLL_GAIN_8      (3 << 0)
#define FLL_GAIN_16     (4 << 0)
#define FLL_GAIN_32     (5 << 0)
#define FLL_GAIN_64     (6 << 0)
#define FLL_GAIN_128    (7 << 0)
#define FLL_GAIN_256    (8 << 0)

#define FLL_CLK_REF_DIV_1       (0 << 3)
#define FLL_CLK_REF_DIV_2       (1 << 3)
#define FLL_CLK_REF_DIV_4       (2 << 3)
#define FLL_CLK_REF_DIV_8       (3 << 3)
#define FLL_CLK_REF_DIV_16      (4 << 3)
#define FLL_CLK_REF_SRC_MCLK    (0 << 0)
#define FLL_CLK_REF_SRC_BCLK    (1 << 0)
#define FLL_CLK_REF_SRC_LRCLK   (2 << 0)


#define GPIO_CTL_GPIO_PU        (1 << 5)
#define GPIO_CTL_GPIO_PD        (1 << 4)
#define GPIO_CTL_GPIO_IN        (0 << 0)
#define GPIO_CTL_GPIO_CLK_OUT   (1 << 0)
#define GPIO_CTL_GPIO_LO        (2 << 0)
#define GPIO_CTL_GPIO_HI        (3 << 0)
#define GPIO_CTL_GPIO_IRQ       (4 << 0)
#define GPIO_CTL_GPIO_FLL_LOCK  (5 << 0)
#define GPIO_CTL_GPIO_MIC_DET   (6 << 0)
#define GPIO_CTL_GPIO_MIC_SHORT (7 << 0)
#define GPIO_CTL_GPIO_DMIC_CLKO (8 << 0)
#define GPIO_CTL_GPIO_FLL_CLKO  (9 << 0)

#define ANA_IN_MUTE             (1 << 7)
#define ANA_IN_VOLUME(x)        ((uint16_t)(((x)&0x1F) << 0))

#define ANA_HP0_HPL_RM_SHORT    (1 << 7)
#define ANA_HP0_HPL_ENA_OUTP    (1 << 6)
#define ANA_HP0_HPL_ENA_DLY     (1 << 5)
#define ANA_HP0_HPL_ENA         (1 << 4)
#define ANA_HP0_HPR_RM_SHORT    (1 << 3)
#define ANA_HP0_HPR_ENA_OUTP    (1 << 2)
#define ANA_HP0_HPR_ENA_DLY     (1 << 1)
#define ANA_HP0_HPR_ENA         (1 << 0)

#define ANA_OUT_MUTE            (1 << 8)
#define ANA_OUT_VOL_UPDATE      (1 << 7)
#define ANA_OUT_ZERO_CROSS      (1 << 6)
#define ANA_OUT_VOLUME(x)       ((uint16_t)((x&0x3F) << 0))

#define DGTL_PULLS_MCLK_PU      (1 << 7)
#define DGTL_PULLS_MCLK_PD      (1 << 6)
#define DGTL_PULLS_DACDAT_PU    (1 << 5)
#define DGTL_PULLS_DACDAT_PD    (1 << 4)
#define DGTL_PULLS_LRCLK_PU     (1 << 3)
#define DGTL_PULLS_LRCLK_PD     (1 << 2)
#define DGTL_PULLS_BCLK_PU      (1 << 1)
#define DGTL_PULLS_BCLK_PD      (1 << 0)

#define FLL_NCO_FRC_NCO_EN      (1 << 0)
#define FLL_NCO_FRC_NCO_DIS     (0 << 0)

#endif   /* ----- #ifndef __WM8904_REG_H  ----- */
