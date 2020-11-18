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

#include <constants.h>
#include <ctype.h>
#include <fdprintf.h>
#include <init.h>
#include <iosys.h>
#include <nbrtos.h>
#include <predef.h>
#include <serial.h>

const char *AppName = "Serial fdprintf() Example";

#define DATA_SERIAL_PORT (0)
#define DEBUG_SERIAL_PORT (0)
#define DATA_BAUDRATE (115200)
#define STOP_BITS (1)
#define DATA_BITS (8)

int fddebug = 0;    // file descriptor for serial debug output
int fdserial = 0;   // file descriptor for serial data I/O

void InitializeSerialPorts()
{
    // Open the data serial port
    SerialClose(DATA_SERIAL_PORT);
    fdserial = OpenSerial(DATA_SERIAL_PORT, DATA_BAUDRATE, STOP_BITS, DATA_BITS, eParityNone);

    // Open the debug serial port
    if (DEBUG_SERIAL_PORT != DATA_SERIAL_PORT)
    {
        SerialClose(DEBUG_SERIAL_PORT);
        fddebug = OpenSerial(DEBUG_SERIAL_PORT, DATA_BAUDRATE, STOP_BITS, DATA_BITS, eParityNone);
    }
    else
    {
        fddebug = fdserial;
    }

    // Since we closed the default debug port, we need to assign
    // the stdin/out/error file descriptors.
    ReplaceStdio(0, fddebug);   // stdin
    ReplaceStdio(1, fddebug);   // stdout
    ReplaceStdio(2, fddebug);   // stderr
}

/*-------------------------------------------------------------------
 * UserMain
 * ----------------------------------------------------------------*/
void UserMain(void *pd)
{
    init();
    StartHttp();
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);

    InitializeSerialPorts();

    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND);
        fdprintf(fdserial, "Secs: %ld\r\n", Secs);
    }
}
