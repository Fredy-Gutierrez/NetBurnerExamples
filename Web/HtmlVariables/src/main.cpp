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
#include <iosys.h>
#include <netinterface.h>
#include <fdprintf.h>

const char *AppName = "HTML Variable Example";

int  gIntVal   = 1234;
char gStrVal[] = "Hello World";

/*
 * Function called from the CPPCALL tag. Display the URL on the web page
 */
void Foo(int fd, PCSTR pUrl)
{
    fdprintf(fd, "The URL value is: %s\r\n", pUrl);
}

/*
 *  Function call with parameters passed from the HTML page. Unlike the CPPCALL tag,
 *  you can use a VARIABLE tag to pass parameters when calling the function.
 *     fd = socket descriptor passed from web server
 *     v = value passed from HTML page
 */
const char *FooWithParameters(int fd, int v)
{
    fdprintf(fd, "This function was called with parameter v = %d\r\n", v);
    return "\0"; // Return a const char * here of zero length so it will not print anything
}

/*
 *  UserMain
 */
void UserMain(void *pd)
{
    init();
    StartHttp();
    WaitForActiveNetwork();

    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND);
    }
}
