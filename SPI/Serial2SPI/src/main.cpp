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


/******************************************************************************
 DSPI2Serial
 This program will illustrate how to use the NetBurner DSPI driver.  This driver
 provides most of the standard DSPI options while providing some extra features.
 Some extra features include:
 4-32 bit transfers instead of the standard 4-16 bit
 Automation of the queuing system
 Interrupt driven transfers

 In this example we demonstrate how to take advantage of the interrupt feature by
 using a Semaphore.  This will allow any other lower priority tasks to run during
 the transfer. There is also an option to not use a semaphore and poll the
 transfer using the BOOL DSPIdone(); function provided in qspi.h.

 This example can be used with no external SPI device by placing a jumper across
 J2[27]-J2[28] on the DEV-70/100 carrier board or Pins[33]-Pins[35] for the
 Nano54415 dev board.  This will loop-back the DSPI data and echo the sent
 character back to the serial terminal.

 The DSPI2Serial example can easily be expanded to DSPI2TCP by creating a TCP socket
 file descriptor similar to the TCP2Serial example.  This TCP file descriptor would
 then used in place of the serial file descriptor.
*****************************************************************************/

#include <predef.h>
#include <stdio.h>
#include <ctype.h>
#include <init.h>
#include <iosys.h>
#include <serial.h>
#include <pins.h>
#include <dspi.h>


// Name for development tools to identify this application
const char * AppName="Serial2SPI";

// Main task
void UserMain( void * pd)
{
    init();

    iprintf( "%s application started\r\n", AppName );

    //Initialize buffers
    static uint8_t RXBuffer[10000], TXBuffer[10000];

    // Initialize pins needed for DSPI
#ifdef NANO54415
    //    Pins[15].function(PIN_15_DSPI1_PCS0);
    Pins[31].function(PIN_31_DSPI1_SCK);
    Pins[33].function(PIN_33_DSPI1_SIN);
    Pins[35].function(PIN_35_DSPI1_SOUT);
#elif defined MOD5441X
    //    J2[30].function(PINJ2_30_DSPI1_PCS0);
    J2[25].function(PINJ2_25_DSPI1_SCK);
    J2[27].function(PINJ2_27_DSPI1_SIN);
    J2[28].function(PINJ2_28_DSPI1_SOUT);
#elif defined MODM7AE70
    //    P2[30].function(PINP2_30_SPI0_NPCS2);
    P2[25].function(PINP2_25_SPI0_SPCK);
    P2[27].function(PINP2_27_SPI0_MISO);
    P2[28].function(PINP2_28_SPI0_MOSI);
#endif
    // The DSPI functionality can be tested with a simple jumper
    // from J2[27] to J2[28] on the MOD-DEV70/100 or from Pins[33] to Pins[35]
    // on the NANO54415 carrier board.

    // Create and initialize semaphore for DSPI (optional)
    OS_SEM DSPI_SEM;

    /* Initialize DSPI options
    void DSPIInit( uint8_t SPIModule = DEFAULT_DSPI_MODLE, DWORD Baudrate = 2000000,
                uint8_t QueueBitSize = 0x8, uint8_t CS = 0x0F,
                uint8_t CSPol = 0x0F, uint8_t ClkPolarity = 0x0,
                uint8_t ClkPhase = 0x1, BOOL DoutHiz = TRUE,
                uint8_t csToClockDelay = 0, uint8_t delayAfterTransfer = 0 )

    DEFAULT_DSPI_MODULE is defined in <dspi.h>, and is use to selet the
    default DSPI module if no argument is given. The standard implementation
    defines this macro to be 1
    */

    // QSPIInit(); <- this is the interface for the SPI driver on MCF52XX parts
    //                  You can still use this interface on the MCF5441X parts,
    //                  but the driver will always use the default DSPI module
    DSPIInit();  // Keep overloaded defaults

    // Open UART1 and get FD
    int U1FD = OpenSerial( 1, 115200, 1, 8, eParityNone );

    while ( 1 )
    {
        if(dataavail(U1FD))
        {
            int num = read(U1FD, (char*)TXBuffer, 10000);   // Read data from UART1

            // QSPIStart(TXBuffer, RXBuffer, num, &QSPI_SEM);
            //
            //  ^ this is the interface for the SPI driver
            //  on MCF52XX parts, same note as before

            DSPIStart(DEFAULT_DSPI_MODULE, TXBuffer, RXBuffer, num, &DSPI_SEM);   // Send data via DSPI
            DSPI_SEM.Pend( 0 );                             // Wait for DSPI to complete
            writeall(U1FD, (char*)RXBuffer, num);           // Send DSPI RX via UART1
        }
    }
}
