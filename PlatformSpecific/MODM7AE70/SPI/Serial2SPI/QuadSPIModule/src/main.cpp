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
 SPI2Serial using QuadSPI in SPI Mode
 This program demonstrates how to use the NetBurner QuadSPI driver in single-bit,
 master SPI mode. This driver provides most of the standard SPI options while
 providing some extra features.
 Some extra features include:
 8-16 bit transfers
 Automation of the queuing system
 Interrupt driven transfers

 In this example we demonstrate how to take advantage of the interrupt feature by
 using a Semaphore.  This will allow any other lower priority tasks to run during
 the transfer. There is also an option to not use a semaphore and poll the
 transfer using the BOOL QuadSPIdone(); functions provided in quadspi.h and dspi.h.

 This example can be used with no external SPI device by placing a jumper across
 P2[43]-P2[47] on the DEV-70/100 carrier board. This will loop-back the SPI data
 and echo the sent character back to the serial terminal.

 The Serial2SPI example can easily be expanded to SPI2TCP by creating a TCP socket
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
#include <quadspi.h>


// Name for development tools to identify this application
const char * AppName="Serial2SPI using QuadSPI in SPI mode";

//Initialize buffers
static uint8_t RXBuffer[10000], TXBuffer[10000];

// Main task
void UserMain( void * pd)
{
    init();

    // Configure QuadSPI pins for SPI mode
    P2[48].function(PINP2_48_QCS);      // Chip Select
    P2[45].function(PINP2_45_QSCK);     // CLOCK
    P2[47].function(PINP2_47_QIO1);     // QSPI MISO
    P2[43].function(PINP2_43_QIO0);     // QSPI MOSI

    // The QuadSPI SPI mode functionality can be tested with a simple jumper
    // from P2[47] to P2[43] on the MOD-DEV70/100

    // Create and initialize semaphore for SPI (optional)
    OS_SEM QUADSPI_SEM;

    /* Initialize QuadSPI peripheral for Single-bit SPI with 2MHz bus speed and default SPI configuration:
     *
     * SPI_QSPI( uint8_t QSPIModule, uint32_t baudRateInBps,
     *         uint8_t transferSizeInBits = 8, uint8_t peripheralChipSelects = 0x00,
     *         uint8_t chipSelectPolarity = 0x0F, uint8_t clockPolarity = 0,
     *         uint8_t clockPhase = 1, BOOL doutHiz = TRUE,
     *         uint8_t csToClockDelay = 0, uint8_t delayAfterTransfer = 0 );
     */
    SPI_QSPI mySPIObject(DEFAULT_QUADSPI_MODULE, 2000000);

    // Register the semaphore with the SPI driver to be able to pend on transaction completion instead of polling.
    mySPIObject.RegisterSem(&QUADSPI_SEM);

    // Open UART1 and get FD
    int uartFd = OpenSerial( 1, 115200, 1, 8, eParityNone );

    iprintf( "%s application started\r\n", AppName );

    while ( 1 )
    {
        if(dataavail(uartFd))
        {
            int num = read(uartFd, (char*)TXBuffer, 10000); // Read data from UART1

            mySPIObject.Start(TXBuffer, RXBuffer, num);     // Write data read from UART1 to the QuadSPI module

            QUADSPI_SEM.Pend( 0 );                          // Wait for SPI transaction to complete

            writeall(uartFd, (char*)RXBuffer, num);         // Send SPI RX via UART1
        }
    }
}
