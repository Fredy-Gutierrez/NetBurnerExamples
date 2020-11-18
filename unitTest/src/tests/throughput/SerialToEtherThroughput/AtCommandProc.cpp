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
#include <iosys.h>
#include <nbrtos.h>

#include "AtCommandProc.h"

extern int gFdSerial;
extern TestModes gConnectionMode;

bool AtCommandProc::SetMode( TestModes mode )
{
    // Prevent us from sending any data for one second in order to initiate the AT command response
    char buf[ 80 ];
    for( int i = 0; i < 80; i++ )
    {
        buf[ i ] = '\0';
    }

    bool suc = InitiateAtCommands();
    if( suc )
    {
        int n = 0;
        switch( mode )
        {
            case eTestModeTCP:
            {
                n = write( gFdSerial, "AT#SYSMD=T\r", 11 );
                OSTimeDly( TICKS_PER_SECOND );
                n = write( gFdSerial, "AT#SER1SP=1234\r", 15 );
                OSTimeDly( TICKS_PER_SECOND );
                n = write( gFdSerial, "AT#SER1CP=1234\r", 15 );
                break;
            }
            case eTestModeUDP:
            {
                n = write( gFdSerial, "AT#SYSMD=U\r", 11 );
                OSTimeDly( TICKS_PER_SECOND );
                n = write( gFdSerial, "AT#SER1SP=1234\r", 15 );
                OSTimeDly( TICKS_PER_SECOND );
                n = write( gFdSerial, "AT#SER1CP=1234\r", 15 );
                break;
        }
        }

        if( n < 11 )
        {
            iprintf( "Error: Unable to set connection mode to %d, wrote %d bytes.\r\n", mode, n );
            write( gFdSerial, "AT&X\r", 5 );
            return false;
        }

        //try to save without the read and see if it works.  since it is so few bytes, might be loosing it
        int totalRead = 0;
        n = -1;
        while( ( n != 0 ) && ( totalRead < 80 ) )
        {
            n = ReadWithTimeout( gFdSerial, &buf[ totalRead ], 80, TICKS_PER_SECOND );
            totalRead += n;
        }
        
        if( totalRead > 0 )
        {
            // Last part of read serial data should be "OK>", so we validate it
            if( (buf[ totalRead - 2 ] == 'O') && (buf[ totalRead - 1 ] == 'K') )
            {
                iprintf( "Mode successfully changed to: %s\r\n", (mode == eTestModeTCP) ? "TCP" : "UDP" );
                n = write( gFdSerial, "AT&P\r", 5 );
                if( n < 5 )
                {
                    iprintf( "Error: Unable to save connection mode to %d\r\n", mode );
                    return false;
                }
                else
                {
                    gConnectionMode = mode;
                    return true;
                }
            }
        
            if( DEBUG_ENABLED )
            {
                iprintf( "Serial received of %d bytes: \n%s\r\n", totalRead, buf );
            }
        }
        else
        {
            iprintf( "Error: Did not receive confirmation that mode %d was set: %s\r\n", mode, buf );
        }
    }
    
    return false;
}

bool AtCommandProc::InitiateAtCommands()
{
    if( gFdSerial < 0 )
    {
        iprintf( "ERROR: Device is not connected, unable to initiate AT commands.\r\n" );
        return false;
    }

    // Prevent us from sending any data for one second in order to initiate the AT command response
    char buf[ 80 ];
    for( int i = 0; i < 80; i++ )
    {
        buf[ i ] = '\0';
    }

    // This doesn't delay correctly
    iprintf( "Initiating AT command mode..." );
    OSTimeDly( TICKS_PER_SECOND );

    int n = write( gFdSerial, "+++", 3 );
    if( n <= 0 )
    {
        iprintf( "Error: Unable to send serial data\r\n" );
        return false;
    }

    int totalRead = 0;
    n = -1;
    while( ( n != 0 ) && ( totalRead < 80 ) )
    {
        n = ReadWithTimeout( gFdSerial, &buf[ totalRead ], 80, TICKS_PER_SECOND );
        totalRead += n;
    }

    if( totalRead > 0 )
    {
        // Last part of read serial data should be "OK>", so we validate it
        if( ( buf[ totalRead - 3 ] == 'O' ) && ( buf[ totalRead - 2 ] == 'K' ) && (buf[ totalRead - 1 ] == '>') )
        {
            iprintf( " success.\r\n" );
            return true;
        }

        if( DEBUG_ENABLED )
        {
            iprintf( "Serial received of %d bytes: \n%s\r\n", totalRead, buf );
        }
    }
    else
    {
        iprintf( "Error: Did not receive serial response, got error: %d and buf %s\r\n", n, buf );
    }

    iprintf( " failed.\r\n" );
    return false;
}
