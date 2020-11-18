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

/*------------------------------------------------------------------------------
 * Example to show how to intercept the serial port receive processing.
 *----------------------------------------------------------------------------*/
#include <predef.h>
#include <basictypes.h>
#include <buffers.h>
#include <serinternal.h>
#include <constants.h>
#include <system.h>
#include <init.h>
#include <nbrtos.h>
#include <iosys.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <serial.h>
#include <pins.h>


extern "C" {
    void UserMain(void *pd);
}

const char * AppName = "Serial Receive Callback";

#define RX_BUF_SIZE 256

volatile uint8_t indexGet;
volatile uint8_t indexPut;
volatile uint8_t CircBufferArray[RX_BUF_SIZE];

/*-----------------------------------------------------------------------------
 * This function is called by the serial interrupt service routine. No I/O
 * functions may be called from here. Only RTOS post functions may be called.
 * ---------------------------------------------------------------------------*/
void SerialReceiveCallback(int uartNum, unsigned char dataByte)
{
    CircBufferArray[indexPut++] = dataByte;

    if ( indexGet == indexPut )
        indexGet++;

    if (indexPut >= RX_BUF_SIZE)
        indexPut = 0;

    if (indexGet >= RX_BUF_SIZE)
        indexGet = 0;

}

/*-------------------------------------------------------------------
 * UserMain
 *-----------------------------------------------------------------*/
void UserMain(void *pd)
{
    init();

    // Open serial port
    int fd_ser = SimpleOpenSerial(1, 115200);

    // Assign UART1 serial receive processing to our callback function
    UartData[1].m_pPutCharFunc = SerialReceiveCallback;

    iprintf( "Application built on %s on %s\r\n", __TIME__, __DATE__ );
    char c = 0;
    while ( 1 )
    {
        OSTimeDly( 1 );
        while ( indexGet != indexPut )
        {
            iprintf( "%c", CircBufferArray[indexGet++] );

            if ( indexGet >= RX_BUF_SIZE )
                indexGet = 0;
        }
    }
}

