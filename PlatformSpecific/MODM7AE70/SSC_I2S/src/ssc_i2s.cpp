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
#include <sim.h>           /*on-chip register definitions*/
#include <pins.h>
#include <nbrtos.h>
#include <cpu_hal.h>
#include <xdmac.h>
#include "ssc_i2s.h"
#include <pins.h>

#define SSC_BUFFER_DEPTH (3)
#define SSC_BUFFER_LENGTH (ETHER_BUFFER_SIZE-70)
#define I2S_CHECK_INTERVAL (2*TICKS_PER_SECOND)

#define LOGME iprintf("L: %d - F: %s\n", __LINE__, __FILE__);

#define PRINT_REG(struc, reg, fmt) iprintf(#struc"_"#reg":  " fmt "\n", (struc)->reg)

void dumpRegs(XdmaCh_t *txCh)
{
    iprintf("SPI Regs:\n");
    PRINT_REG(SPI0, SPI_CR, "%08lx");
    PRINT_REG(SPI0, SPI_MR, "%08lx");
    PRINT_REG(SPI0, SPI_SR, "%08lx");
    PRINT_REG(SPI0, SPI_IMR, "%08lx");
    PRINT_REG(SPI0, SPI_CSR[2], "%08lx");

    iprintf("DMA: \n");
    iprintf("txCh ID: %d\n", txCh->getID());
    PRINT_REG(XDMAC, XDMAC_GS, "%08lx");
    PRINT_REG(XDMAC, XDMAC_GIM, "%08lx");
    PRINT_REG(XDMAC, XDMAC_GIS, "%08lx");
    PRINT_REG(XDMAC, XDMAC_GRS, "%08lx");
    PRINT_REG(XDMAC, XDMAC_GWS, "%08lx");

    iprintf("\nTX DMA:\n");
    PRINT_REG(txCh, XDMAC_CC, "%08lx");
    PRINT_REG(txCh, XDMAC_CIM, "%08lx");
    PRINT_REG(txCh, XDMAC_CIS, "%08lx");
    PRINT_REG(txCh, XDMAC_CSA, "%08lx");
    PRINT_REG(txCh, XDMAC_CDA, "%08lx");
    PRINT_REG(txCh, XDMAC_CBC, "%08lx");
    PRINT_REG(txCh, XDMAC_CUBC, "%08lx");
    PRINT_REG(txCh, XDMAC_CNDA, "%08lx");
    PRINT_REG(txCh, XDMAC_CNDC, "%08lx");
}

/****************************************/
/*  Standard Predefined Configurations  */
/****************************************/

#define SSC_CFG_SLAVE(name, rxClk, txClk, rxStart, txStart, dly, bits) extern const SSC_cfg_t name;\
const SSC_cfg_t name{\
68,\
{true, 64, dly, rxStart, CLK_GATE_CONTINUOUS, DATA_VALID_RISING,\
    CLK_OUT_INPUT, rxClk,\
    0, FRAME_SYNC_FALLING, false, FRAME_SYNC_INPUT, 1, MOST_SIG_FIRST, 0, bits,\
    DEPLETED_REPEAT_LAST},\
{true, 64, dly, txStart, CLK_GATE_CONTINUOUS, DATA_VALID_RISING,\
    CLK_OUT_INPUT, txClk,\
    0, FRAME_SYNC_FALLING, false, FRAME_SYNC_INPUT, 1, MOST_SIG_FIRST, 0, bits,\
    DEPLETED_REPEAT_LAST}\
};


#define _ssc_xstr(x) _ssc_str(x)
#define _ssc_str(x) #x
#define SSC_CFG_SLAVE_BIT_BLOCK(name, dly, bits) SSC_CFG_SLAVE(name ## _ ## bits ## _TXRX_RK, \
        CLK_SRC_RK, CLK_SRC_RK, START_FRAME_EDGE, START_SYNC_RX_TX, dly, bits \
        ) \
\
SSC_CFG_SLAVE(name ## _ ## bits ## _TXRX_TK, \
        CLK_SRC_TK, CLK_SRC_TK, START_SYNC_RX_TX, START_FRAME_EDGE, dly, bits \
        ) \
\
SSC_CFG_SLAVE(name ## _ ## bits ## _TXRX_TKRK, \
        CLK_SRC_RK, CLK_SRC_TK, START_FRAME_EDGE, START_FRAME_EDGE, dly, bits \
        )

/****************************************/
/*  I2S Slave Configurations            */
/****************************************/
/*  I2S Slave: 24 Bit   */
//  I2S Slave: 24 Bit, Rx clock, RF sync
//      SSC_I2S_SLAVE_24_TXRX_RK
//  I2S Slave: 24 Bit, Tx clock, TF sync
//      SSC_I2S_SLAVE_24_TXRX_TK
//  I2S Slave: 24 Bit, Rx+Tx clock, RF+TF sync
//      SSC_I2S_SLAVE_24_TXRX_TKRK
SSC_CFG_SLAVE_BIT_BLOCK(SSC_I2S_SLAVE, 1, 24)

/*  I2S Slave: 16 Bit   */
//  I2S Slave: 16 Bit, Rx clock, RF sync
//      SSC_I2S_SLAVE_16_TXRX_RK
//  I2S Slave: 16 Bit, Tx clock, TF sync
//      SSC_I2S_SLAVE_16_TXRX_TK
//  I2S Slave: 16 Bit, Rx+Tx clock, RF+TF sync
//      SSC_I2S_SLAVE_16_TXRX_TKRK
SSC_CFG_SLAVE_BIT_BLOCK(SSC_I2S_SLAVE, 1, 16)

/*  I2S Slave: 8 Bit   */
//  I2S Slave: 8 Bit, Rx clock, RF sync
//      SSC_I2S_SLAVE_8_TXRX_RK,
//  I2S Slave: 8 Bit, Tx clock, TF sync
//      SSC_I2S_SLAVE_8_TXRX_TK,
//  I2S Slave: 8 Bit, Rx+Tx clock, RF+TF sync
//      SSC_I2S_SLAVE_8_TXRX_TKRK,
SSC_CFG_SLAVE_BIT_BLOCK(SSC_I2S_SLAVE, 1, 8)

/********************************************/
/*  I2S Slave Configurations - Justified    */
/********************************************/
/*  I2S Slave: 24 Bit, Justified   */
//  I2S Slave: 24 Bit, Rx clock, RF sync, Justified
//      SSC_LJUST_SLAVE_24_TXRX_RK
//  I2S Slave: 24 Bit, Tx clock, TF sync, Justified
//      SSC_LJUST_SLAVE_24_TXRX_TK
//  I2S Slave: 24 Bit, Rx+Tx clock, RF+TF sync, Justified
//      SSC_LJUST_SLAVE_24_TXRX_TKRK
SSC_CFG_SLAVE_BIT_BLOCK(SSC_LJUST_SLAVE, 0, 24)

/*  I2S Slave: 16 Bit, Justified   */
//  I2S Slave: 16 Bit, Rx clock, RF sync, Justified
//      SSC_LJUST_SLAVE_16_TXRX_RK
//  I2S Slave: 16 Bit, Tx clock, TF sync, Justified
//      SSC_LJUST_SLAVE_16_TXRX_TK
//  I2S Slave: 16 Bit, Rx+Tx clock, RF+TF sync, Justified
//      SSC_LJUST_SLAVE_16_TXRX_TKRK
SSC_CFG_SLAVE_BIT_BLOCK(SSC_LJUST_SLAVE, 0, 16)

/*  I2S Slave: 8 Bit, Justified   */
//  I2S Slave: 8 Bit, Rx clock, RF sync, Justified
//      SSC_LJUST_SLAVE_8_TXRX_RK,
//  I2S Slave: 8 Bit, Tx clock, TF sync, Justified
//      SSC_LJUST_SLAVE_8_TXRX_TK,
//  I2S Slave: 8 Bit, Rx+Tx clock, RF+TF sync, Justified
//      SSC_LJUST_SLAVE_8_TXRX_TKRK,
SSC_CFG_SLAVE_BIT_BLOCK(SSC_LJUST_SLAVE, 0, 8)


static dma_descview_0   sscTxBD[SSC_BUFFER_DEPTH] DO_NOT_CACHE;
static dma_descview_0   sscRxBD[SSC_BUFFER_DEPTH] DO_NOT_CACHE;
static uint32_t         sscTxBDFlag[SSC_BUFFER_DEPTH];
static uint32_t         sscRxBDFlag[SSC_BUFFER_DEPTH];


void SSC_DMA_TxIsr(XdmaCh_t *ch);
void SSC_DMA_RxIsr(XdmaCh_t *ch);

SSCCtx_t ssc;

int InitSSC(const SSC_cfg_t &cfg)
{
//    P1[5].PullUp(true);
//    P2[4].function(PINP2_4_TK);
//    P2[3].function(PINP2_3_TF);

    P2[17].function(PINP2_17_OUT);
    P2[18].function(PINP2_18_OUT);
    P2[19].function(PINP2_19_OUT);
    P2[20].function(PINP2_20_OUT);


    return ssc.Init(cfg);
}

void SSCCtx_t::initHw_Pins(const SSC_cfg_t &cfg)
{

    if (cfg.rx.enable)
    {
        if ((cfg.rx.clkSrc == CLK_SRC_RK) || (cfg.rx.clkOut != CLK_OUT_INPUT))
        {
            P1[5].function(PINP1_5_RK);
        }
        if ((cfg.rx.syncOut != FRAME_SYNC_INPUT)
                || ( (cfg.rx.startCond != START_CONTINUOUS)
                    && (cfg.rx.startCond != START_SYNC_RX_TX)
                    && (cfg.rx.startCond != START_CMP_0) ))
        {
            P2[15].function(PINP2_15_RF);
        }
        P2[35].function(PINP2_35_RD);
    }
    if (cfg.tx.enable)
    {
        if ((cfg.tx.clkSrc == CLK_SRC_TK) || (cfg.tx.clkOut != CLK_OUT_INPUT))
        {
            P2[4].function(PINP2_4_TK);
        }
        if ((cfg.rx.syncOut != FRAME_SYNC_INPUT)
                || ( (cfg.rx.startCond != START_CONTINUOUS)
                    && (cfg.rx.startCond != START_SYNC_RX_TX) ))
        {
            P2[3].function(PINP2_3_TF);
        }
        P2[44].function(PINP2_44_TD);
    }

    asm volatile ("dsb");
}


void SSCCtx_t::initBDs(bool loopRx, bool loopTx)
{
    TxRing.xdmaCh = txCh;
    TxRing.descs = sscTxBD;
    TxRing.BDFlag = sscTxBDFlag;
    TxRing.ringLen = SSC_BUFFER_DEPTH;
    TxRing.spinLength = 0;
    TxRing.spinBuf = NULL;
    TxRing.ubCtrlBits = 0x3;
    // If setting spinOnLastNotNext to false, there must always be one
    //  descriptor *not* in use by the requested transfers such that the driver
    //  can use it to loop
    TxRing.spinOnLastNotNext = true;


    RxRing.xdmaCh = rxCh;
    RxRing.descs = sscRxBD;
    RxRing.BDFlag = sscRxBDFlag;
    RxRing.ringLen = SSC_BUFFER_DEPTH;
    RxRing.spinLength = 0;
    RxRing.spinBuf = NULL;
    RxRing.ubCtrlBits = 0x5;
    // If setting spinOnLastNotNext to false, there must always be one
    //  descriptor *not* in use by the requested transfers such that the driver
    //  can use it to loop
    RxRing.spinOnLastNotNext = true;
}

void SSCCtx_t::initHw_SSC(const SSC_cfg_t &cfg)
{
    uint32_t rxClkSrc = 0;
    uint32_t txClkSrc = 0;
    switch (cfg.rx.clkSrc) {
    case CLK_SRC_MCK:   rxClkSrc = SSC_RCMR_CKS_MCK; break;
    case CLK_SRC_RK:    rxClkSrc = SSC_RCMR_CKS_RK; break;
    case CLK_SRC_TK:    rxClkSrc = SSC_RCMR_CKS_TK; break;
    }
    switch (cfg.tx.clkSrc) {
    case CLK_SRC_MCK:   txClkSrc = SSC_TCMR_CKS_MCK; break;
    case CLK_SRC_RK:    txClkSrc = SSC_TCMR_CKS_RK; break;
    case CLK_SRC_TK:    txClkSrc = SSC_TCMR_CKS_TK; break;
    }

    SSC->SSC_CR = SSC_CR_SWRST;
    asm volatile ("dsb");
    SSC->SSC_CMR = cfg.clkDiv; // 150e6/(44100*(24+1))/2 = 68.027
    SSC->SSC_RCMR = rxClkSrc
                    | (cfg.rx.clkOut << SSC_RCMR_CKO_Pos)
                    | (cfg.rx.clkGate << SSC_RCMR_CKG_Pos)
                    | (cfg.rx.startCond << SSC_RCMR_START_Pos)
                    | ((cfg.rx.dataValid == DATA_VALID_RISING) ?SSC_RCMR_CKI :0)
                    | SSC_RCMR_PERIOD(cfg.rx.period)
                    | SSC_RCMR_STTDLY(cfg.rx.startDly);

    SSC->SSC_RFMR = SSC_RFMR_DATLEN(cfg.rx.bitsPerWord-1)
                    | SSC_RFMR_DATNB(cfg.rx.wordsPerFrame-1)
                    | (cfg.rx.bitOrder ? SSC_RFMR_MSBF : 0)
                    | (cfg.rx.syncOut << SSC_RFMR_FSOS_Pos)
                    | ((cfg.rx.syncEdge == FRAME_SYNC_FALLING)
                            ? SSC_RFMR_FSEDGE_NEGATIVE
                            : SSC_RFMR_FSEDGE_POSITIVE);
    SSC->SSC_TCMR = txClkSrc
                    | (cfg.tx.clkOut << SSC_TCMR_CKO_Pos)
                    | (cfg.tx.clkGate << SSC_TCMR_CKG_Pos)
                    | (cfg.tx.startCond << SSC_TCMR_START_Pos)
                    | ((cfg.tx.dataValid == DATA_VALID_RISING) ?0 :SSC_TCMR_CKI)
                    | SSC_TCMR_PERIOD(cfg.tx.period)
                    | SSC_TCMR_STTDLY(cfg.tx.startDly);

    SSC->SSC_TFMR = SSC_TFMR_DATLEN(cfg.tx.bitsPerWord-1)
                    | SSC_TFMR_DATNB(cfg.tx.wordsPerFrame-1)
                    | (cfg.tx.bitOrder ? SSC_TFMR_MSBF : 0)
                    | (cfg.tx.syncOut << SSC_TFMR_FSOS_Pos)
                    | ((cfg.tx.syncEdge == FRAME_SYNC_FALLING)
                            ? SSC_TFMR_FSEDGE_NEGATIVE
                            : SSC_TFMR_FSEDGE_POSITIVE);

//    SSC->SSC_TCMR = SSC_TCMR_CKS_RK | SSC_TCMR_CKO_NONE |
//                    SSC_TCMR_CKG_CONTINUOUS | SSC_TCMR_START_RECEIVE |
//                    SSC_TCMR_CKI |
//                    SSC_TCMR_STTDLY(1);
//    SSC->SSC_TFMR = SSC_TFMR_DATLEN(cfg.tx.bitsPerWord-1)
//                    | SSC_TFMR_DATNB(cfg.tx.wordsPerFrame-1)
//                    | (cfg.tx.bitOrder ? SSC_TFMR_MSBF : 0)
//                    | (cfg.tx.bitOrder ? SSC_TFMR_MSBF : 0)
//                    | SSC_TFMR_FSOS_NONE |
//                    SSC_TFMR_FSEDGE_NEGATIVE;
//

}

void SSCCtx_t::initHw_DMA(const SSC_cfg_t &cfg)
{
    XdmaCh_t::DWidth_t dmaWidth;

    txCh = xdmacGetFreeCh();
    rxCh = xdmacGetFreeCh();

    rxByteWidth = (cfg.rx.bitsPerWord > 16) ? 4 : (
                        (cfg.rx.bitsPerWord > 8) ? 2 : 1
                        );
    txByteWidth = (cfg.tx.bitsPerWord > 16) ? 4 : (
                        (cfg.tx.bitsPerWord > 8) ? 2 : 1
                        );
    dmaWidth = (txByteWidth > 2) ? XdmaCh_t::DWidth_Word : (
                (txByteWidth > 1) ? XdmaCh_t::DWidth_HalfWord :
                XdmaCh_t::DWidth_Byte );

    initBDs(cfg.rx.depletionBehavior, cfg.tx.depletionBehavior);

    txCh->disable();
    txCh->XDMAC_CID = 0x7F;
    txCh->XDMAC_CIE = XDMAC_CIE_RBIE | XDMAC_CIE_BIE;
    *(&txCh->XDMAC_CIS);
    txCh->s_BLen(0);

    txCh->sDstAddr((uint32_t)&(SSC->SSC_THR));
    txCh->sNextDescAddr(TxRing.descs+1);
    txCh->XDMAC_CNDC = XDMAC_CNDC_NDE | XDMAC_CNDC_NDVIEW(0);
    TxRing.xfrIncSiz = txByteWidth;

    txCh->sConfig(XdmaCh_t::Periph, XdmaCh_t::MBSiz_16, XdmaCh_t::Mem2Periph,
                XdmaCh_t::Req_HW, XdmaCh_t::ChunkSiz_1,
                dmaWidth,
                xdmacGetIntfForAddr((uint32_t)TxRing.descs[0].addr),
                xdmacGetIntfForAddr((uint32_t)&SSC->SSC_THR),
                XdmaCh_t::AddrMode_Inc,
                XdmaCh_t::AddrMode_Fixed,
                32 // SSC Tx DMA Periph Req ID
                );

    dmaWidth = (rxByteWidth > 2) ? XdmaCh_t::DWidth_Word : (
                (rxByteWidth > 1) ? XdmaCh_t::DWidth_HalfWord :
                XdmaCh_t::DWidth_Byte );

    rxCh->disable();
    rxCh->XDMAC_CID = 0x7F;
    rxCh->XDMAC_CIE = XDMAC_CIE_WBIE | XDMAC_CIE_BIE;
    *(&rxCh->XDMAC_CIS);
    rxCh->s_BLen(0);

    rxCh->sSrcAddr((uint32_t)&SSC->SSC_RHR);
    rxCh->sNextDescAddr(RxRing.descs+1);
    rxCh->XDMAC_CNDC = XDMAC_CNDC_NDE | XDMAC_CNDC_NDVIEW(0);
    RxRing.xfrIncSiz = rxByteWidth;

    rxCh->sConfig(XdmaCh_t::Periph, XdmaCh_t::MBSiz_16, XdmaCh_t::Periph2Mem,
                XdmaCh_t::Req_HW, XdmaCh_t::ChunkSiz_1,
                dmaWidth,
                xdmacGetIntfForAddr((uint32_t)&SSC->SSC_RHR),
                xdmacGetIntfForAddr((uint32_t)RxRing.descs[0].addr),
                XdmaCh_t::AddrMode_Fixed,
                XdmaCh_t::AddrMode_Inc,
                33 // SSC Rx DMA Periph Req ID
                );
}

int SSCCtx_t::Init(const SSC_cfg_t &cfg)
{
    if (cfg.tx.startCond == START_CMP_0) {
        return -1;
    }
    initHw_Pins(cfg);

    EnablePeriphClock(ID_SSC);

    initHw_SSC(cfg);
    initHw_DMA(cfg);

    for (int i = 0; i < SSC_BUFFER_DEPTH; i++) {
        txSem.Post();
        rxSem.Post();
    }

    state = CTX_STATE_INIT;
    txCh->RegisterIsr(SSC_DMA_TxIsr);
    rxCh->RegisterIsr(SSC_DMA_RxIsr);
    return 0;
}

void SSCCtx_t::Shutdown()
{
    txCh->disable();
    rxCh->disable();
    state = CTX_STATE_STOPPED;

    // Wait for Tx DMA channel to disable
    while (txCh->XDMAC_CIS & XDMAC_CIS_DIS_Msk) { }
    while (rxCh->XDMAC_CIS & XDMAC_CIS_DIS_Msk) { }

    for (int i = 0; i < SSC_BUFFER_DEPTH; i++) {
        if (txBufDone && (TxRing.BDFlag[i] & XDMA_BD_INUSE)) {
            TxRing.BDFlag[i] &= ~XDMA_BD_INUSE;
            txBufDone((void *)TxRing.descs[i].addr, false);
        }
        if (rxBufDone && (RxRing.BDFlag[i] & XDMA_BD_INUSE)) {
            RxRing.BDFlag[i] &= ~XDMA_BD_INUSE;
            rxBufDone((void *)RxRing.descs[i].addr, false);
        }
    }
    // Flush all tasks that might be waiting on the driver semaphores
    USER_ENTER_CRITICAL();
    while (txSem.PendNoWait() != OS_NO_ERR) {
        txSem.Post();
    }
    while (rxSem.PendNoWait() != OS_NO_ERR) {
        rxSem.Post();
    }
    USER_EXIT_CRITICAL();
}

int SSCCtx_t::TransmitBuffer(void *buf, uint32_t bufLen, bool waitIfNeeded)
{
    // Check if we're running at all
    if (state == CTX_STATE_STOPPED)
    {
        return -1;
    }

    nTxCall++;
    if (waitIfNeeded)
    {
        if (txSem.Pend(I2S_CHECK_INTERVAL) == OS_TIMEOUT)
        {
            if (txBufDone) { txBufDone(buf, false); }
            return -1;
        }
    }
    else if (txSem.PendNoWait() == OS_TIMEOUT)
    {
        if (txBufDone) { txBufDone(buf, false); }
        return -1;
    }
    if (state == CTX_STATE_STOPPED)
    {
        if (txBufDone) { txBufDone(buf, false); }
        return -1;
    }

    // Check if we've begun transmitting at all
    if ((state != CTX_STATE_TX) && (state != CTX_STATE_TXRX))
    {
        nTxStart++;
        USERCritObj crit;
        TxRing.ConfigFirstAndEnableCh((uint32_t) buf, bufLen/txByteWidth);
        state = (state == CTX_STATE_RX) ? CTX_STATE_TXRX : CTX_STATE_TX;
        txCh->suspWr();
        // We only want to synchronize on TF if we're using TF for a start
        //  condition
        uint32_t start = (SSC->SSC_TCMR & SSC_TCMR_START_Msk);
        if (start == SSC_TCMR_START_RECEIVE)
        {
            (void)SSC->SSC_SR;
            while (!(SSC->SSC_SR & SSC_SR_RXSYN)) {}
        }
        else if (start != SSC_TCMR_START_CONTINUOUS)
        {
            (void)SSC->SSC_SR;
            while (!(SSC->SSC_SR & SSC_SR_TXSYN)) {}
        }

        SSC->SSC_CR = SSC_CR_TXEN;
//        while (!(SSC->SSC_SR & SSC_SR_TXSYN)) {}
        txCh->resWr();
        txCh->enableGIrq();
    }
    else
    {
        nTxAdd++;
        P2[18] = 1;
        TxRing.AddXfr((uint32_t)buf, bufLen/txByteWidth, 10);
        P2[18] = 0;
    }
    return 0;
}

int SSCCtx_t::ReadyReceiveBuffer(void *buf, uint32_t bufLen, bool waitIfNeeded)
{
    // Check if we're running at all
    if (state == CTX_STATE_STOPPED)
    {
        return -1;
    }

    nRxCall++;
    if (waitIfNeeded)
    {
        if (rxSem.Pend(I2S_CHECK_INTERVAL) == OS_TIMEOUT)
        {
            if (rxBufDone) { rxBufDone(buf, false); }
            return -1;
        }
    }
    else if (rxSem.PendNoWait() == OS_TIMEOUT)
    {
        if (rxBufDone) { rxBufDone(buf, false); }
    }
    if (state == CTX_STATE_STOPPED)
    {
        if (rxBufDone) { rxBufDone(buf, false); }
        return -1;
    }

    // Check if we've begun receiving at all
    if ((state != CTX_STATE_RX) && (state != CTX_STATE_TXRX))
    {
        nRxStart++;
        USERCritObj crit;
        RxRing.ConfigFirstAndEnableCh((uint32_t) buf, bufLen/rxByteWidth);
        state = (state == CTX_STATE_RX) ? CTX_STATE_TXRX : CTX_STATE_RX;
        rxCh->suspRd();

        // We only want to synchronize on RF if we're using RF for a start
        //  condition
        uint32_t start = (SSC->SSC_RCMR & SSC_RCMR_START_Msk);
        if (start == SSC_RCMR_START_TRANSMIT)
        {
            (void)SSC->SSC_SR;
            while (!(SSC->SSC_SR & SSC_SR_TXSYN)) {}
        }
        else if ((start != SSC_RCMR_START_CONTINUOUS)
                && (start != SSC_RCMR_START_TRANSMIT)
                && (start != SSC_RCMR_START_CMP_0))
        {
            SSC->SSC_CR = SSC_CR_RXEN;
            SSC->SSC_RCMR &= ~SSC_RCMR_START_Msk;
            SSC->SSC_RCMR |= (SSC->SSC_RFMR & SSC_RFMR_FSEDGE_NEGATIVE)
                                ? SSC_RCMR_START_RF_FALLING
                                : SSC_RCMR_START_RF_RISING;
            (void)SSC->SSC_SR;
            while (!(SSC->SSC_SR & SSC_SR_RXSYN)) {}
            SSC->SSC_RCMR = (SSC->SSC_RCMR & ~SSC_RCMR_START_Msk) | start;
        }
        else
        {
            SSC->SSC_CR = SSC_CR_RXEN;
        }
        (void)SSC->SSC_RHR;
        rxCh->resRd();
        rxCh->enableGIrq();
    }
    else
    {
        nRxAdd++;
        RxRing.AddXfr((uint32_t)buf, bufLen/rxByteWidth, 10);
    }
    return 0;
}

void SSCCtx_t::RegisterTxBufferDoneCB(SSC_BufferDoneFn_t cb)
{
    txBufDone = cb;
}

void SSCCtx_t::RegisterRxBufferDoneCB(SSC_BufferDoneFn_t cb)
{
    rxBufDone = cb;
}


void SSC_DMA_TxIsr(XdmaCh_t *ch)
{
    ssc.TxIsr();
}

void SSC_DMA_RxIsr(XdmaCh_t *ch)
{
    ssc.RxIsr();
}

bool isActiveDesc(dma_descview_0 *desc, uint32_t addr, uint32_t remLen, uint32_t dwidth)
{
    return ((desc->addr <= addr)
            && (((desc->addr + (desc->ubCtrl & ~(0x7 << 24))*dwidth)) > addr)
            && ((desc->ubCtrl & ~(0x7 << 24)) >= remLen));
}

void SSCCtx_t::TxIsr()
{
    volatile uint32_t stat = txCh->readyStatus();
    nTxIrq++;
    // Did we get a read fault for the memory config?
    // If so, the channel is disabled, so clear the TX State
    if (stat & XDMAC_CIS_RBEIS)
    {
        nTxErr++;
        state = (state == CTX_STATE_TXRX) ? CTX_STATE_RX : CTX_STATE_INIT;
        txCh->disable();
        SSC->SSC_CR = SSC_CR_TXDIS;
//        txCh->disableGIrq();
        for (uint32_t free_idx = 0; free_idx < SSC_BUFFER_DEPTH; free_idx++)
        {
            if (TxRing.BDFlag[free_idx] & XDMA_BD_INUSE)
            {
                TxRing.BDFlag[free_idx] &= ~(XDMA_BD_INUSE | XDMA_BD_SWEEP_MARK);
                TxRing.nBDFreed++;
                if (txBufDone) { txBufDone((void *)TxRing.descs[free_idx].addr, false); }
                nTxPost++;
                txSem.Post();
            }
            else
            {
                TxRing.BDFlag[free_idx] &= ~(XDMA_BD_INUSE | XDMA_BD_SWEEP_MARK);
            }
        }
    }
    else if (stat & XDMAC_CIS_BIS)
    {
        uint32_t ubLen;
        uint32_t addr = txCh->gSrcAddr();
        dma_descview_0 *first = (dma_descview_0 *)txCh->getNextDesc(&ubLen);
        dma_descview_0 *last = first;
        if (isActiveDesc(TxRing.queueHead, addr,
                                    ubLen, txCh->gXfrWidth()))
        {
        P2[17] = 1;
        }
        while (TxRing.queueHead
                && (TxRing.queueHead != first)
                && (TxRing.queueHead != last)
                && (TxRing.queueHead != TxRing.queueHead->pNext_0)
                && !isActiveDesc(TxRing.queueHead, addr,
                                    ubLen, txCh->gXfrWidth()))

        {
            P2[19] = 1;
            TxRing.BDFlag[TxRing.queueHead-TxRing.descs] &= ~(XDMA_BD_INUSE);
            TxRing.nBDFreed++;
            if (txBufDone)
            {
                txBufDone((void *)TxRing.queueHead->addr, true);
            }
            nTxPost++;
            TxRing.queueHead = TxRing.queueHead->pNext_0;
            txSem.Post();
            P2[19] = 0;

            last = (dma_descview_0 *)txCh->getNextDesc(&ubLen);
            addr = txCh->gDstAddr();
        }
#if 0
        // While we *should* be able to reduce this to simply:
        //      while (last != last->pNext) { last = last->pNext; }
        //  we need to ensure that in the event there is *not* an end element,
        //  that we do not loop forever
        if (first != first->pNext) {
            while (last
                    && (last->pNext)
                    && (last != last->pNext)
                    && (last->pNext != first)) {
                TxRing.BDFlag[last-TxRing.descs] |= XDMA_BD_SWEEP_MARK;
                // if the descriptor does not enable fetch, it's the end
                if (!(last->ubCtrl & (0x1 << 24))) { break; }
                last = (dma_descview_0 *)last->pNext;
            }
        }
        if (last)
        {
            TxRing.BDFlag[last-TxRing.descs] |= XDMA_BD_SWEEP_MARK;
        }

//        int free_idx = ((last - TxRing.descs) + 1) % SSC_BUFFER_DEPTH;
//        while (free_idx != (first - TxRing.descs))
        for (int free_idx = 0; free_idx < TxRing.ringLen; free_idx++)
        {
            if (((TxRing.BDFlag[free_idx] & XDMA_BD_BUFFER_DONE_Msk)
                    == XDMA_BD_BUFFER_DONE)
                && (!isActiveDesc(TxRing.descs+free_idx, addr,
                            ubLen, txCh->gXfrWidth())))
            {
                P2[19] = 1;
                TxRing.BDFlag[free_idx] &= ~(XDMA_BD_INUSE
                                            | XDMA_BD_SWEEP_MARK);
                TxRing.nBDFreed++;
                if (txBufDone)
                {
                    txBufDone((void *)TxRing.descs[free_idx].addr, true);
                }
                nTxPost++;
                txSem.Post();
                P2[19] = 0;
            }
            else
            {
                TxRing.BDFlag[free_idx] &= ~(XDMA_BD_SWEEP_MARK);
            }
        }
#endif
        P2[17] = 0;
    }
}

void SSCCtx_t::RxIsr()
{
    volatile uint32_t stat = rxCh->readyStatus();
    nRxIrq++;
    // Did we get a write fault for the memory config?
    // If so, the channel is disabled, so clear the RX State
    if (stat & XDMAC_CIS_WBEIS)
    {
        nRxErr++;
        state = (state == CTX_STATE_TXRX) ? CTX_STATE_TX : CTX_STATE_INIT;
        rxCh->disable();
        SSC->SSC_CR = SSC_CR_RXDIS;
//        rxCh->disableGIrq();
        for (uint32_t free_idx = 0; free_idx < SSC_BUFFER_DEPTH; free_idx++)
        {
            if (RxRing.BDFlag[free_idx] & XDMA_BD_INUSE)
            {
                RxRing.BDFlag[free_idx] &= ~(XDMA_BD_INUSE | XDMA_BD_SWEEP_MARK);
                RxRing.nBDFreed++;
                if (rxBufDone) { rxBufDone((void *)RxRing.descs[free_idx].addr, false); }
                nRxPost++;
                rxSem.Post();
            }
            else
            {
                RxRing.BDFlag[free_idx] &= ~(XDMA_BD_INUSE | XDMA_BD_SWEEP_MARK);
            }
        }
    }
    else if (stat & XDMAC_CIS_BIS)
    {
        uint32_t ubLen;
        dma_descview_0 *first = (dma_descview_0 *)rxCh->getNextDesc(&ubLen);
        uint32_t addr = rxCh->gDstAddr();
        dma_descview_0 *last = first;

        while (RxRing.queueHead
                && (RxRing.queueHead != first)
                && (RxRing.queueHead != last)
                && (RxRing.queueHead != RxRing.queueHead->pNext_0)
                && !isActiveDesc(RxRing.queueHead, addr,
                                    ubLen, rxCh->gXfrWidth()))

        {
            P2[20] = 1;
            RxRing.BDFlag[RxRing.queueHead-RxRing.descs] &= ~(XDMA_BD_INUSE);
            RxRing.nBDFreed++;
            if (rxBufDone)
            {
                rxBufDone((void *)RxRing.queueHead->addr, true);
            }
            nRxPost++;
            RxRing.queueHead = RxRing.queueHead->pNext_0;
            rxSem.Post();
            P2[20] = 0;

            last = (dma_descview_0 *)rxCh->getNextDesc(&ubLen);
            addr = rxCh->gDstAddr();
        }
#if 0
        // While we *should* be able to reduce this to simply:
        //      while (last != last->pNext) { last = last->pNext; }
        //  we need to ensure that in the event there is *not* an end element,
        //  that we do not loop forever
        if (first != first->pNext) {
            while (last
                    && (last->pNext)
                    && (last != last->pNext)
                    && (last->pNext != first)) {
                RxRing.BDFlag[last-RxRing.descs] |= XDMA_BD_SWEEP_MARK;
                // if the descriptor does not enable fetch, it's the end
                if (!(last->ubCtrl & (0x1 << 24))) { break; }
                last = (dma_descview_0 *)last->pNext;
            }
        }
        if (last)
        {
            RxRing.BDFlag[last-RxRing.descs] |= XDMA_BD_SWEEP_MARK;
        }

//        int free_idx = (last - RxRing.descs + 1) % SSC_BUFFER_DEPTH;
        for (int free_idx = 0; free_idx < RxRing.ringLen; free_idx++)
        {
            if (((RxRing.BDFlag[free_idx] & XDMA_BD_BUFFER_DONE_Msk)
                    == XDMA_BD_BUFFER_DONE)
                && (!isActiveDesc(RxRing.descs+free_idx, addr,
                            ubLen, rxCh->gXfrWidth())))
            {
                P2[20] = 1;
                RxRing.BDFlag[free_idx] &= ~(XDMA_BD_INUSE
                                            | XDMA_BD_SWEEP_MARK);
                RxRing.nBDFreed++;
                if (rxBufDone)
                {
                    rxBufDone((void *)RxRing.descs[free_idx].addr, true);
                }
                nRxPost++;
                rxSem.Post();
                P2[20] = 0;
            }
            else
            {
                RxRing.BDFlag[free_idx] &= ~(XDMA_BD_SWEEP_MARK);
            }
        }
#endif
    }


}


void SSCCtx_t::dump()
{
    XdmaCh_t * xdmaCh = ssc.TxRing.xdmaCh;
    XdmaCh_t *txCh = ssc.txCh;
    XdmaCh_t *rxCh = ssc.rxCh;
    iprintf("========================================");
    iprintf("\nDMA: \n");
    iprintf("txCh ID: %d\n", txCh->getID());
    iprintf("rxCh ID: %d\n", rxCh->getID());
    PRINT_REG(SSC, SSC_TCMR, "%08lx");
    PRINT_REG(SSC, SSC_TFMR, "%08lx");
    PRINT_REG(SSC, SSC_RCMR, "%08lx");
    PRINT_REG(SSC, SSC_RFMR, "%08lx");
    PRINT_REG(SSC, SSC_SR, "%08lx\n");

    PRINT_REG(XDMAC, XDMAC_GS, "%08lx");
    PRINT_REG(XDMAC, XDMAC_GIM, "%08lx");
    PRINT_REG(XDMAC, XDMAC_GIS, "%08lx");
    PRINT_REG(XDMAC, XDMAC_GRS, "%08lx");
    PRINT_REG(XDMAC, XDMAC_GWS, "%08lx\n");
    for (int i = 0; i < 2; i++)
    {

        PRINT_REG(xdmaCh, XDMAC_CC, "%08lx");
        PRINT_REG(xdmaCh, XDMAC_CIM, "%08lx");
        PRINT_REG(xdmaCh, XDMAC_CIS, "%08lx");
        PRINT_REG(xdmaCh, XDMAC_CSA, "%08lx");
        PRINT_REG(xdmaCh, XDMAC_CDA, "%08lx");
        PRINT_REG(xdmaCh, XDMAC_CBC, "%08lx");
        PRINT_REG(xdmaCh, XDMAC_CUBC, "%08lx");
        PRINT_REG(xdmaCh, XDMAC_CNDA, "%08lx");
        PRINT_REG(xdmaCh, XDMAC_CNDC, "%08lx");
        PRINT_REG(xdmaCh, XDMAC_CDS_MSP, "%08lx");
        PRINT_REG(xdmaCh, XDMAC_CDUS, "%08lx");
        PRINT_REG(xdmaCh, XDMAC_CSUS, "%08lx");
        iprintf("\n");
        xdmaCh = ssc.RxRing.xdmaCh;
    }

    iprintf("TxIRQ:   %-8lu - RxIRQ:   %-8lu\n", ssc.nTxIrq, ssc.nRxIrq);
    iprintf("TxErr:   %-8lu - RxErr:   %-8lu\n", ssc.nTxErr, ssc.nRxErr);
    iprintf("TxCall:  %-8lu - RxCall:  %-8lu\n", ssc.nTxCall, ssc.nRxCall);
    iprintf("TxStart: %-8lu - RxStart: %-8lu\n", ssc.nTxStart, ssc.nRxStart);
    iprintf("TxAdd:   %-8lu - RxAdd:   %-8lu\n", ssc.nTxAdd, ssc.nRxAdd);
    iprintf("TxPost:  %-8lu - RxPost:  %-8lu\n", ssc.nTxPost, ssc.nRxPost);
    TxRing.PrintDescChain();
    RxRing.PrintDescChain();

    iprintf("\n");
    ssc.TxRing.PrintCounts();
    iprintf("\n");
    ssc.RxRing.PrintCounts();
}
