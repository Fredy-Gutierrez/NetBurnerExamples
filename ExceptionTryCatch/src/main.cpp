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

#include <init.h>
#include <nbrtos.h>

#include <iostream>   // Needed for exceptions

const char *AppName = "ExceptionTryCatch";

using namespace std;

void UserMain(void *pd)
{
    init();

    iprintf("Throwing an exception... \r\n");
    try
    {
        throw runtime_error("Error");
    }

    catch (const exception &e)
    {
        cout << "An exception occurred. Exception: " << e.what() << endl;
    }

    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND);
    }
}
