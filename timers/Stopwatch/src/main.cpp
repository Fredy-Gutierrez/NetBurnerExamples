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
#include <constants.h>
#include <nbrtos.h>
#include <init.h>
#include <stopwatch.h>

const char *AppName = "Stopwatch Example";


void UserMain( void *pd )
{
    init();
    WaitForActiveNetwork();

    StopWatch myStopwatch;

    volatile uint32_t count = 0;
    const int numLoop = 10000;
    printf("Starting stopwatch on for loop of %d counts...", numLoop);
    myStopwatch.Start();
    for ( int i = 0; i < numLoop; i++ )
    {
        count++;
    }
    myStopwatch.Stop();
    iprintf("complete\r\n");

    unsigned long long elapsedTime = myStopwatch.GetTime();
    printf("Elapsed time: %lld, %e seconds\r\n", elapsedTime, myStopwatch.Convert(elapsedTime) );
    printf("Resolution: %e seconds\r\n", myStopwatch.CountResolution() );

    while ( 1 )
    {
        OSTimeDly(TICKS_PER_SECOND);
    }
}
