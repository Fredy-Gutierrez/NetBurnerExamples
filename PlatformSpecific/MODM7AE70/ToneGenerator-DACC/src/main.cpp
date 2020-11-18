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
#include <math.h>

extern "C"
{
   void UserMain( void *pd );
};

const char *AppName = "SAME70 D/A Tone Generator";
void EnablePeriphClock(int id);
IRQn GetTimerPeriphID(int timer);

#define SAMPLE_RATE 48000
#define SIGNAL_FREQ 480
#define MAX_SAMPLES 2400
#define TWO_PI (3.14159 * 2)

uint16_t    OutputSamples[MAX_SAMPLES];
uint32_t    SampleCount;
uint32_t    idx;

extern volatile uint32_t CPU_CLOCK;
extern volatile uint32_t PERIPH_CLOCK;

extern "C" void DACC_Handler()
{
    DACC->DACC_CDR[0] = OutputSamples[idx];
    idx = ((idx + 1) >= SampleCount) ? 0 : (idx+1);
}

void ConfigureTrigger(int timer, uint32_t freq)
{
    int tcnum = timer / 3;
    Tc &tc = ((tcnum == 0) ? *TC0 :
                ((tcnum == 1) ? *TC1 :
                    ((tcnum == 2) ? *TC2 : *TC3)));
    TcChannel &tcc = tc.TC_CHANNEL[timer%3];

    IRQn periphID = GetTimerPeriphID(timer);

    EnablePeriphClock(periphID);
    // Disable the TC channel
    tcc.TC_CCR = TC_CCR_CLKDIS;
    asm volatile ("dsb"::: "memory");
    // Configer the timer channel for PCK6 and reset on C compare match
    tcc.TC_CMR = TC_CMR_WAVE | TC_CMR_TCCLKS_TIMER_CLOCK2
                |TC_CMR_ACPA_SET | TC_CMR_ACPC_CLEAR
                    | TC_CMR_WAVSEL_UP_RC;

    tcc.TC_RA = (tcc.TC_RC = PERIPH_CLOCK/freq) - 1;

//    NVIC_SetPriority(periphID, 0x5);
//    NVIC_EnableIRQ(periphID);
    // Enable the timer channel
    tcc.TC_CCR = TC_CCR_CLKEN | TC_CCR_SWTRG;
}

void EnableDAC()
{
    EnablePeriphClock(ID_DACC);

    DACC->DACC_CR = DACC_CR_SWRST;
    asm("dsb");
    DACC->DACC_MR = DACC_MR_PRESCALER(9);// | DACC_MR_WORD_ENABLED;
    DACC->DACC_MR = DACC_MR_PRESCALER(9);// | DACC_MR_WORD_ENABLED;
    DACC->DACC_TRIGR = DACC_TRIGR_TRGEN0 | DACC_TRIGR_TRGSEL0(1);

    DACC->DACC_IER = DACC_IER_TXRDY0;
    DACC->DACC_ACR = DACC_ACR_IBCTLCH0(3);
    DACC->DACC_CHER = DACC_CHER_CH0;

    NVIC_SetPriority(DACC_IRQn, 0x2);
    NVIC_EnableIRQ(DACC_IRQn);
}

void PauseDAC()
{
    NVIC_DisableIRQ(DACC_IRQn);
}

void ResumeDAC()
{
    NVIC_EnableIRQ(DACC_IRQn);
}

void buildSinTable(uint32_t sample_rate, uint32_t freq)
{
    SampleCount = sample_rate/freq;
    if (SampleCount > MAX_SAMPLES) { SampleCount = MAX_SAMPLES; }

    double phaseIncrement = TWO_PI/(double)SampleCount;
    SampleCount = (MAX_SAMPLES/SampleCount)*SampleCount;
    double currentPhase = 0.0;

    for (uint32_t i = 0; i < SampleCount; i++){
        double curSin = sin(currentPhase) + 1;
        OutputSamples[i] = ((uint32_t)(0x7FF*curSin));

        currentPhase += phaseIncrement;
    }
}

char *getsn(char *str, int num);

void UserMain(void *pd)
{
    char buf[80];
    init();
    iprintf("Application Started\n");
    buildSinTable(SAMPLE_RATE, SIGNAL_FREQ);
    ConfigureTrigger(0, SAMPLE_RATE);
    EnableDAC();


    iprintf("DAC Running\n");

    while (1)
    {
        iprintf("Enter frequency: ");
        getsn(buf, 80);
        int newFreq = strtod(buf, NULL);
        if (newFreq < 20) { newFreq = 20; }
        else if (newFreq > 22000) { newFreq = 22000; }
        PauseDAC();
        buildSinTable(SAMPLE_RATE, newFreq);
        ResumeDAC();

    }
}
