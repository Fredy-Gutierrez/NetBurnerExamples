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
#include <stdlib.h>
#include <stdio.h>
#include <system.h>
#include <iosys.h>
#include <serial.h>
#include "nvsettings.h"
#include "datapump.h"

// data counters
uint32_t fd1_rxctr = 0;
uint32_t fd2_rxctr = 0;
uint32_t fd1_txctr = 0;
uint32_t fd2_txctr = 0;

/*---------------------------------------------------------------------------------
 *  DataPump
 *
 *  Pump data from fd1 to fd2 and from fd2 to fd1
 *
 *  PARAMETERS
 *  The function takes 3 parameters:
 *  Two file descriptors in which to send/receive data.
 *  The socket file descriptor of the server listening for incoming connections.
 *  Only one connection may be active at a time for this simple example, but it
 *  is certainly possible to lisent to multiple serial and network connections at
 *  the same time.
 *
 *
 *  RETURNS
 *  The function will return for two reasons:
 *
 *  Errors: The function returns when it encounters an error.
 *  The return value is the fd number that caused the error.
 *  The caller is then responsible to for handling the error.
 *
 *  Timeout: The function will return with a value of 0 if nothing
 *  has happened for the number of ticks specified by the
 *  ClientTimeout value specified by the user settings.
 *----------------------------------------------------------------------------------*/
int DataPump(int fd1, int fd2, int serverfd)
{
    // Declare two buffers to hold the data to pump from 1 to the other 
    static char buffer1to2[BUFFER_SIZE];
    static char buffer2to1[BUFFER_SIZE];

    int buf1to2start = 0;   // Pointer to the position in buffer1to2 that has the start of unsent data
    int buf1to2end   = 0;   // Pointer to the position in buffer1to2 that has the end of unsent data
    int buf2to1start = 0;   // Pointer to the position in buffer2to1 that has the start of unsent data
    int buf2to1end   = 0;   // Pointer to the position in buffer2to1 that has the end of unsent data
    int rv;

    NV_SettingsStruct *pData = (NV_SettingsStruct *)GetUserParameters();
    uint32_t overrideTimeout = pData ? pData->ClientOverrideTimeout : DEF_OVERRIDE_TIMEOUT;
    uint32_t totalTimeout    = pData ? pData->ClientTimeout : DEF_INACTIVITY_TIMEOUT;
    uint32_t tcp_timeout     = 0;

    while (1)
    {
        // Set up a file set so we can select on the file descriptors
        fd_set err_fds;
        fd_set read_fds;
        fd_set write_fds;

        FD_ZERO(&read_fds);
        FD_ZERO(&err_fds);
        FD_ZERO(&write_fds);

        // Always check for errors on both ports
        FD_SET(fd1, &err_fds);
        FD_SET(fd2, &err_fds);

        // We only check for a client waiting on the server fd if we can accpet it
        bool allowOveride = (overrideTimeout != 0xFFFFFFFF);
        bool overideExpired = (overrideTimeout == 0) || (tcp_timeout >= (overrideTimeout * TICKS_PER_SECOND));
        if( allowOveride && overideExpired )
        {
            FD_SET( serverfd, &read_fds );
        }

        // In the connection from 1 to 2 are we reading or writing this pass?
        if (buf1to2start != buf1to2end)
        {
            // We have data left to write to fd2
            FD_SET(fd2, &write_fds);
        }
        else
        {
            // We need to read data from fd 1
            FD_SET(fd1, &read_fds);
        }

        // In the connection from 2 to 1 are we reading or writing this pass?
        if (buf2to1start != buf2to1end)
        {
            // We have data left to write to fd1
            FD_SET(fd1, &write_fds);
        }
        else
        {
            // We need to read data from fd2
            FD_SET(fd2, &read_fds);
        }

        // Now actually do the select
        if (select(FD_SETSIZE, &read_fds, &write_fds, &err_fds, TCP_WRITE_TIMEOUT))
        {
            // Data to read on fd1?
            if (FD_ISSET(fd1, &read_fds))
            {
                rv = read(fd1, buffer1to2, BUFFER_SIZE);
                if (rv > 0)
                {
                    buf1to2start = 0;
                    buf1to2end = rv;
                    fd1_rxctr += rv;
                }
            }

            // Data to read on fd2?
            if (FD_ISSET(fd2, &read_fds))
            {
                rv = read(fd2, buffer2to1, BUFFER_SIZE);
                if (rv > 0)
                {
                    buf2to1start = 0;
                    buf2to1end = rv;
                    fd2_rxctr += rv;
                }
            }

            // Data to write on fd1?
            if (FD_ISSET(fd1, &write_fds))
            {
                rv = write(fd1, buffer2to1 + buf2to1start, buf2to1end - buf2to1start);
                if (rv > 0)
                {
                    buf2to1start += rv;
                    fd1_txctr += rv;
                }
            }

            // Data to write on fd2?
            if (FD_ISSET(fd2, &write_fds))
            {
                rv = write(fd2, buffer1to2 + buf1to2start, buf1to2end - buf1to2start);
                if (rv > 0)
                {
                    buf1to2start += rv;
                    fd2_txctr += rv;
                }
            }

            if (FD_ISSET(fd1, &err_fds)) 
                return fd1;
            
            if (FD_ISSET(fd2, &err_fds)) 
                return fd2;

            // This is only checked after the override timeout has expired
            if (FD_ISSET(serverfd, &read_fds))
            {
                // We have a new suitor waiting on the listening socket
                writestring(fd2, MTS_WHEN_CONNECTION_OVERIDDEN);
                writestring(fd1, MTN_WHEN_CONNECTION_OVERIDDEN);
                return 0;
            }
        }
        else if (totalTimeout > 0)  // check for inactivity timeout if not disabled
        {
            tcp_timeout += TCP_WRITE_TIMEOUT;
            if (tcp_timeout >= (totalTimeout * TICKS_PER_SECOND))
            {
                writestring(fd2, MTS_WHEN_CONNECTION_TIMEDOUT);
                writestring(fd1, MTN_WHEN_CONNECTION_TIMEDOUT);
                return 0;
            }
        }
    }   // End of while
    return 0;
}




