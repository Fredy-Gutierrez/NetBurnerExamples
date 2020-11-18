#ifndef  __SSC_I2S_H
#define  __SSC_I2S_H
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
#include <sim.h>           /*on-chip register definitions*/
#include <pins.h>
#include <nbrtos.h>
#include <xdmac.h>

//class SSC_I2S : private Ssc {
//
//
//public:
//    inline void Reset() { SSC_CR = SSC_CR_SWRST; }
//    inline void EnableTx(bool enable) {
//        SSC_CR = (enable ? SSC_CR_TXEN : SSC_CR_TXDIS);
//    }
//    inline void EnableRx(bool enable) {
//        SSC_CR = (enable ? SSC_CR_RXEN : SSC_CR_RXDIS);
//    }
//    inline uint16_t SetClkDiv(uint16_t div) {
//        return SSC_CMR = div & SSC_CMR_DIV_Msk;
//    }
//
//    static Ssc *get() { return reinterpret_cast<SSC_I2S*>(SSC); }
//    Ssc &GetRegisers() { return *this; }
//
//} __attribute__((packed));
//
//static_assert( sizeof (SSC_I2S == sizeof(Ssc) );
//

typedef void (*SSC_BufferDoneFn_t)(void *buffer, bool valid);
/****************************************/
/*      SSC Configuration Enums         */
/****************************************/
/**
 * @enum clkSrc_t The clock source to use for transactions.
 */
enum clkSrc_t {
    CLK_SRC_MCK,    /**< Clock source is the divided Peripheral Master clock. */
    CLK_SRC_RK,     /**< Clock source is the Receive Clock. */
    CLK_SRC_TK      /**< Clock source is the Transmit Clock. */
};

/**
 * @enum startCond_t Starting conditions for SSC transfers.
 *
 * @var startCond_t START_CONTINUOUS
 *  SSC transfers start as soon as the previous transfer ends.
 * @var startCond_t START_SYNC_RX_TX
 *  SSC transfers start syncronously when the opposite mode starts.
 * @var startCond_t START_FRAME_LOW
 *  SSC transfers start when the frame sync signal is low.
 * @var startCond_t START_FRAME_HIGH
 *  SSC transfers start when the frame sync signal is high.
 * @var startCond_t START_FRAME_FALLING
 *  SSC transfers start on a frame sync falling edge.
 * @var startCond_t START_FRAME_RISING
 *  SSC transfers start on a frame sync rising edge.
 * @var startCond_t START_FRAME_LEVEL
 *  SSC transfers start on a frame sync level change.
 * @var startCond_t START_FRAME_EDGE
 *  SSC transfers start on a frame sync edge (positive or negative).
 * @var startCond_t START_CMP_0
 *  SSC transfers start after Rx receives a transfer matching the value
 *  in the Compare 0 register. (Rx channel only)
 */
enum startCond_t {
    START_CONTINUOUS,
    START_SYNC_RX_TX,
    START_FRAME_LOW,
    START_FRAME_HIGH,
    START_FRAME_FALLING,
    START_FRAME_RISING,
    START_FRAME_LEVEL,
    START_FRAME_EDGE,
    START_CMP_0
};

/**
 * @enum clkGate_t Clock gating mode.
 */
enum clkGate_t {
    CLK_GATE_CONTINUOUS,/**< Clock is continously enabled. */
    CLK_GATE_FRAME_LOW, /**< Clock enabled only when the frame sync is: low.*/
    CLK_GATE_FRAME_HIGH,/**< Clock enabled only when the frame sync is: high.*/
};

/**
 * @enum dataValid_t Which clock edge data is valid on.
 */
enum dataValid_t {
    DATA_VALID_RISING,  /**< Bus data is valid at the rising edge of the clock. */
    DATA_VALID_FALLING, /**< Bus data is valid at the falling edge of the clock. */
};

/**
 * @enum frameEdge_t Which edge of the frame sync signal begins a new frame
 */
enum frameEdge_t {
    FRAME_SYNC_RISING,  /**< Frame begins on a rising edge of frame sync. */
    FRAME_SYNC_FALLING  /**< Frame begins on a falling edge of frame sync. */
};

/**
 * @enum clkOut_t Clock output mode.
 */
enum clkOut_t {
    CLK_OUT_INPUT,      /**< Clock signal is an input,
                                and its output is disabled. */
    CLK_OUT_CONTINUOUS, /**< Clock signal is continuously output */
    CLK_OUT_TRANSFER    /**< Clock signal is only output during a transfer.*/
};

/**
 * @enum frameSyncOut_t Frame sync signal output mode
 */
enum frameSyncOut_t {
    FRAME_SYNC_INPUT,       /**< Sync signal is an input,
                                    and its output is disabled. */
    FRAME_SYNC_NEGATIVE,    /**< A negative pulse is sent at the
                                    start of a frame. */
    FRAME_SYNC_POSITIVE,    /**< A positive pulse is sent at the
                                    start of a frame. */
    FRAME_SYNC_LOW,         /**< Sync signal is driven low during the frame. */
    FRAME_SYNC_HIGH,        /**< Sync signal is driven high during the frame. */
    FRAME_SYNC_TOGGLE       /**< Output state is toggled between frames. */
};

/**
 * @enum bitOrder_t Transfer bit order
 */
enum bitOrder_t {
    LEAST_SIG_FIRST,/**< Frames are transmitted least significant bit first. */
    MOST_SIG_FIRST  /**< Frames are transmitted most significant bit first. */
};

/**
 * @enum bufferDepletionBehavior_t What should DMA channel do upon buffer
 *  depletion
 *
 * @var bufferDepletionBehavior_t DEPLETED_PAUSE
 *  When buffers are depleted, the DMA channel should pause transactions.
 * @var bufferDepletionBehavior_t DEPLETED_REPEAT_LAST
 *  When buffers are depleted, the DMA channel repeat using the last buffer in
 *  the chain.
 */
enum bufferDepletionBehavior_t {
    DEPLETED_PAUSE,
    DEPLETED_REPEAT_LAST,
};

/****************************************/
/*      SSC Configuration Struct        */
/****************************************/

/**
 * @struct SSC_rxtx_cfg_t
 * @brief Configuration structure for a given direction (rx or tx) of the SSC
 *  module. Passed to the initialize function to configure the hardware.
 *
 * @var SSC_rxtx_cfg_t::enable
 *  Whether this direction should be enabled.
 * @var SSC_rxtx_cfg_t::period
 *  The period of each frame, in terms of the bit clock.
 * @var SSC_rxtx_cfg_t::startDly
 *  How many bit periods should data be delayed by at the start of the frame.
 * @var SSC_rxtx_cfg_t::startCond
 *  The starting condition for this channel.
 * @var SSC_rxtx_cfg_t::clkGate
 *  The clock gating condition for this channel.
 * @var SSC_rxtx_cfg_t::dataValid
 *  The data valid condition for this channel.
 * @var SSC_rxtx_cfg_t::clkOut
 *  The clock out condition for this channel.
 * @var SSC_rxtx_cfg_t::clkSrc
 *  The clock source for this channel.
 *
 * @var SSC_rxtx_cfg_t::syncLen
 *  The bit length of the synchronization data (when used).
 * @var SSC_rxtx_cfg_t::syncEdge
 *  The frame sync edge defining frame start.
 * @var SSC_rxtx_cfg_t::syncDataEnabled
 *  Whether synchronizing data is used.
 * @var SSC_rxtx_cfg_t::syncOut
 *  The frame sync output configuration.
 * @var SSC_rxtx_cfg_t::wordsPerFrame
 *  Number of words contained in a frame.
 * @var SSC_rxtx_cfg_t::bitOrder
 *  The bit order of transations.
 * @var SSC_rxtx_cfg_t::lineIdleState
 *  The state the line should be driven to when not transmitting a word.
 * @var SSC_rxtx_cfg_t::bitsPerWord
 *  The bit width of words to be transfered.
 *
 * @var SSC_rxtx_cfg_t::depletionBehavior
 *  The depletion behavior that should be used for driver DMA transfers.
 */
struct SSC_rxtx_cfg_t {
    bool        enable{false};
    uint8_t     period{64};
    uint8_t     startDly{1};
    startCond_t startCond{START_FRAME_EDGE};
    clkGate_t   clkGate{CLK_GATE_CONTINUOUS};
    dataValid_t dataValid{DATA_VALID_RISING};
    clkOut_t    clkOut{CLK_OUT_INPUT};
    clkSrc_t    clkSrc{CLK_SRC_RK};

    uint8_t     syncLen{0};
    frameEdge_t syncEdge{FRAME_SYNC_FALLING};
    bool        syncDataEnabled{false};
    frameSyncOut_t syncOut{FRAME_SYNC_INPUT};
    uint8_t     wordsPerFrame{1};
    bitOrder_t  bitOrder{MOST_SIG_FIRST};
    bool        lineIdleState{1};
    uint8_t     bitsPerWord{24};

    bufferDepletionBehavior_t   depletionBehavior{DEPLETED_PAUSE};
};

/**
 * @struct SSC_cfg_t
 * @brief Configuration structure for the SSC driver.
 *  Passed to the initialize function to configure the hardware.
 *
 * @var SSC_cfg_t::clkDiv
 *  The divider value for the peripheral clock used to generate the bit clock
 *  when CLK_SRC_MCK is selected.
 * @var SSC_cfg_t::rx
 *  The receive configuration.
 * @var SSC_cfg_t::tx
 *  The transmit configuration.
 */
struct SSC_cfg_t {
    uint16_t    clkDiv{150};

    SSC_rxtx_cfg_t rx;
    SSC_rxtx_cfg_t tx;
};

/**
 * @var SSC_cfg_t SSC_I2S_SLAVE_24_TXRX_RK
 *  SSC configuration: I2S Slave, 24 bit word, TX+RX, BClk: RK, Frame: RF
 * @var SSC_cfg_t SSC_I2S_SLAVE_24_TXRX_TK
 *  SSC configuration: I2S Slave, 24 bit word, TX+RX, BClk: TK, Frame: TF
 * @var SSC_cfg_t SSC_I2S_SLAVE_24_TXRX_TKRK
 *  SSC configuration: I2S Slave, 24 bit word, TX+RX, BClk: TK+RK, Frame: TF+RF
 * @var SSC_cfg_t SSC_I2S_SLAVE_16_TXRX_RK
 *  SSC configuration: I2S Slave, 16 bit word, TX+RX, BClk: RK, Frame: RF
 * @var SSC_cfg_t SSC_I2S_SLAVE_16_TXRX_TK
 *  SSC configuration: I2S Slave, 16 bit word, TX+RX, BClk: TK, Frame: TF
 * @var SSC_cfg_t SSC_I2S_SLAVE_16_TXRX_TKRK
 *  SSC configuration: I2S Slave, 16 bit word, TX+RX, BClk: TK+RK, Frame: TF+RF
 * @var SSC_cfg_t SSC_I2S_SLAVE_8_TXRX_RK
 *  SSC configuration: I2S Slave, 8 bit word, TX+RX, BClk: RK, Frame: RF
 * @var SSC_cfg_t SSC_I2S_SLAVE_8_TXRX_TK
 *  SSC configuration: I2S Slave, 8 bit word, TX+RX, BClk: TK, Frame: TF
 * @var SSC_cfg_t SSC_I2S_SLAVE_8_TXRX_TKRK
 *  SSC configuration: I2S Slave, 8 bit word, TX+RX, BClk: TK+RK, Frame: TF+RF

 * @var SSC_cfg_t SSC_LJUST_SLAVE_24_TXRX_RK
 *  SSC configuration: Left Justified Slave, 24 bit word, TX+RX, BClk: RK, Frame: RF
 * @var SSC_cfg_t SSC_LJUST_SLAVE_24_TXRX_TK
 *  SSC configuration: Left Justified Slave, 24 bit word, TX+RX, BClk: TK, Frame: TF
 * @var SSC_cfg_t SSC_LJUST_SLAVE_24_TXRX_TKRK
 *  SSC configuration: Left Justified Slave, 24 bit word, TX+RX, BClk: TK+RK, Frame: TF+RF
 * @var SSC_cfg_t SSC_LJUST_SLAVE_16_TXRX_RK
 *  SSC configuration: Left Justified Slave, 16 bit word, TX+RX, BClk: RK, Frame: RF
 * @var SSC_cfg_t SSC_LJUST_SLAVE_16_TXRX_TK
 *  SSC configuration: Left Justified Slave, 16 bit word, TX+RX, BClk: TK, Frame: TF
 * @var SSC_cfg_t SSC_LJUST_SLAVE_16_TXRX_TKRK
 *  SSC configuration: Left Justified Slave, 16 bit word, TX+RX, BClk: TK+RK, Frame: TF+RF
 * @var SSC_cfg_t SSC_LJUST_SLAVE_8_TXRX_RK
 *  SSC configuration: Left Justified Slave, 8 bit word, TX+RX, BClk: RK, Frame: RF
 * @var SSC_cfg_t SSC_LJUST_SLAVE_8_TXRX_TK
 *  SSC configuration: Left Justified Slave, 8 bit word, TX+RX, BClk: TK, Frame: TF
 * @var SSC_cfg_t SSC_LJUST_SLAVE_8_TXRX_TKRK
 *  SSC configuration: Left Justified Slave, 8 bit word, TX+RX, BClk: TK+RK, Frame: TF+RF
 */

extern const SSC_cfg_t SSC_I2S_SLAVE_24_TXRX_RK;
extern const SSC_cfg_t SSC_I2S_SLAVE_24_TXRX_TK;
extern const SSC_cfg_t SSC_I2S_SLAVE_24_TXRX_TKRK;
extern const SSC_cfg_t SSC_I2S_SLAVE_16_TXRX_RK;
extern const SSC_cfg_t SSC_I2S_SLAVE_16_TXRX_TK;
extern const SSC_cfg_t SSC_I2S_SLAVE_16_TXRX_TKRK;
extern const SSC_cfg_t SSC_I2S_SLAVE_8_TXRX_RK;
extern const SSC_cfg_t SSC_I2S_SLAVE_8_TXRX_TK;
extern const SSC_cfg_t SSC_I2S_SLAVE_8_TXRX_TKRK;

extern const SSC_cfg_t SSC_LJUST_SLAVE_24_TXRX_RK;
extern const SSC_cfg_t SSC_LJUST_SLAVE_24_TXRX_TK;
extern const SSC_cfg_t SSC_LJUST_SLAVE_24_TXRX_TKRK;
extern const SSC_cfg_t SSC_LJUST_SLAVE_16_TXRX_RK;
extern const SSC_cfg_t SSC_LJUST_SLAVE_16_TXRX_TK;
extern const SSC_cfg_t SSC_LJUST_SLAVE_16_TXRX_TKRK;
extern const SSC_cfg_t SSC_LJUST_SLAVE_8_TXRX_RK;
extern const SSC_cfg_t SSC_LJUST_SLAVE_8_TXRX_TK;
extern const SSC_cfg_t SSC_LJUST_SLAVE_8_TXRX_TKRK;



/****************************************/
/*      SSC Driver Context Object       */
/****************************************/

/**
 * @class SSCCtx_t
 * SSCtcx_t is a driver for the Synchronous Serial Controller. It operates on
 *  a buffer level, where buffers are handed to the system and either transmitted
 *  or filled with received data as appropriate.
 */
class SSCCtx_t {
public:
    enum ctxState_t {
        CTX_STATE_STOPPED,
        CTX_STATE_INIT,
        CTX_STATE_TX,
        CTX_STATE_RX,
        CTX_STATE_TXRX,
    };

private:
    XdmaCh_t * txCh;
    XdmaCh_t * rxCh;
    OS_SEM   txSem;
    OS_SEM   rxSem;
    SSC_BufferDoneFn_t txBufDone;
    SSC_BufferDoneFn_t rxBufDone;
    uint32_t    txReadyCount;
    uint32_t    rxReadyCount;
    uint32_t    nTxIrq;
    uint32_t    nTxErr;
    uint32_t    nTxPost;
    uint32_t    nTxCall;
    uint32_t    nTxStart;
    uint32_t    nTxAdd;

    uint32_t    nRxIrq;
    uint32_t    nRxErr;
    uint32_t    nRxPost;
    uint32_t    nRxCall;
    uint32_t    nRxStart;
    uint32_t    nRxAdd;

    Desc0Ring       TxRing;
    Desc0Ring       RxRing;
    uint8_t     rxByteWidth;
    uint8_t     txByteWidth;

    ctxState_t state;
    void initBDs(bool loopRx, bool loopTx);
    void initHw_Pins(const SSC_cfg_t &cfg);
    void initHw_SSC(const SSC_cfg_t &cfg);
    void initHw_DMA(const SSC_cfg_t &cfg);
public:

    /**
     *  @brief Initializes the SSC hardware and driver context.
     *  @param[in] cfg The configuration to use.
     *
     *  @return Negative on failure.
     * */
    int Init(const SSC_cfg_t &cfg);
    /**
     *  @brief Shuts down the SSC hardware and driver.
     */
    void Shutdown();

    /**
     * @brief Fills in the config object with the current active configuration.
     * @param[out] cfg The configuration object to populate.
     *
     * @return Negative on failure.
     */
    int getCurrentConfig(SSC_cfg_t &cfg);
    /**
     * @brief Returns the current driver state.
     * @return The current driver state.
     */
    ctxState_t getState();


    /**
     * @brief Hands off a buffer to be transmitted by the SSC driver.
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
     * @brief Hands off a buffer to be written to by the SSC driver.
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

    void TxIsr();
    void RxIsr();

    void dump();
};

extern SSCCtx_t ssc;

#endif   /* ----- #ifndef __SSC_I2S_H  ----- */
