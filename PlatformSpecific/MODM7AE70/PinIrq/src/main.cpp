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
#include <pin_irq.h>

/******************************************************************************/
/*  @file                                                                     */
/*  @brief PinIrq example for the MODM7AE70.                                  */
/*                                                                            */
/*  @mainpage MODM7AE70 PinIrq Example                                        */
/*                                                                            */
/*  @section Purpose                                                          */
/*                                                                            */
/*  This example demonstrates how to use PinIrq feature in its various        */
/*  triggering modes.                                                         */
/*                                                                            */
/*  @section Description                                                      */
/*                                                                            */
/*  The PinIrq example configures 8 gpio io signals to act as interrupt       */
/*  sources for the processor. The lines used are the ones connected to the   */
/*  DIP switches on the MOD-DEV-70 carrier board. When configuring the lines, */
/*  they are configured in the various different modes that the PIO controller*/
/*  can support.                                                              */
/*                                                                            */
/*  Additionally, some common application level behavior for disabling and    */
/*  reenabling certain IRQs is demonstated as well.                           */
/*                                                                            */
/*  WARNING: Continually leaving switch 8 in the on state will cause the      */
/*           watchdog to reset the module. This is due to continously         */
/*           triggering interrupts to be handled, which the example           */
/*           *does not mask*, leading to CPU exhaustion starving the          */
/*           OS scheduler.                                                    */
/******************************************************************************/


const char *AppName = "SAME70 PinIrq";

// dipsw is a convenience array that holds PinIO objects for the GPIO pins
//  connected to the carrier board's DIP switches.
//  That way we can just refer to the switch number rather than remember which
//  header pin connects.
PinIO dipsw[8] = {P2[8], P2[6], P2[7], P2[10], P2[9], P2[11], P2[12], P2[13]};
// Counters to keep track of how many times an IRQ has triggered.
uint32_t counters[8];


extern "C"
{
   void UserMain( void *pd );
};

void dipswIrq_0(int pio_idx, int pin);
void dipswIrq_1(int pio_idx, int pin);
void dipswIrq_2(int pio_idx, int pin);
void dipswIrq_3(int pio_idx, int pin);
void dipswIrq_4(int pio_idx, int pin);
void dipswIrq_5(int pio_idx, int pin);
void dipswIrq_6(int pio_idx, int pin);
void dipswIrq_7(int pio_idx, int pin);


/******************************************************************************/
/* The PinIrq Information to know:                                            */
/*  bool SetPinIrq( PinIO pin, int polarity, PinIrq_t func );                 */
/*  * @brief Set up an interrupt request (IRQ) pin.                           */
/*  *                                                                         */
/*  * @param pin A PinIO object that is to be configured as an IRQ Pin.       */
/*  * @param polarity Polarity and edge vs. level sensitivity configuration.  */
/*  *          *  2: high level sensitive                                     */
/*  *          *  1: positive edge sensitive                                  */
/*  *          *  0: (either) edge sensitive                                  */
/*  *          * -1: negative edge sensitive                                  */
/*  *          * -2: low level sensitive                                      */
/*  *  @param func A function pointer of type PinIrq_t to register as the serv*/
/*  *      handler (ISR) for the given PinIO.                                 */
/*  *                                                                         */
/*  *  @returns Boolean, whether or not the PinIO was successfully registered */
/*  *      for IRQs.                                                          */
/*                                                                            */
/*  typedef void (*PinIrq_t) (int pio_idx, int pin);                          */
/*  *  @brief Function type for PinIrq IRQ handlers.                          */
/*  *                                                                         */
/*  *  If the triggering pin is needed to decipher between multiple possible  */
/*  *  triggers, the PinIO can be constructed as 'PinIO trigger(pio_idx, pin);*/
/*  *                                                                         */
/*  *  @param pio_idx The port index to pass to the PinIO constructor, should */
/*  *      the Interrupt Service Handler (ISR) require a PinIO of what trigger*/
/*  *      the interrupt.                                                     */
/*  *  @param pin The pin number in the given port that triggered the interrupt;*/
/*  *      used to construct a PinIO along with pio_idx.                      */
/*  *                                                                         */
/*  *  @returns void                                                          */
/******************************************************************************/

void ConfigureIrqPins()
{
    // Always enabled Irq, (either) edge sensitive
    SetPinIrq( dipsw[0], 0, dipswIrq_0 );
    // Always enabled Irq, rising edge sensitive
    SetPinIrq( dipsw[1], 1, dipswIrq_1 );
    // Always enabled Irq, falling edge sensitive
    SetPinIrq( dipsw[2], -1, dipswIrq_2 );
    // Disables on IRQ, Resets based on time delay, high level sensitive
    SetPinIrq( dipsw[3], 2, dipswIrq_3 );
    // Always enabled Irq, level sensitive, changes polarity on trigger
    SetPinIrq( dipsw[4], 2, dipswIrq_4 );
    // Ping pong irq with switch 6, disables on trigger, enables switch 6 irq,
    //  falling edge sensitive
    SetPinIrq( dipsw[5], -1, dipswIrq_5 );

    // Ping pong irq with switch 5, disables on trigger, enables switch 5 irq,
    //  rising edge sensitive
    SetPinIrq( dipsw[6], 1, dipswIrq_6 );
    // Always enabled Irq, high level sensitive
    SetPinIrq( dipsw[7], 2, dipswIrq_7 );
}



// Always enabled Irq, (either) edge sensitive
void dipswIrq_0(int pio_idx, int pin)  { counters[0]++; }
// Always enabled Irq, rising edge sensitive
void dipswIrq_1(int pio_idx, int pin)  { counters[1]++; }
// Always enabled Irq, falling edge sensitive
void dipswIrq_2(int pio_idx, int pin)  { counters[2]++; }
// Disables on IRQ, Resets based on time delay, high level sensitive
void dipswIrq_3(int pio_idx, int pin)
{
    DisableIrq(PinIO(pio_idx, pin));
    counters[3]++;
}

// Always enabled Irq, level sensitive, changes polarity on trigger
void dipswIrq_4(int pio_idx, int pin)
{
    PinIO irqPin(pio_idx, pin);
    static volatile bool levelSelect;
    // switch out the polarity by re-registering
    SetPinIrq(irqPin, (levelSelect ? -2 : 2), dipswIrq_4);
    levelSelect = !levelSelect;
    counters[4]++;
}

// Ping pong irq with switch 6, disables on trigger, enables switch 6 irq,
//  falling edge sensitive
void dipswIrq_5(int pio_idx, int pin)
{
    DisableIrq(PinIO(pio_idx, pin));
    EnableIrq(dipsw[6]);
    counters[5]++;
}

// Ping pong irq with switch 5, disables on trigger, enables switch 5 irq,
//  rising edge sensitive
void dipswIrq_6(int pio_idx, int pin)
{
    DisableIrq(PinIO(pio_idx, pin));
    EnableIrq(dipsw[5]);
    counters[6]++;
}

// Always enabled Irq, high level sensitive
void dipswIrq_7(int pio_idx, int pin)
{
    counters[7]++;
}

void PrintIrqCounts()
{
    iprintf(
    "Irq:     dip_0 |  dip_1 |  dip_2 |  dip_3 |  dip_4 |  dip_5 |  dip_6 |  dip_7\r\n"
    "       -------- -------- -------- -------- -------- -------- -------- --------\r\n"
    "Counts:%7lu |%7lu |%7lu |%7lu |%7lu |%7lu |%7lu |%7lu\r\n\r\n",
    counters[0], counters[1], counters[2], counters[3],
    counters[4], counters[5], counters[6], counters[7]
);
}

const char infomsg[] =
"Welcome to the PinIrq demo.\r\n"
"\r\n"
"This example uses the pins connected to the DIP switches on the carrier board.\r\n"
"The interrupt behavior varies depending on the input source. They are:\r\n"
"\r\n"
"Switch 1 (P2[8]):   Always enabled Irq, (either) edge sensitive\r\n"
"Switch 2 (P2[6]):   Always enabled Irq, rising edge sensitive\r\n"
"Switch 3 (P2[7]):   Always enabled Irq, falling edge sensitive\r\n"
"Switch 4 (P2[10]):  Disables on IRQ, Resets based on time delay,\r\n"
"                        high level sensitive\r\n"
"Switch 5 (P2[9]):   Always enabled Irq, level sensitive,\r\n"
"                        changes polarity on trigger\r\n"
"Switch 6 (P2[11]):  Ping pong irq with switch 7, disables on trigger,\r\n"
"                        enables switch 7 irq, falling edge sensitive\r\n"
"Switch 7 (P2[12]):  Ping pong irq with switch 6, disables on trigger,\r\n"
"                        enables switch 6 irq, rising edge sensitive\r\n"
"Switch 8 (P2[13]):  Always enabled Irq, high level sensitive\r\n"
"\r\n"
"Press any key to begin.\r\n\r\n";

void UserMain(void *pd)
{
    init();
    EnableSmartTraps();

    /* Print a functional description of the application does on the
     *  debug serial port and then wait for the user to start the application.*/
    iprintf(infomsg);
    getchar();

    // Configure all the pin to be used, their IRQ trigger, and their ISRs
    ConfigureIrqPins();

    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND);
        // Reenable DIP switch 3 irq (as our handler will have disabled the irq
        // if it has triggered)
        EnableIrq(dipsw[3]);
        // Display the current counts for how many IRQs have triggered for each
        //  source
        PrintIrqCounts();
    }
}
