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

#include <iosys.h>

#include "CommandBuffer.h"

bool CommandBuffer::ReadChar( int fd )
{
    int n = read( fd, &m_commandBuffer[ 0 ], 1 );
    return ( n > 0 ) ? true : false;;
}

void CommandBuffer::ReadString( int fd )
{
    ClearCommandBuffer();

    char *lastReadByte = nullptr;
    char buf[ 10 ];
    int readBytes = 0;

    while( ( lastReadByte == nullptr ) || ( *lastReadByte != '\r' && readBytes <= COMMAND_BUFFER_SIZE ) )
    {
        int n = read( fd, &m_commandBuffer[ readBytes ], COMMAND_BUFFER_SIZE - readBytes );
        write( fd, &m_commandBuffer[ readBytes ], n );
        readBytes += n;
        lastReadByte = &m_commandBuffer[ readBytes - 1 ];
    }

    writestring( fd, "\r\n" );
    if( DEBUG_ENABLED )
    {
        iprintf( "Read Final:  %s\r\n", m_commandBuffer );
    }
}

void CommandBuffer::ClearCommandBuffer()
{
    for( int i = 0; i < COMMAND_BUFFER_SIZE; i++ )
    {
        m_commandBuffer[ i ] = '\0';
    }
}
