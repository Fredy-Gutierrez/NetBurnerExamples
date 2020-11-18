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
#include <startnet.h>
#include <nbtime.h>      // Include for NTP functions
#include <time.h>
#include <init.h>
#include <dns.h>
#include <tcp.h>
#include <netinterface.h>
#include <ipshow.h>
#include <timezones.h>

#if (defined IPV6)
#include <ipv6/ipv6_interface.h>
#endif

#define NTP_SERVER_NAME "pool.ntp.org"  // Use a pool of NTP server addresses rather than just one

void  tzsetchar(char * tzenv);

const char *AppName = "NTP Client Example";
const char *dow_str[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT", "\0" };

/*
 * SetTimeZone()
 *
 * Set time zone from user input. The TZRecords structure is in timezones.h.
 */
void SetTimeZone()
{
    iprintf("Do you want US or International time zones? (U/I)");
    char c = getchar();

    int i = 0;
    int v = 0;

    if ((c == 'I') || (c == 'i'))
    {
        while (TZRecords[i].Posix)  // display time zones
        {
            iprintf("%d: %s, %s\r\n", i, TZRecords[i].Name, TZRecords[i].Description);
            i++;
        }

        scanf("%d", &v);            // get user input
        if (v < i)
            tzsetchar( (char *) TZRecords[v].Posix );     // set time zone
        iprintf("Set Time to[%s]\r\n", TZRecords[i].Name);

    }
    else    // US or Canada
    {
        int nrecs = 0;
        while (TZRecords[i].Posix)      // display time zones
        {
            if (TZRecords[i].bUsCanada)
                iprintf("%d: %s , %s\r\n", nrecs++, TZRecords[i].Name, TZRecords[i].Description);
            i++;
        }

        scanf("%d", &v);                // get user input
        if (v < nrecs)
        {
            nrecs = 0;
            i = 0;
            while (TZRecords[i].Posix)
            {
                if (TZRecords[i].bUsCanada)
                {
                    if (nrecs == v)
                    {
                        tzsetchar((char *) TZRecords[i].Posix);
                        iprintf("Set Time to[%s]\r\n", TZRecords[i].Name);
                        return;
                    }
                    else
                        nrecs++;
                }
                i++;
            }
        }
    }
}


/*
 * PrintTimeStruct()
 *
 * Print the time to stdout, which is the debug serial port by default.
 */
void PrintTimeStruct(struct tm &bt )
{
    iprintf( "Time[%s] = ", _tzname[0] );
    iprintf( "%d/%d/%d day: %d (%s) %02d:%02d:%02d",
             bt.tm_mon+1,
             bt.tm_mday,
             bt.tm_year+1900,
             bt.tm_yday,
             dow_str[bt.tm_wday],
             bt.tm_hour,
             bt.tm_min,
             bt.tm_sec );
    iprintf( "\r\n" );
}


// Format strings for various time zones. Many more in timezones.h and timezones.cpp
const char tzsetFormatString[][120] = {
        {"EST5EDT4,M3.2.0/01:00:00,M11.1.0/02:00:00"},    // eastern
        {"CST6CDT5,M3.2.0/01:00:00,M11.1.0/02:00:00"},    // central
        {"MST7MDT6,M3.2.0/01:00:00,M11.1.0/02:00:00"},    // mountain
        {"PST8PDT7,M3.2.0/01:00:00,M11.1.0/02:00:00"},    // pacific
        {""}
};



/*
 * UserMain()
 *
 * Main entry point of the program.
 */
void UserMain(void *pd)
{
    init();

    iprintf("Application: %s\r\nNNDK Revision: %s\r\n", AppName, GetReleaseTag());

    WaitForActiveNetwork(TICKS_PER_SECOND * 10);
    showIpAddresses();
    iprintf("\r\n\r\n");

    BOOL result = FALSE;
    while (!result)
    {
        IPADDR ipAddress = IPADDR::NullIP();

        iprintf("Resolving NTP server name: %s...", NTP_SERVER_NAME);
        int rv = GetHostByName(NTP_SERVER_NAME, &ipAddress, INADDR_ANY, TICKS_PER_SECOND * 10);
        ipAddress.print();  iprintf("\r\n");
        if (rv == DNS_OK)
        {
            iprintf("Using NTP Server to set system time\r\n");
            result = SetNTPTime(ipAddress);
            if (!result)
            {
                iprintf("SetNTPTime() failed, waiting 30 seconds to try again\r\n");
                OSTimeDly(TICKS_PER_SECOND * 30);
            }
        }
        else
        {
            iprintf("Name resolution failed, %d, waiting 5 seconds to try again\r\n", rv);
            OSTimeDly(TICKS_PER_SECOND * 5);
        }
    }


    SetTimeZone();

    // Display the current system time
    while (1)
    {
        time_t tv = time(NULL);
        struct tm tm_struct;

        localtime_r( &tv, &tm_struct);
        iprintf("\r\nLocal time based on SetTimeZone() function: \r\n");
        PrintTimeStruct(tm_struct);

        /*
         struct tm *gmtime_r(const time_t *clock, struct tm *result);

         Converts a time in seconds since the Epoch (00:00:00 UTC, January 1, 1970)
         into a broken-down time, expressed as Coordinated Universal Time (UTC).

         clock  = The time to be converted
         result = Points to the structure where the converted time is to be stored

         On success, gmtime_r() return result. On error, it returns NULL
         */
        gmtime_r( &tv, &tm_struct);
        iprintf("UTC time = ");
        PrintTimeStruct(tm_struct);

        // Display time with timezone formatting
        iprintf("Displaying US time zones with local format strings: \r\n");
        int i = 0;
        while (tzsetFormatString[i][0] != '\0' )
        {
            tzsetchar((char*) tzsetFormatString[i]);
            localtime_r(&tv, &tm_struct);
            PrintTimeStruct(tm_struct);
            i++;
        }
        iprintf("\r\n");

        iprintf("Hit any key to display time\r\n");
        getchar();
    }
}

