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

// NB Definitions
#include <predef.h>

// NB Libs
#include <ctype.h>
#include <init.h>
#include <nbrtos.h>
#include <nbstring.h>
#include <stdio.h>
#include <string.h>
#include <system.h>

const char *AppName = "NB String Class Test";

void UserMain(void *pd)
{
    init();

    NBString nbs((const char *)"Testing 123");
    NBString nbs2((const char *)"Totally Differnt");

    char buffer[40];
    strcpy(buffer, "This is ram");
    NBString nbsb(buffer);

    iprintf("nbs length =  %u [%s]\r\n", (unsigned int)nbs.length(), nbs.c_str());
    iprintf("nbs2 length = %u [%s]\r\n", (unsigned int)nbs2.length(), nbs2.c_str());
    iprintf("nbsb length = %u [%s]\r\n", (unsigned int)nbsb.length(), nbsb.c_str());

    nbsb = nbs2;
    iprintf("nbsb length = %u [%s]\n", (unsigned int)nbsb.length(), nbsb.c_str());

    nbs += (const char *)"Add Very very long extra stuff";
    iprintf("nbs length = %u [%s]\r\n", (unsigned int)nbs.length(), nbs.c_str());

    nbs += (const char *)"X";
    iprintf("nbs length = %u [%s]\r\n", (unsigned int)nbs.length(), nbs.c_str());

    nbs += nbs2;
    iprintf("nbs length = %u [%s]\r\n", (unsigned int)nbs.length(), nbs.c_str());

    NBString nb3;

    iprintf("nb3 length = %u [%s]\r\n", (unsigned int)nb3.length(), nb3.c_str());

    nb3 = "Testing 3";
    iprintf("nb3 length = %u [%s]\r\n", (unsigned int)nb3.length(), nb3.c_str());

    nb3 = 'X';
    iprintf("nb3 length = %u [%s]\r\n", (unsigned int)nb3.length(), nb3.c_str());

    nb3 = nbs;
    iprintf("nb3 length = %u [%s]\r\n", (unsigned int)nb3.length(), nb3.c_str());

    iprintf("P[11]=%c\n", nb3[11]);

    iprintf("compare nbs,nb3=%d\n", nb3 == nbs);
    iprintf("compare nbs,nb2=%d\n", nb3 == nbs2);
    iprintf("compare nb2,nbs=%d\n", nbs2 == nbs);
    iprintf("compare nb2,tzxt=%d\n", nbs2 == "Totally Differnt");
    iprintf("compare nbs,tzxt=%d\n", nbs == "Totally Differnt");

    iprintf("All done\n");

    nb3 = nbs.substr(20, 4);
    iprintf("nb3 length = %u [%s]\r\n", (unsigned int)nb3.length(), nb3.c_str());

    iprintf("nb3 length = %u [%s]\r\n", (unsigned int)nb3.length(), nb3.c_str());
    iprintf("nb2 length = %u [%s]\r\n", (unsigned int)nbs2.length(), nbs2.c_str());

    swap(nb3, nbs2);

    iprintf("nb3 length = %u [%s]\r\n", (unsigned int)nb3.length(), nb3.c_str());
    iprintf("nb2 length = %u [%s]\r\n", (unsigned int)nbs2.length(), nbs2.c_str());

    nbs.siprintf("This is int text %d", 1234);
    iprintf("nbs length = %u [%s]\r\n", (unsigned int)nbs.length(), nbs.c_str());

    nbs.sprintf("This is float text %g", 1234.5678);
    iprintf("nbs length = %u [%s]\r\n", (unsigned int)nbs.length(), nbs.c_str());

    nb3 = "This is int string %d";
    nbs.siprintf(nb3, 4567);
    iprintf("nbs length = %u [%s]\r\n", (unsigned int)nbs.length(), nbs.c_str());

    nb3 = "This is float string %g";
    nbs.siprintf(nb3, 4567.789);
    iprintf("nbs length = %u [%s]\r\n", (unsigned int)nbs.length(), nbs.c_str());

    nbs.strcopy(buffer, 20);
    iprintf("Buf[%s]\r\n", buffer);

    nbs.strcopy(buffer, 10);
    iprintf("Buf[%s]\r\n", buffer);

    nbs.strcopy(buffer, 10, 2);
    iprintf("Buf[%s]\r\n", buffer);

    nbs = " +1234.5678";

    double d = nbs.stod();
    int i = nbs.stoi();
    long l = nbs.stol();
    unsigned int ui = nbs.stoui();
    unsigned long ul = nbs.stoul();

    nbs = "10.1.1.32";

    // Convert from text to IPADDR
    IPADDR ia1 = nbs.to_ipaddr();

    nbs = "2001:0:9d38:6abd:c29:687:d2e9:3b56";
    IPADDR ia2 = nbs.to_ipaddr();

    printf("d=%g i=%d ui=%u  l=%ld  ul=%lu\r\n", d, i, ui, l, ul);
    ia1.print();
    printf("\r\n");
    ia2.print();
    printf("\r\n");

    printf("All done\r\n");

    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND * 1);
        iprintf("*");
        if ((Secs % 60) == 0) iprintf("\r\n:%ld", Secs);
    }
}
