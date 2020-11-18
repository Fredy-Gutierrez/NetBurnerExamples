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
#include <init.h>
#include <smarttrap.h>
#include <nbrtos.h>
#include <stdio.h>
#include <sim.h>
#include <hal.h>
#include <config_obj.h>
#include <same70_wdt.h>

/******************************************************************************/
/*  @file                                                                     */
/*  @brief Watchdog example for the MODM7AE70.                                */
/*                                                                            */
/*  @mainpage MODM7AE70 Watchdog Example                                      */
/*                                                                            */
/*  @section Purpose                                                          */
/*                                                                            */
/*  This example demonstrates how to use the Watchdog on the SAME70.          */
/*                                                                            */
/*  @section Description                                                      */
/*                                                                            */
/*  Refer to chapter 24 (Watchdog Timer) of the SAME70 Reference Manual for   */
/*  details on the watchdog timer. The SAME70 Reference Manual can be found   */
/*  in /nburn/docs/microchip/SAM-E70S70V70V71_RM.pdf of your NNDK install.    */
/*                                                                            */
/*  The Watchdog Timer Mode Register, used to enable/disable/configure the    */
/*  watchdog timer, can only be written to once after boot. Read access is    */
/*  always possible. The watchdog timer on the SAME70 is enabled on boot by   */
/*  default by design of the processor. The NBRTOS will disable the watchdog  */
/*  timer when initializing the system if the watchdog_enabled config         */
/*  variable is set to false. Thus, before enabling/configuring the watchdog, */
/*  the user must set watchdog_enabled config variable to true to prevent the */
/*  NBRTOS from writing to the Watchdog Timer Mode Register. This logic is    */
/*  performed in this example in the beginning of UserMain().                 */
/*                                                                            */
/*  If you find that you accidentally selected a counter value that is too    */
/*  small or you don't service the watchdog often enough, your MODM7AE70 will */
/*  be stuck in a boot loop due to the watchdog reseting. The "Recovery"      */
/*  jumper located on the top of the MODM7AE70 can be used to flash a         */
/*  recovery application to get your module back to a stable state. For more  */
/*  details on how to recover your MODM7AE70, see the section titled          */
/*  "Recovery for MODM7AE70 Module" in the NNDK 3.0 documentation. The NNDK   */
/*  3.0 documentation can be found locally in your NNDK install in            */
/*  /nburn/docs/NetBurner/Developer/html/index.html or on our website at      */
/*  https://www.netburner.com/NBDocs/Developer/html/index.html                */
/*                                                                            */
/******************************************************************************/

const char *AppName = "SAME70 Watchdog";

extern "C"
{
   void UserMain( void *pd );
};

extern config_bool watchdog_enabled;

inline void serviceWatchdog()
{
    WDT->WDT_CR = WDT_CR_KEY_PASSWD | WDT_CR_WDRSTT;
}

/* A valid counterValue can range from 0x1 to 0xFFF. See the WDV (Watchdog Counter Value)
 * description in section 24.5.2 (Watchdog Timer Mode Register) of the
 * SAME70 Reference Manual for details on the counter value.
 * Note: Enabling/Disabling the watchdog can only be done ONCE after boot. Writing to
 * the watchdog timer mode register prevents any further writes to the register. */
void enableWatchdog(uint16_t counterValue )
{
    uint32_t wdt_mr = 0;

    if(counterValue > 0xFFF) { counterValue = 0xFFF; }
    if(counterValue < 0x001) { counterValue = 0x001; }

    /* Assign the watchdog_service_function function pointer to your custom function
    that services the watchdog. This will allow the NBRTOS to service the watchdog
    during system processes, such as during the process of writing a new application to flash. */
    watchdog_service_function = &serviceWatchdog;

    /* See section 24.5.2 for details on the Watchdog Timer Mode Register. We intentionally
     * configure the watchdog to run while in idle and debug states. */
    wdt_mr =  WDT_MR_WDD(0xFFF) | WDT_MR_WDRSTEN | counterValue;

    WDT->WDT_MR = wdt_mr;
}

void UserMain(void *pd)
{
    init();
    EnableSmartTraps();

    if(watchdog_enabled == false)
    {
        iprintf("Reconfiguring MODM7AE70 watchdog to be enabled on boot and rebooting the device\r\n");
        // monitor_config.watchdog_enabled = true;
        watchdog_enabled = true;
        SaveConfigToStorage();  // Save the change to watchdog_enabled config variable to flash

        OSTimeDly(TICKS_PER_SECOND);
        ForceReboot();          // The NBRTOS will not write to the Watchdog Timer Mode Register on next boot
    }
    else
    {
        iprintf("The MODM7AE70 watchdog is configured to be enabled on boot\r\n");
    }

    enableWatchdog(0xFFF);      // 0xFFF counter value is ~16 second timeout

    while (1)
    {
        iprintf("seconds = %d\r", TimeTick/TICKS_PER_SECOND);
        serviceWatchdog();
        OSTimeDly(TICKS_PER_SECOND/10);
    }
}
