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

const char *AppName = "OSCrit Example";   // Program name
int GlobalVariable = 0;                   // A shared resource

void UserMain(void *pd)
{
    init();
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

    //===== Implement critical section with OSCritEnter() & OSCritLeave() =====
    OS_CRIT cs;   // The critical section identifer

    // The second parameter is a timeout value; the number of time ticks to wait for
    // "cs" to be available if another task currently ownes it. A value of 0 waits forever.
    cs.Enter();

    GlobalVariable = 1;

    // You need to call OSCritLeave() for each call to OSCritEnter(). While easy to see
    // in this example, in larger applications it becomes easier to get out of sync.
    cs.Leave();
    iprintf("GlobalVariable = %d\r\n", GlobalVariable);

    //===== Implement critical section with a critical C++ object =====
    OS_CRIT cs2;   // The critical section identifer

    {   // Opening scope operator. Now create an OSCriticalSectionObj object. When the
        // object is created the constructor will be called, which will call OSCritEnter().
        // This call always sets the timeout to wait forever.
        OSCriticalSectionObj UserMainCriticalSection(cs2);
        GlobalVariable = 2;

        // Note that the OSCritLeave() call is not required

    }   // destructor is called when the critical section object goes out of scope

    iprintf("GlobalVariable = %d\r\n", GlobalVariable);

    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND * 5);
    }
}
