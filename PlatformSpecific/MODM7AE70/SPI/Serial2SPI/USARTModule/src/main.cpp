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
 SPI2Serial using USART in SPI mode
 This program demonstrates how to use the NetBurner USART driver in single-bit,
 master SPI mode. This driver provides most of the standard SPI options.
 8-bit transfers
 Automation of the queuing system
 Interrupt driven transfers

 In this example we demonstrate how to take advantage of the interrupt feature by
 using a Semaphore.  This will allow any other lower priority tasks to run during
 the transfer. There is also an option to not use a semaphore and poll the
 transfer using the BOOL DSPIdone(); functions provided in usart.h and dspi.h.

 This example can be used with no external SPI device by placing a jumper across
 P2[2]-P2[3] for USART0 or P2[21]-P2[22] for USART1 on the DEV-70/100 carrier board.
 This will loop-back the SPI data and echo the sent character back to the serial
 terminal.

 There are two USARTS available to be configured for SPI mode on the MODM7AE70.
 This example is designed to test one of the USARTS in SPI mode at a time. This is
 because we need to use the other USART as a serial port for print statements and
 input from the user.

 If you're using a MOD-DEV-70CR and you want to test USART0 in SPI mode, you will
 need to remove jumpers JP2 and JP3 to disconnect the hardware that connects USART0
 on the MODM7AE70 to UART0 on the MOD-DEV-70CR.

 If you're using a MOD-DEV-70CR and you want to test USART1 in SPI mode, you may find
 it easier to not mount your MODM7AE70 in the MOD-DEV-70CR and simply jumper the power
 and UART0 pins to the MOD-DEV-70CR. This is because USART1 of the MODM7AE70 would be
 connected to UART1 hardware on the MOD-DEV-70CR and it cannot be bypassed otherwise.

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
#include <usart.h>

/* Only uncomment one of these macros at a time. This example will configure one USART
 * for SPI and the other USART for serial I/O.
 */
#define CONFIGURE_USART_0_SPI_MODE     (1)
// #define CONFIGURE_USART_1_SPI_MODE     (1)

// Name for development tools to identify this application
const char * AppName="Serial2SPI using USART in SPI mode";


//Initialize buffers
static uint8_t RXBuffer[10000], TXBuffer[10000];

// Main task
void UserMain( void * pd)
{
    init();

    /* The RTOS initializes USARTS for serial I/O in the init() call above. We need to close
     * the serial ports so that we can reconfigure the USART for SPI mode.
     */
    SerialClose(0);
    SerialClose(1);

#ifdef CONFIGURE_USART_0_SPI_MODE
    // Open serial port on USART1 and get FD
    int uartFd = OpenSerial( 1, 115200, 1, 8, eParityNone );
#endif
#ifdef CONFIGURE_USART_1_SPI_MODE
    // Open serial port on USART0 and get FD
    int uartFd = OpenSerial( 0, 115200, 1, 8, eParityNone );
#endif

/* The USART modules on the MODM7AE70 are used for serial communication to the UART ports on
 * the MOD-DEV-70CR development board. You will need to do a couple things to reconfigure
 * the USART module for SPI mode to prevent contention with the serial output. First, you will
 * need to close the serial port since the RTOS opens up the serial port for serial print statements
 * upon calling init(). The serial port opened by the RTOS will depend on the value of the "BootUart"
 * config variable. If you want to test USART0 in SPI mode, you may want to configure your "BootUart"
 * to UART1 instead of the default UART0. Then all print statements will come out UART1 on the
 * MOD-DEV-70CR.
 */
#ifdef CONFIGURE_USART_0_SPI_MODE
    // configure USART0 pins for SPI mode
    P2[38].function(PINP2_38_RTS0);     // USART0 Chip Select in SPI Master Mode
    P2[11].function(PINP2_11_SCK0);     // USART0 Serial Clock
    P2[3].function(PINP2_3_RXD0);       // USART0 SPI mode MISO in SPI Master Mode
    P2[4].function(PINP2_4_TXD0);       // USART0 SPI mode MOSI in SPI Master Mode
#endif

#ifdef CONFIGURE_USART_1_SPI_MODE
    // configure USART1 pins for SPI mode
    P2[32].function(PINP2_32_RTS1);     // USART1 Chip Select in SPI Master Mode
    P2[31].function(PINP2_31_SCK1);     // USART1 Serial Clock
    P2[21].function(PINP2_21_RXD1);     // USART1 SPI mode MISO in SPI Master Mode
    P2[22].function(PINP2_22_TXD1);     // USART1 SPI mode MOSI in SPI Master Mode
#endif

    // The SPI functionality can be tested with a simple jumper
    // from P2[3] to P2[4] for USART0 or P2[21] to P2[22] for USART1 on the MOD-DEV70/100.

    // Create and initialize semaphore for USART SPI transactions (optional)
    OS_SEM USART_SPI_SEM;

    /* Initialize USART peripheral for Single-bit SPI mode with 2MHz bus speed:
     *
     * SPI_USART( uint8_t USARTModule, uint32_t baudRateInBps,
     *         uint8_t transferSizeInBits = 8, uint8_t peripheralChipSelects = 0x00,
     *         uint8_t chipSelectPolarity = 0x0F, uint8_t clockPolarity = 0,
     *         uint8_t clockPhase = 1, BOOL doutHiz = TRUE,
     *         uint8_t csToClockDelay = 0, uint8_t delayAfterTransfer = 0 );
     */
#ifdef CONFIGURE_USART_0_SPI_MODE
    SPI_USART mySPIObject(0, 2000000);      // Initialize USART0 in SPI mode with 2MHz bus speed
#endif

#ifdef CONFIGURE_USART_1_SPI_MODE
    SPI_USART mySPIObject(1, 2000000);      // Initialize USART1 in SPI mode with 2MHz bus speed
#endif

    // Register the semaphore with the SPI driver to be able to pend on transaction completion instead of polling.
    mySPIObject.RegisterSem(&USART_SPI_SEM);

    iprintf( "%s application started\r\n", AppName );

    while ( 1 )
    {
        if(dataavail(uartFd))
        {
            int num = read(uartFd, (char*)TXBuffer, 10000); // Read data from UART1

            mySPIObject.Start(TXBuffer, RXBuffer, num);     // Send data via SPI

            USART_SPI_SEM.Pend( 0 );                        // Wait for SPI to complete

            writeall(uartFd, (char*)RXBuffer, num);         // Send DSPI RX via UART1
        }
    }
}
