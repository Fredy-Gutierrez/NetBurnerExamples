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
#include <math.h>
#include <init.h>
#include <sim.h>           /*on-chip register definitions*/
#include <pins.h>
#include <nbrtos.h>
#include <i2c.h>
#include <serial.h>
#include <iosys.h>
#include <random.h>
#include <smarttrap.h>
#include <bsp.h>
#include <netinterface.h>
#include "wm8904.h"
#include "hd44780.h"
#include "ssc_i2s.h"

#define LOGME iprintf("L: %d - F: %s\n", __LINE__, __FILE__);

extern "C"
{
   void UserMain( void *pd );
};

extern volatile uint32_t CPU_CLOCK;
extern volatile uint32_t PERIPH_CLOCK;

const char *AppName = "SAME70 SSC_I2S example";

WM8904 codec(i2c[2]);

void EnablePeriphClock(int id);

//uint32_t LocalDefUart = 1;

#define SAMP_BIT_SIZ (8)
#define SAMP_MAX_VAL ((uint32_t)((1ULL << SAMP_BIT_SIZ) - 1))
#define MAX_SAMPLES 2400
#define MAX_TONES 4
#define TWO_PI (3.14159265 * 2)

OS_SEM fpuOff;

uint32_t    ToneSamples[MAX_TONES][MAX_SAMPLES];
uint32_t    SampleCount;
uint32_t    idx;
uint32_t    tone_idx;
bool        LnR;

bool        rxLnR;
uint32_t    toneVal;
uint32_t    rxVal[2];
bool        invert;

class ToneGen {
protected:
    double curSin;
    double phase;
    double phaseInc;
    uint32_t sampleRate;
    uint32_t sampleCount;
    uint32_t nextFreq;
    uint32_t curFreq;

public:
    ToneGen(uint32_t sampleRate, uint32_t freq, double startPhase);
    virtual uint32_t getNext();
    uint32_t getCurFreq(double *phase) {
        USERCritObj crit();
        if (phase) { *phase = this->phase; }
        return curFreq;
    }
    void setNextFreq(uint32_t freq) { nextFreq = freq; }
    uint32_t operator++(int) { return getNext(); }
};

typedef uint32_t (*nextFreqFn_t) (uint32_t sampleRate, uint32_t curFreq, void *ctx);

class SweepGen : public ToneGen {
    uint32_t samplesPerFreq;
    nextFreqFn_t nextFreqFn;
    void    *freqFnCtx;
    uint32_t remainingSamples;
public:
    SweepGen(uint32_t sampleRate, uint32_t freq, double startPhase,
                 uint32_t samplesPerFreq,
                nextFreqFn_t freqGen, void *freqGenCtx);
    virtual uint32_t getNext() override;
};

struct expSweepCtx {
    uint32_t prevFreq;
    int dir;
    double a;
    double b;
    double c;
    double d;
};

// Up:   f_i = a*((f_(i-1))^b) + c*(f_(i-1)) - d
// Down: f_i = ((f_(i-1))^(1/b))/a - (f_(i-1))/c - d
uint32_t nextFreq_exponential(uint32_t sampleRate, uint32_t curFreq, void *void_ctx)
{
    expSweepCtx &ctx = *(expSweepCtx *)void_ctx;

    uint32_t nextFreq = 0;
    double f_cur = (double)curFreq;
    int switched = 0;
    do {
        if (ctx.dir > 0) {
            nextFreq = (uint32_t)((ctx.a * pow(f_cur, ctx.b))
                    + (ctx.c*f_cur) + ctx.d);
            if (nextFreq > (uint32_t)(((double)sampleRate)/(double)2)) {
                f_cur = (double)nextFreq;//((double)sampleRate)/(double)2;
                ctx.dir *= -1;
                switched++;
            }
            else {
                ctx.prevFreq = curFreq;
                break;
            }
        }
        else {
            nextFreq = (uint32_t)((pow(f_cur, 1/ctx.b)/ctx.a)
                    - (ctx.c*f_cur) - ctx.d);
            if (nextFreq < 30) {
                f_cur = (double)nextFreq;//(double)30;
                ctx.dir *= -1;
                switched++;
            }
            else {
                ctx.prevFreq = curFreq;
                break;
            }
        }
    } while (switched < 2);

    return (switched < 2) ? nextFreq : curFreq;
}

struct linearSweepCtx {
    uint32_t prevFreq;
    int dir;
    double a;
    double b;
};

uint32_t nextFreq_linear(uint32_t sampleRate, uint32_t curFreq, void *void_ctx)
{
    linearSweepCtx &ctx = *(linearSweepCtx *)void_ctx;

    uint32_t nextFreq = 0;
    double f_cur = (double)curFreq;
    int switched = 0;
    do {
        if (ctx.dir > 0) {
            nextFreq = (uint32_t)(f_cur + (ctx.a*f_cur) + ctx.b);
            if (nextFreq > (uint32_t)(((double)sampleRate)/(double)2)) {
                f_cur = (double)nextFreq;//((double)sampleRate)/(double)2;
                ctx.dir *= -1;
                switched++;
            }
            else {
                ctx.prevFreq = curFreq;
                break;
            }
        }
        else {
            nextFreq = (uint32_t)(f_cur - (ctx.a*f_cur) - ctx.b);
            if (nextFreq < 30) {
                f_cur = (double)nextFreq;//(double)30;
                ctx.dir *= -1;
                switched++;
            }
            else {
                ctx.prevFreq = curFreq;
                break;
            }
        }
    } while (switched < 2);

    return (switched < 2) ? nextFreq : curFreq;
}

ToneGen::ToneGen(uint32_t sampleRate, uint32_t freq, double startPhase)
    : curSin(0), phase(startPhase), phaseInc(0), sampleRate(sampleRate),
        sampleCount(0), nextFreq(freq), curFreq(0)
{
}
#ifdef ENABLE_LCD
HD44780_LCD_Bus lcd_bus __attribute__((section("EBI_2_SECT")));
HD44780_LCD lcd(lcd_bus, 16, 2);
#endif


#ifdef ENABLE_LCD
void DisplayPhaseInc(uint32_t freq, double phinc)
{
    ExtBusEnableCtx busEnable;
    char buf[40];
    sprintf(buf, "Freq: %-10d", freq);
    lcd.setCursor(0, 0);
    lcd.putstr(buf);
    for (int i = lcd[0] & 0x3F; i < 21; i++)
    {
        lcd.putchar(' ');
    }
    lcd.setCursor(1, 0);
    sprintf(buf, "Incr: %f", phinc);
    lcd.putstr(buf);
    for (int i = lcd[0] & 0x7F; i < (0x40+21); i++)
    {
        lcd.putchar(' ');
    }
}
#endif

uint32_t ToneGen::getNext()
{
    if (nextFreq != curFreq) {
        phaseInc  = TWO_PI/(((double)sampleRate)/(double)nextFreq);
        asm("dsb");
#ifdef ENABLE_LCD
        DisplayPhaseInc(nextFreq, phaseInc);
#endif

        curFreq = nextFreq;
    }
//    if (sampleCount > (sampleRate/curFreq))
//    {
//        sampleCount = 0;
//        return 0x7FFFFF;
//    }
//    if (sampleCount > (sampleRate/(2*curFreq)))
//    {
//        sampleCount++;
//        return 0x0;
//    }
//    else {
//        sampleCount++;
//        return 0x7FFFFF;
//    }
    asm("dsb");
    phase += phaseInc;
    if (phase > TWO_PI) { phase -= TWO_PI; }
    curSin = sin(phase) + 1.0;
//    volatile uint32_t ret =((uint32_t)(curSin*500000));
//    asm("dsb");
    return ((uint32_t)((SAMP_MAX_VAL & (SAMP_MAX_VAL>>1)) *curSin));
}

SweepGen::SweepGen(uint32_t sampleRate, uint32_t freq, double startPhase,
                    uint32_t samplesPerFreq,
                    nextFreqFn_t freqGen, void *freqGenCtx)
    : ToneGen(sampleRate, freq, startPhase), samplesPerFreq(samplesPerFreq),
        nextFreqFn(freqGen), freqFnCtx(freqGenCtx),
        remainingSamples(samplesPerFreq)
{
}

uint32_t SweepGen::getNext()
{
    uint32_t ret = ToneGen::getNext();
    if (remainingSamples == 0) {
        if (nextFreqFn) {
            nextFreq = nextFreqFn(sampleRate, curFreq, freqFnCtx);
        }
        remainingSamples = samplesPerFreq;
    }
    else {
        remainingSamples--;
    }

    return ret;
}

expSweepCtx sweep{ 0, 1, 1.0000, 1.0000, 0.005, 1 };
linearSweepCtx linsweep{ 0, 1, 0.005, 1 };

const uint64_t start64 = 0x4137f37fdfc20c06ULL;
SweepGen tone(48000, 7504, *(double*)(&start64), 300, nextFreq_exponential, &sweep);
//SweepGen tone(48000, 7504, 0, 900, nextFreq_exponential, &sweep);
//SweepGen tone(48000, 7504, 0, 24000, nextFreq_linear, &linsweep);
ToneGen lowTone(48000, 330, 0);
ToneGen highTone(48000, 660, 0);

extern "C"
void SSC_Handler()
{
//    P2[20] = 1;
//    SSC->SSC_THR = 0xFFFFFFFF;
//    SSC->SSC_THR = ToneSamples[tone_idx][idx];
    uint32_t sr = SSC->SSC_SR;
    if (sr & SSC_SR_RXRDY) {
        uint32_t val = SSC->SSC_RHR;
        if (invert) {
            if (val > 0x800000) {
                val = 0x800000 - val;
            }
            else {
                val += 0x800000;
            }
        }
        rxVal[rxLnR] = val;
        rxLnR = !rxLnR;
    }
//    if (sr & SSC_SR_TXRDY) {
//        SSC->SSC_THR = rxVal[rxLnR];
//        LnR = !LnR;
//    }
    SSC->SSC_THR = toneVal;
    if (LnR) {
        idx = ((idx + 1) >= SampleCount) ? 0 : (idx+1);
        toneVal = tone++;
    }
    LnR = !LnR;
//    P2[20] = 0;
}

void InitSSC(const SSC_cfg_t &cfg);
extern SSCCtx_t ssc;


void EnablePCK2()
{
    P2[24].function(PINP2_24_PCK2);
    PMC->PMC_PCK[2] = PMC_PCK_CSS_MAIN_CLK | PMC_PCK_PRES(3);
    PMC->PMC_SCER = PMC_SCER_PCK2;
}

void ConfigureWM8904();


void buildTable(uint32_t sample_rate, uint32_t freq, uint32_t tone)
{
    if (tone >= MAX_TONES) { return; }
    double fSampleCount = ((double)sample_rate)/(double)freq;
    SampleCount = sample_rate/freq;
    if (fSampleCount > MAX_SAMPLES) { fSampleCount = MAX_SAMPLES; }

    double phaseIncrement = TWO_PI/fSampleCount;
    SampleCount = (uint32_t)fSampleCount;
    SampleCount = (MAX_SAMPLES/SampleCount)*SampleCount;
    double currentPhase = 0.0;

    for (uint32_t i = 0; i < SampleCount; i++){
        double curSin = sin(currentPhase) + 1;
        ToneSamples[tone][i] = ((uint32_t)(0x7FFFFF00*curSin)) >> 8;

        currentPhase += phaseIncrement;
    }
}

extern const char LinkTime[];

void NoFPUTask(void *pd)
{
    while (1) {
        fpuOff.Pend(0);
asm volatile ("\n\t"
"ldr r0, =1f\n\t"
"vldr d0, [r0]\n\t"
"vldr d1, [r0]\n\t"
"vldr d2, [r0]\n\t"
"vldr d3, [r0]\n\t"
"vldr d4, [r0]\n\t"
"vldr d5, [r0]\n\t"
"vldr d6, [r0]\n\t"
"vldr d7, [r0]\n\t"
"vldr d8, [r0]\n\t"
"vldr d9, [r0]\n\t"
"vldr d10, [r0]\n\t"
"vldr d11, [r0]\n\t"
"vldr d12, [r0]\n\t"
"vldr d13, [r0]\n\t"
"vldr d14, [r0]\n\t"
"vldr d15, [r0]\n\t"
"b 2f\n"
"1:\n"
".quad 0x3FFC000000000000\n"
"2:\n\t"
:
:
: "r0", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
    "d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15"
);
    }
}

#ifdef ENABLE_LCD
void UpdateInfoDisplay()
{
    ExtBusEnableCtx busEnable;
    char IPBuf[40];
    InterfaceIP(1).sprintf(IPBuf, 40);
    lcd.setCursor(0, 0);
    lcd.putstr("IP: ");
    lcd.putstr(IPBuf);
    for (int i = lcd[0] & 0x3F; i < 21; i++)
    {
        lcd.putchar(' ');
    }
    lcd.setCursor(1, 0);
    lcd.putstr("Seconds: ");
    siprintf(IPBuf, "%lu", Secs);
    lcd.putstr(IPBuf);
    for (int i = lcd[0] & 0x7F; i < (0x40+21); i++)
    {
        lcd.putchar(' ');
    }
}
#endif


#define BUF_SAMP_COUNT (360)
#if     SAMP_BIT_SIZ == 32
#define SAMP_BYTE_SIZ (4)
#elif   SAMP_BIT_SIZ == 24
#define SAMP_BYTE_SIZ (4)
#elif   SAMP_BIT_SIZ == 16
#define SAMP_BYTE_SIZ (2)
#elif   SAMP_BIT_SIZ == 8
#define SAMP_BYTE_SIZ (1)
#endif
#define TX_SAMP_SIZ (SAMP_BYTE_SIZ)
#define TX_BUF_SIZ (TX_SAMP_SIZ * BUF_SAMP_COUNT)
#define TX_BUF_COUNT (16)

struct _txBuf {
    uint8_t pData[TX_BUF_SIZ];
};
_txBuf  txBufPool[TX_BUF_COUNT] FAST_USER_VAR;
bool    txBufFree[TX_BUF_COUNT] FAST_USER_VAR;;
uint32_t txLastUsed;
uint8_t *rxQStorage[TX_BUF_COUNT];
TEMPL_Q<uint8_t> rxBufQ(rxQStorage, TX_BUF_COUNT/2);

void AddToneData();

void FreeTxBuf(_txBuf *buf);

void I2STxBufDone(void *voidBuf, bool valid)
{
    _txBuf *buf = (_txBuf *)voidBuf;

    FreeTxBuf(buf);
}

void I2SRxBufDone(void *voidBuf, bool valid)
{
    _txBuf *buf = (_txBuf *)voidBuf;

    if (valid) {
        if (rxBufQ.Post((uint8_t *)voidBuf) != OS_Q_FULL)
        {
            // We intentionally *don't* free the buffer if we put it in the queue
            return;
        }
    }

    txBufFree[buf-txBufPool] = true;
}

void initTxBufPool()
{
    for (int i = 0; i < TX_BUF_COUNT; i++)
    {
        txBufFree[i] = true;
    }
}

void FreeTxBuf(_txBuf *buf)
{
    txBufFree[buf-txBufPool] = true;
}

uint8_t * GetTxBuf()
{
    for (uint32_t i = (txLastUsed + 1) % TX_BUF_COUNT;
            i != txLastUsed;
            i = (i + 1) % TX_BUF_COUNT)
    {
        if (txBufFree[i])
        {
            txBufFree[i] = false;
            txLastUsed = (txLastUsed + 1) % TX_BUF_COUNT;
            return txBufPool[i].pData;
        }
    }
    iprintf("NoBuffer\n");
    OSTimeDly(TICKS_PER_SECOND/2);

    return NULL;
}

uint8_t *GetToneBuffer()
{
    uint8_t *buf;
    buf=GetTxBuf();
    while (!buf) {
        OSTimeDly(1);
        buf=GetTxBuf();
    }
    uint32_t *buf32 = (uint32_t *)buf;

    int offset = 0;
    for (int i = 0; i < BUF_SAMP_COUNT; i+=2, offset += 2*TX_SAMP_SIZ)
    {
        volatile uint32_t samp = tone.getNext();
        if (TX_SAMP_SIZ == 4)
        {
            buf32[i] = samp;
            buf32[i+1] = samp;
        }
        else {
        for (int x = 0; x < TX_SAMP_SIZ; x++) {
            buf[offset+x] = (samp >> ((x) *8)) & 0xFF;
            buf[offset+TX_SAMP_SIZ+x] = (samp >> ((x) *8)) & 0xFF;
        }
        }
    }

    return buf;
}

uint8_t *GetSplitToneBuffer()
{
    uint8_t *buf;
    buf=GetTxBuf();
    while (!buf) {
        OSTimeDly(1);
        buf=GetTxBuf();
    }
    uint32_t *buf32 = (uint32_t *)buf;

    int offset = 0;
    for (int i = 0; i < BUF_SAMP_COUNT; i++, offset += TX_SAMP_SIZ)
    {
        volatile uint32_t samp = (i & 1) ? highTone.getNext() : lowTone.getNext();
        if (TX_SAMP_SIZ == 4)
        {
            buf32[i] = samp;
            buf32[i+1] = samp;
        }
        else {
        for (int x = 0; x < TX_SAMP_SIZ; x++) {
            buf[offset+x] = (samp >> ((x) *8)) & 0xFF;
            buf[offset+TX_SAMP_SIZ+x] = (samp >> ((x) *8)) & 0xFF;
        }
        }
    }

    return buf;
}

uint32_t lastTone;
uint32_t toneFIR;
uint32_t lastSamp;
uint32_t lastDeltaSamp;
uint32_t sampFIR;
uint32_t lastComposite;
uint32_t bClipped;
uint8_t *GetFilledBuffer()
{
    switch ((P2[6] << 1) | (P2[8]))
    {
    case 0x1:
    {
        uint8_t ret;
        uint8_t *buf = rxBufQ.PendNoWait(ret);
        if (ret == OS_NO_ERR) {
            FreeTxBuf((_txBuf *)buf);
        }
        return GetToneBuffer();
    }
    case 0x2:
    {
        uint8_t ret;
        uint8_t *buf = rxBufQ.PendNoWait(ret);
        if (ret == OS_NO_ERR) {
            FreeTxBuf((_txBuf *)buf);
        }
        return GetSplitToneBuffer();
    }
    case 0x3:
    {
        uint8_t *toneBuf = GetToneBuffer();
        uint8_t *inBuf = rxBufQ.Pend();
        InvalidateCache_ByAddr((uint32_t *)inBuf, TX_BUF_SIZ);
        asm("dsb");
        int offset = 0;
        uint32_t tone = 0;
        uint32_t samp = 0;
        for (int i = 0; i < BUF_SAMP_COUNT; i++, offset += TX_SAMP_SIZ)
        {
            tone = 0;
            samp = 0;
            for (int x = 0; x < TX_SAMP_SIZ; x++) {
                tone |= toneBuf[offset+x] << (x * 8);
                samp |= inBuf[offset+x] << (x * 8);
            }
            uint32_t deltaTone = (((SAMP_MAX_VAL/2)+1) - tone) & SAMP_MAX_VAL;
            uint32_t deltaSamp = (((SAMP_MAX_VAL/2)+1) - samp) & SAMP_MAX_VAL;

            toneFIR = (toneFIR >> 2) + (toneFIR >> 3);
            sampFIR = (sampFIR >> 2) + (sampFIR >> 3);
            toneFIR += (deltaTone >> 2);
            sampFIR += (deltaSamp >> 2);
            lastDeltaSamp = deltaSamp;
            int32_t iDeltaTone = (((int32_t)tone) - (int32_t)lastTone);
            int32_t iDeltaSamp = (((int32_t)samp) - (int32_t)lastSamp);

            int32_t dcOffTone = toneFIR - ((SAMP_MAX_VAL/2)+1);
            int32_t dcOffSamp = sampFIR - ((SAMP_MAX_VAL/2)+1);

            lastTone = tone;
            lastSamp = samp;
            tone = tone + toneFIR;
            samp = samp + sampFIR;
            uint64_t prod = samp&SAMP_MAX_VAL;
            prod *= tone&SAMP_MAX_VAL;
            prod >>= SAMP_BIT_SIZ;
            prod &= SAMP_MAX_VAL | (0x1ULL << SAMP_BIT_SIZ);
            samp = samp + tone - ((uint32_t)prod);

            for (int x = 0; x < TX_SAMP_SIZ; x++) {
                inBuf[offset+x] = (samp >> ((x) * 8)) & 0xFF;
            }
        }

        FlushCache_ByAddr((uint32_t*)inBuf, TX_BUF_SIZ);
        FreeTxBuf((_txBuf*)toneBuf);
        return inBuf;
    }
    default:
    {
        // Would Invalidate the Rx Buffer from the cache if we were going
        //  to be handling the data in CPU
        return rxBufQ.Pend();
    }
    }

}

void AddToneData()
{
    uint8_t *buf=GetTxBuf();
    while (!buf) {
        OSTimeDly(1);
        buf=GetTxBuf();
    }
    codec.ReadyReceiveBuffer(buf, TX_BUF_SIZ, false);
    buf = GetFilledBuffer();
    codec.TransmitBuffer(buf, TX_BUF_SIZ, true);
}

void I2STask(void *pd)
{
    initTxBufPool();
    codec.RegisterTxBufferDoneCB(I2STxBufDone);
    codec.RegisterRxBufferDoneCB(I2SRxBufDone);
    codec.ReadyReceiveBuffer(GetTxBuf(), TX_BUF_SIZ, true);
    codec.ReadyReceiveBuffer(GetTxBuf(), TX_BUF_SIZ, true);
    codec.ReadyReceiveBuffer(GetTxBuf(), TX_BUF_SIZ, true);
    codec.TransmitBuffer(GetToneBuffer(), TX_BUF_SIZ, true);
    codec.TransmitBuffer(GetToneBuffer(), TX_BUF_SIZ, true);
    while (1) {
        AddToneData();
    }
}

#define PRINT_REG(struc, reg, fmt) iprintf(#struc"_"#reg":  " fmt "\n", (struc)->reg)

dma_descview_0 chainview[30];
dma_descview_0 *chaindescs[30];
uint32_t chainLen;

uint8_t volume = 0x0D;

WM8904::cfg_t codec_cfg
{
    48000,
    3000000,
    WM8904::DATA_FMT_16_I2S,
    WM8904::CH_SELECT_LEFT_RIGHT,
    WM8904::CH_SELECT_LEFT_RIGHT,
    WM8904::AUDIO_IN_2,
    WM8904::AUDIO_OUT_HP,
    0,
    0x0D
};

#include <bsp.h>

char * getline(char *charbuf, uint32_t bufLen)
{
    char c;
    int i = 0;
    iprintf("getline");
    while (i < bufLen-1) {
        read(0, &c, 1);
        switch (c)
        {
            case 0x8:
                if (i > 0) i--;
                putchar('-');
                break;
            case '\n':
                charbuf[i] = '\0';
                return charbuf;
            default:
                charbuf[i++] = c;
                break;
        }
    };
    charbuf[i] = '\0';
    return charbuf;
}

void UserMain(void *pd)
{
#ifdef ENABLE_LCD
    lcd.init();
    UpdateInfoDisplay();
#endif
    init();
//    EnableExtBusBuff(true);
//    LocalDefUart = 1;
    EnableSmartTraps();

//    int fd_ser = SimpleOpenSerial(1, 115200);
//    int old_stdio = ReplaceStdio(0, fd_ser);
//    ReplaceStdio(1, fd_ser);
//    ReplaceStdio(2, fd_ser);
//    close(old_stdio);
//    iprintf("Serial 1 enabled\n");
//    iprintf("\n\n\nSerial 1 enabled\n");
//    iprintf("PMC_MCKR: %#08lx\n", PMC->PMC_MCKR);
//    iprintf("%s\n", LinkTime);
//    FPU->FPDSCR = 0x2 << FPU_FPDSCR_RMode_Pos;
//    P2[4].function(PINP2_4_TK);

    InterfaceBlock *pif = GetInterfaceBlock(0);
    if (pif)
    {
        iprintf("discovery: %s\n", pif->discovery_server.c_str());
    }
//    OSSimpleTaskCreatewNameSRAM(NoFPUTask, ETHER_SEND_PRIO-2, "FPU Disable Task");
//    OSSimpleTaskCreatewNameSRAM(NoFPUTask, MAIN_PRIO-1, "FPU Disable Task");

    P2[20] = 0;
    P2[19] = 0;
    P2[18] = 0;
    P2[17] = 0;
    P2[20].drive();
    P2[19].drive();
    P2[18].drive();
    P2[17].drive();

    EnablePCK2();
    OSTimeDly(TICKS_PER_SECOND/2);


    codec.Init(codec_cfg, SSC_I2S_SLAVE_8_TXRX_RK);

    P2[8].function(PINP2_8_IN);
    P2[19].function(PINP2_19_OUT);
    P2[18].function(PINP2_19_OUT);

    OSSimpleTaskCreatewNameSRAM(I2STask, MAIN_PRIO-1, "I2S Task");

    char charbuf[80];
    while (1)
    {
        iprintf("\nEnter new Volume: ");
        char *s = getline(charbuf, 80);
        iprintf("\n  Input: \"%s\"", s);
        if ((*s >= '0') && (*s <= '9'))
        {
            iprintf("strconv\n");
            uint32_t volume = strtoul(s, NULL, 0);
            codec.SetVolume(WM8904::AUDIO_OUT_HP, WM8904::CH_SELECT_LEFT_RIGHT, volume);
        }
        else
        {
            ssc.dump();
        }
    }
}
