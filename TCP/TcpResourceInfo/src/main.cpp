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
#include <stdio.h>
#include <tcp.h>
#include <string.h>
#include <iosys.h>
#include <iointernal.h>
#include <init.h>

const char *AppName = "TCP Resource Info";

extern int GetFreeSocketCount( void );

/*-----------------------------------------------------------------------------
 * Show serial port menu options
 *----------------------------------------------------------------------------*/
void showMenu()
{
    iprintf("1 - Show free buffers and sockets\r\n");
    iprintf("2 - Add a listen socket, display info, and close the socket\r\n");
    iprintf("3 - Get extra fd count\r\n");
    iprintf("\r\n");
}


/*-----------------------------------------------------------------------------
 * Process serial port command
 *----------------------------------------------------------------------------*/
void processCommand(char cmd)
{
    switch (toupper(cmd))
    {
        case '1':
            iprintf("Free buffers: %d, Sockets: %d\r\n", GetFreeCount(), GetFreeSocketCount());
            iprintf("Extra file descriptors: %d\r\n", GetFreeExtraFDCount());
            break;

        case '2':
        {
            iprintf("\r\nAdding a listening socket...");
            int fdListen = listen( INADDR_ANY, 10000, 5 );
            if ( fdListen > 0)
                iprintf("complete\r\n");
            else
                iprintf("error, fd = %d\r\n", fdListen );

            iprintf("Free buffers: %d, Sockets: %d\r\n", GetFreeCount(), GetFreeSocketCount());

            iprintf("Closing the listen socket\r\n");
            close(fdListen);
            iprintf("Free buffers: %d, Sockets: %d\r\n", GetFreeCount(), GetFreeSocketCount());
            break;
        }

        case '3':
            iprintf("Extra FD Count: %d\r\n", GetFreeExtraFDCount());
            break;

        default: showMenu();
    }

    iprintf("\r\n");
}


/*-----------------------------------------------------------------------------
 * UserMain
 *----------------------------------------------------------------------------*/
void UserMain( void *pd )
{
    init();                     // initialize network
    WaitForActiveNetwork(TICKS_PER_SECOND * 10);

    showMenu();
    while (1)
    {
        char c = getchar();
        processCommand(c);
    }
}


