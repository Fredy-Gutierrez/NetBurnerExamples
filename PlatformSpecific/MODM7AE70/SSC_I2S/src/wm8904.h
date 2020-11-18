#ifndef  __WM8904_H
#define  __WM8904_H
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
#include "ssc_i2s.h"

#include "wm8904_reg.h"

/**
 * @class WM8904
 * WM8904 is a driver for the WM8904 Audio Codec.
 */
class WM8904 {
public:
    struct Reg {
    enum RegAddr_t {
        SWRST_ID       = 0x00,
        BIAS_CTL0      = 0x04,
        VMID_CTL0      = 0x05,
        MIC_BIAS_CTL0  = 0x06,
        MIC_BIAS_CTL1  = 0x07,
        ANA_ADC0       = 0x0A,
        PWR_MGMNT0     = 0x0C,
        PWR_MGMNT2     = 0x0E,
        PWR_MGMNT3     = 0x0F,
        PWR_MGMNT6     = 0x12,
        CLK_RATES0     = 0x14,
        CLK_RATES1     = 0x15,
        CLK_RATES2     = 0x16,
        AUDIO_INTF0    = 0x18,
        AUDIO_INTF1    = 0x19,
        AUDIO_INTF2    = 0x1A,
        AUDIO_INTF3    = 0x1B,
        DAC_DGTL_VOL_L = 0x1E,
        DAC_DGTL_VOL_R = 0x1F,
        DAC_DGTL0      = 0x20,
        DAC_DGTL1      = 0x21,
        ADC_DGTL_VOL_L = 0x24,
        ADC_DGTL_VOL_R = 0x25,
        ADC_DGTL0      = 0x26,
        DGTL_MIC0      = 0x27,
        DRC0           = 0x28,
        DRC1           = 0x29,
        DRC2           = 0x2A,
        DRC3           = 0x2B,
        ANA_L_IN0      = 0x2C,
        ANA_R_IN0      = 0x2D,
        ANA_L_IN1      = 0x2E,
        ANA_R_IN1      = 0x2F,
        ANA_OUT1_L     = 0x39,
        ANA_OUT1_R     = 0x3A,
        ANA_OUT2_L     = 0x3B,
        ANA_OUT2_R     = 0x3C,
        ANA_OUT12_ZC   = 0x3D,
        DC_SERVO0      = 0x43,
        DC_SERVO1      = 0x44,
        DC_SERVO2      = 0x45,
        DC_SERVO4      = 0x47,
        DC_SERVO5      = 0x48,
        DC_SERVO6      = 0x49,
        DC_SERVO7      = 0x4A,
        DC_SERVO8      = 0x4B,
        DC_SERVO9      = 0x4C,
        DC_SERVO_RDBK0 = 0x4D,
        ANA_HP0        = 0x5A,
        ANA_LINEOUT0   = 0x5E,
        CHARGE_PUMP0   = 0x62,
        CLASS_W0       = 0x68,
        WR_SEQ0        = 0x6C,
        WR_SEQ1        = 0x6D,
        WR_SEQ2        = 0x6E,
        WR_SEQ3        = 0x6F,
        WR_SEQ4        = 0x70,
        FLL_CTL1       = 0x74,
        FLL_CTL2       = 0x75,
        FLL_CTL3       = 0x76,
        FLL_CTL4       = 0x77,
        FLL_CTL5       = 0x78,
        GPIO_CTL1      = 0x79,
        GPIO_CTL2      = 0x7A,
        GPIO_CTL3      = 0x7B,
        GPIO_CTL4      = 0x7C,
        DGTL_PULLS     = 0x7E,
        IRQ_STAT       = 0x7F,
        IRQ_STAT_MASK  = 0x80,
        IRQ_POL        = 0x81,
        IRQ_DEBOUNCE   = 0x82,
        EQ1            = 0x86,
        EQ2            = 0x87,
        EQ3            = 0x88,
        EQ4            = 0x89,
        EQ5            = 0x8A,
        EQ6            = 0x8B,
        EQ7            = 0x8C,
        EQ8            = 0x8D,
        EQ9            = 0x8E,
        EQ10           = 0x8F,
        EQ11           = 0x90,
        EQ12           = 0x91,
        EQ13           = 0x92,
        EQ14           = 0x93,
        EQ15           = 0x94,
        EQ16           = 0x95,
        EQ17           = 0x96,
        EQ18           = 0x97,
        EQ19           = 0x98,
        EQ20           = 0x99,
        EQ21           = 0x9A,
        EQ22           = 0x9B,
        EQ23           = 0x9C,
        EQ24           = 0x9D,
        ADC_TEST0      = 0xC6,
        FLL_NCO_TEST0  = 0xF7,
        FLL_NCO_TEST1  = 0xF8
    };
    struct cmd_t {
        RegAddr_t   reg;
        uint16_t    val;
        uint16_t    dly;
        cmd_t & operator=(const cmd_t rhs)
        {
            reg = rhs.reg;
            val = rhs.val;
            dly = rhs.dly;
            return *this;
        }
    };
    static cmd_t init_cmds_0[];
    static cmd_t init_cmds_1[];
    static cmd_t init_cmds_2[];
    static cmd_t init_cmds_3[];
    static cmd_t init_cmds_4[];
    static cmd_t init_cmds_5[];
    static cmd_t init_cmds_6[];
    };

    enum DataLen_t {
        DATA_LEN_16,
        DATA_LEN_24,
        DATA_LEN_32
    };

    enum DataFmt_t {
        DATA_FMT_16_L_JUSTIFIED,
        DATA_FMT_16_I2S,
        DATA_FMT_32_L_JUSTIFIED,
        DATA_FMT_32_I2S
    };

    enum AudioChSelect_t {
        CH_SELECT_LEFT,
        CH_SELECT_RIGHT,
        CH_SELECT_LEFT_RIGHT
    };

    enum AudioOutSelect_t {
        AUDIO_OUT_NONE,
        AUDIO_OUT_HP,
        AUDIO_OUT_LINE,
        AUDIO_OUT_SPKR
    };

    enum AudioInSelect_t {
        AUDIO_IN_NONE,
        AUDIO_IN_1,
        AUDIO_IN_2,
        AUDIO_IN_3,
    };

    struct cfg_t {
        uint16_t sampleRate;
        uint32_t refClkRate;
        DataFmt_t dataFormat;
        AudioChSelect_t inCh;
        AudioChSelect_t outCh;
        AudioInSelect_t inSrc;
        AudioOutSelect_t outSrc;

        uint8_t initMicGain;
        uint8_t initOutVol;
    };

private:
    I2C &twi;

    void InitInput(const cfg_t &cfg);
    void ConfigFLL(const cfg_t &cfg);
public:
    /**
     * @brief The constructor for the WM8904 context.
     * @param module A reference to the I2C module to be used for configuring
     *  the codec.
     * */
    WM8904(I2C &module);
    /**
     * @brief Configures and initializes both the driver and the codec.
     * @param cfg The driver configuration to use.
     * @param ssc_cfg The SSC driver configuration to use.
     */
    void Init(const cfg_t &cfg, const SSC_cfg_t &ssc_cfg);
    /**
     * @brief Shuts down the WM8904 codec driver.
     */
    void Shutdown();
    /**
     * @brief Write a register on the WM8904.
     * @param reg Register to write.
     * @param dat Data to write.
     */
    void WriteReg(Reg::RegAddr_t reg, uint16_t dat);
    /**
     * @brief Read a register on the WM8904.
     * @param reg Register to write.
     * @return Data read.
     */
    uint16_t ReadReg(Reg::RegAddr_t reg);

    /**
     * @brief Execute codec Command. A command is: a register to write,
     *  a value to write, and a delay of time required for command execution.
     * @param cmd Command to execute
     */
    void SendCmd(Reg::cmd_t cmd);
    /**
     * @brief Execute an array of codec Commands. A command is: a register to
     *  write, a value to write, and a delay of time required for
     *  command execution.
     * @param cmds Commands to execute
     * @param len Number of commands to execute.
     */
    void SendCmdList(Reg::cmd_t *cmds, uint32_t len);
    /**
     * @brief Execute a codec Command to update a register. A command is: a
     *  register to write, a value to write, and a delay of time required for
     *  command execution.
     * @param cmd Command to execute
     * @param updateMask A Positive bit mask of bits to update in the target
     *  register
     */
    void UpdateCmd(Reg::cmd_t cmd, uint16_t updateMask);

    /**
     * @brief Set the volume of the selected audio output and channel.
     * @param out Audio Output to select.
     * @param channel Output Channel to select.
     * @param volume Volume to set channel to.
     */
    void SetVolume(AudioOutSelect_t out, AudioChSelect_t channel, uint8_t volume);
    /**
     * @brief Get the volume of the selected audio output and channel.
     * @param out Audio Output to select.
     * @param channel Output Channel to select.
     * @return Volume the channel is set to.
     */
    uint8_t GetVolume(AudioOutSelect_t out, AudioChSelect_t channel);
    /**
     * @brief Mute or unmute the selected audio output and channel.
     * @param out Audio Output to select.
     * @param channel Output Channel to select.
     * @param mute Whether the channel is to be muted.
     */
    void Mute(AudioOutSelect_t out, AudioChSelect_t channel, bool mute);

    /**
     * @brief Set the microphone gain of the selected input channel.
     * @param channel Input Channel to select.
     * @param gain Gain to set channel to.
     */
    void SetMicGain(AudioChSelect_t channel, uint8_t gain);
    /**
     * @brief Get the microphone gain of the selected input channel.
     * @param channel Input Channel to select.
     * @return Gain the channel is set to.
     */
    uint8_t GetMicGain(AudioChSelect_t channel);
    /**
     * @brief Mute or unmute the selected input channel.
     * @param channel Input Channel to select.
     * @param mute Whether the channel is to be muted.
     */
    void MuteMic(AudioChSelect_t channel, bool mute);

    /**
     * @brief Hands off a buffer to be transmitted to the codec.
     *
     * @param buffer A pointer to the buffer to be transmit.
     * @param bufferLen The length of the buffer to be transmit.
     *  (Must be multiples of 1, 2, or 4 bytes depending on word bit width)
     * @param waitIfNeeded Whether the driver should wait for space to transmit
     *  or fail immediately upon exhausting the queue depth.
     *
     * @return Negative on failure.
     */
    int TransmitBuffer(void *buffer, uint32_t bufferLen, bool waitIfNeeded);
    /**
     * @brief Hands off a buffer to be written to by the codec.
     *
     * @param buffer A pointer to the buffer to be written to.
     * @param bufferLen The length of the buffer to be written.
     *  (Must be multiples of 1, 2, or 4 bytes depending on word bit width)
     * @param waitIfNeeded Whether the driver should wait for space to receive
     *  or fail immediately upon exhausting the queue depth.
     *
     * @return Negative on failure.
     */
    int ReadyReceiveBuffer(void *buffer, uint32_t bufferLen, bool waitIfNeeded);

    /**
     * @brief Registers a callback for when a transmit buffer is finished.
     * */
    void RegisterTxBufferDoneCB(SSC_BufferDoneFn_t cb);
    /**
     * @brief Registers a callback for when a receive buffer is finished.
     * */
    void RegisterRxBufferDoneCB(SSC_BufferDoneFn_t cb);

};
#endif   /* ----- #ifndef __WM8904_H  ----- */
