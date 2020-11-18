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

#include <command.h>
#include <init.h>
#include <iosys.h>
#include <serial.h>
#include <string.h>
#include <tcp.h>

#define SERIALPORT_TO_USE   (0)
#define BAUDRATE_TO_USE     (115200)
#define STOP_BITS           (1)
#define DATA_BITS           (8)
#define TCP_PORT_TO_USE     (23) // The Telnet port

const char *AppName = "TelnetCmd Example";

/**
 * The User Authentication Callback
 */
int ProcessAuth( const char *name, const char *pass )
{
    /* For testing reject the set if they are the same */
    if( strcmp( name, pass ) == 0 )
    {
        return CMD_FAIL;
    }
    else
    {
        return CMD_OK;
    }
}


/**
 * The Command Processing Callback 
 */
int ProcessCommand( const char *command, FILE *fp, void *pData )
{
    if( command[ 0 ] )
    {
        fiprintf( fp, "  # %s Sent Cmd[%s]", (char *)pData, command );
    }

    /* Close the connection if we receive Logout */
    if( strcasecmp( command, "logout" ) == 0 )
    {
        return CMD_CLOSE;
    }

    return CMD_OK;
}


/**
* The Connection Processing Callback
*/
void * ProcessConnect( FILE *fp )
{
    static int SessionNum;
    const char *prompt;
    fiprintf( fp, "Hello!\r\n" );
    fiprintf( fp, "Welcome to the NetBurner Test Command Program\r\n" );
    fiprintf( fp, "Number of connections since boot since boot: %d\r\n", ++SessionNum );

    if( (int)(fp->_file) == (SERIAL_SOCKET_OFFSET) )
    {
        prompt = "Serial0";
    }
    else if( (int)(fp->_file) == (SERIAL_SOCKET_OFFSET + 1) )
    {
        prompt = "Serial1";
    }
    else
    {
        prompt = "Telnet";
    }

    /* To test arbitrary data tracking return the session number */
    return (void *)prompt;
}


/**
* The Prompt Processing Callback
*/
void ProcessPrompt( FILE *fp, void *pData )
{
    /* For fun give each session its type in the prompt */
    fiprintf( fp, "\r\nNB:%s>", (char *)pData );
}


/**
* The Disconnect Processing Callback
*/
void ProcessDisconnect( FILE *fp, int cause, void *pData )
{
    switch( cause )
    {
        case CMD_DIS_CAUSE_TIMEOUT:
        {
            fputs( "\r\nTimed out\r\n\r\n", fp );
            break;
        }
        case CMD_DIS_SOCKET_CLOSED:
        {
            fputs( "Socket closed\r\n\r\n", fp );
            break;
        }
        case CMD_DIS_AUTH_FAILED:
        {
            fputs( "Authentication failed\r\n\r\n", fp );
            break;
        }
        // Intentional fall through
        case CMD_DIS_CAUSE_CLOSED:
        default:
        {
            fputs( "\r\nGoodBye\r\n\r\n", fp );
            break;
        }
    }
}


/**
* UserMain, the entry point of the example
*/
void UserMain( void *pd )
{
    init();

    //Close the serial port in case it is already open.
    SerialClose( SERIALPORT_TO_USE );

    //Open the serial port
    int fdserial = OpenSerial( SERIALPORT_TO_USE, BAUDRATE_TO_USE, STOP_BITS, DATA_BITS, eParityNone );

    // Replace the standard stdio with the serial port we just opened
    ReplaceStdio( 0, fdserial );
    ReplaceStdio( 1, fdserial );
    ReplaceStdio( 2, fdserial );

    writestring( fdserial, "Starting...\r\n\r\n" );

    // Specify the required callback functions
    CmdAuthenticateFunc = ProcessAuth; /* No authentication to start */
    CmdCmd_func         = ProcessCommand;
    CmdConnect_func     = ProcessConnect;
    CmdPrompt_func      = ProcessPrompt;
    CmdDisConnect_func  = ProcessDisconnect;

    CmdIdleTimeout  = TICKS_PER_SECOND * 60;
    Cmdlogin_prompt = "Please Log in\r\n";

    CmdStartCommandProcessor( MAIN_PRIO + 1 );

    // Add the serial FD to the list of FDs that are managed by the command processor 
    CmdAddCommandFd( fdserial, TRUE, TRUE );

    // Specify which port to listen on, if it's a telnet connection, and the max number of connections
    CmdListenOnTcpPort( 23, 1, 5 );

    while( 1 )
    {
        OSTimeDly( TICKS_PER_SECOND );
    }
}
