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
#include <hal.h>
#include <ctype.h>
#include <init.h>
#include <nbrtos.h>
#include <nettypes.h>
#include <nbtime.h>
#include <stdio.h>
#include <string.h>

//#include <..\..\drivers\RTC-NXPPCF8563\include\rtc.h>
//#if ( defined(MOD5441X) )
//#include <..\..\platform\MOD5441X\include\rtc.h>
//#endif
//
//#if ( defined(MODM7AE70) )
//#include <..\..\platform\MOD5441X\include\rtc.h>
//#endif

#include <rtc.h>

const char *AppName = "NTP External RTC Example";

/**
 * Gets the current UTC time from an available time server in the NTP pool at
 * pool.ntp.org and sets the system time
 */
int syncSystemTimewithNTP()
{
    // Note the TRUE parameter is optional - it sends status messages to stdout
    if (SetTimeNTPFromPool(TRUE))
    {
        iprintf("\r\nSTATUS -- System time sync with NTP server successful\r\n");
        return 0;
    }
    else
    {
        iprintf("\r\nERROR  -- System time sync with NTP server failed\r\n");
        return -1;
    }
}

/**
 * Synchronize the real-time clock with the current system time
 */
int syncRTCTimewithSystem()
{
    if (RTCSetRTCfromSystemTime() == 0)
    {
        iprintf("\r\nSTATUS -- RTC sync with system successful\r\n");
        return 0;
    }
    else
    {
        iprintf("\r\nERROR  -- RTC sync with system failed\r\n");
        return -1;
    }
}

/**
 * Synchronize the system time with the current real-time clock time
 */
int syncSystemTimewithRTC()
{
    if (RTCSetSystemFromRTCTime() == 0)
    {
        iprintf("\r\nSTATUS -- System sync with RTC successful\r\n");
        return 0;
    }
    else
    {
        iprintf("\r\nERROR  -- System sync with RTC failed\r\n");
        return -1;
    }
}

/**
 * Check for synchronization between system time and RTC time - synchronization
 * passes if both times are the same down the minute, excluding seconds
 */
BOOL checkSystemandRTCTimeSync()
{
    time_t tt_sys;       // System time in time_t format
    struct tm stm_sys;   // System time in struct tm format
    struct tm stm_rtc;   // RTC time in struct tm format

    if (RTCGetTime(stm_rtc) == 0)   // Read the RTC time and store in struct tm format
    {
        tt_sys = time(NULL);           // Read the current system time
        gmtime_r(&tt_sys, &stm_sys);   // Store system time in struct tm format

        //
        // Check for synchronization between RTC and system time
        //
        if ((stm_sys.tm_min != stm_rtc.tm_min) || (stm_sys.tm_hour != stm_rtc.tm_hour) || (stm_sys.tm_mday != stm_rtc.tm_mday) ||
            (stm_sys.tm_mon != stm_rtc.tm_mon) || (stm_sys.tm_year != stm_rtc.tm_year))
        {
            iprintf("\r\nSTATUS -- RTC and system are not in sync\r\n");
            return FALSE;
        }
        else
        {
            iprintf("\r\nSTATUS -- RTC is already in sync with system\r\n");
            return TRUE;
        }
    }
    else
    {
        iprintf("\r\nERROR  -- Unable to read time from RTC\r\n");
        return FALSE;
    }
}

/**
 * Display the currently stored system and RTC time
 */
void displaySystemandRTCTimes()
{
    iprintf("\r\n*** System Time Information ***\r\n");

    time_t tt;             // Stores the system time in time_t format
    struct tm stm_utc;     // Stores UTC time as struct tm
    struct tm stm_local;   // Stores local time as struct tm
    char buffer[80];       // For displaying time strings

    tt = time(NULL);   // Get system time as time_t

    //
    // Convert system time as UTC from time_t to struct tm format, then display
    // the UTC time
    //
    gmtime_r(&tt, &stm_utc);
    strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S", &stm_utc);
    iprintf("UTC Time   = %s\r\n", buffer);

    //
    // Convert system time as local from time_t to struct tm format, then
    // display the local time
    //
    localtime_r(&tt, &stm_local);
    strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S [%z offset of UTC]", &stm_local);
    iprintf("Local Time = %s\r\n", buffer);

    /***************************************************/

    iprintf("\r\n*** Real-Time Clock Time Information ***\r\n");

    struct tm stm_rtc;

    RTCGetTime(stm_rtc);
    strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S (UTC time)", &stm_rtc);
    iprintf("RTC Time   = %s\r\n", buffer);

    /***************************************************/

    //
    // Display configured local time zone information
    //
    iprintf("\r\n*** Local Time Zone Information ***\r\n");
    iprintf("Local time zone offset (in seconds):  %9ld\r\n", -_timezone);
    iprintf("Local standard time abbreviation:  %12s\r\n", _tzname[0]);
    if (_daylight == 1)
        iprintf("Local daylight saving time abbreviation:  %5s\r\n", _tzname[1]);
    else
        iprintf("\r\n");
}

/**
 * Software-resets the device. This feature is useful in verifying whether the
 * RTC can retain set time information
 */
void rebootDevice()
{
    iprintf("\r\nSTATUS -- Rebooting the device...\r\n");
    OSTimeDly(1);
    ForceReboot();
}

/**
 * Displays the menu of numeric options made available by this application
 */
void displayMenu()
{
    iprintf("\r\n***** NTP-System-RTC Demo Main Menu *****\r\n");
    iprintf("[1] Sync the system time with NTP (DNS required)\r\n");
    iprintf("[2] Synchronize real-time clock (RTC) from system time\r\n");
    iprintf("[3] Synchronize system time from RTC\r\n");
    iprintf("[4] Verify synchronization between system and RTC\r\n");
    iprintf("[5] Display current system and RTC times\r\n");
    iprintf("[6] Re-display menu\r\n");
    iprintf("[0] Software-reset the device\r\n");
}

/**
 * Executes an action based on the numeric option entered by the user
 */
void processCommand(char command)
{
    iprintf("\r\n");

    switch (command)
    {
        case '1': syncSystemTimewithNTP(); break;
        case '2': syncRTCTimewithSystem(); break;
        case '3': syncSystemTimewithRTC(); break;
        case '4': checkSystemandRTCTimeSync(); break;
        case '5': displaySystemandRTCTimes(); break;
        case '6': displayMenu(); break;
        case '0': rebootDevice(); break;
        default: iprintf("\r\nERROR  -- Invalid option!\r\n"); displayMenu();
    }
}

/**
 * The main task
 */
void UserMain(void *pd)
{
    init();

    iprintf("Starting NTP External RTC Example\r\n");

    //
    // Set the TZ environment variable to PST/PDT local time zone
    //     - PST: Pacific Standard Time (UTC ahead of PST by +8 hours)
    //     - PDT: Pacific Daylight Time (UTC ahead of PDT by +7 hours)
    //     - PDT starts on the 2nd (2) Sunday (0) of March (3) at 02:00
    //     - PDT ends on the 1st (1) Sunday (0) of November (11) at 02:00
    //
    // To properly set the TZ environment variable to your local time zone, see
    // the NetBurner Runtime Libraries document, section 14.7, "tzsetchar"
    //
    char tzInfo[] = "PST8PDT7,M3.2.0/02:00:00,M11.1.0/02:00:00";
    tzsetchar(tzInfo);

    //
    // Display time zone settings set by the tzsetchar() call and display
    // daylight saving time zone if available
    //
    iprintf("STATUS -- TZ environment variable set for %s", _tzname[0]);
    (_daylight == 1) ? iprintf("/%s ", _tzname[1]) : iprintf(" ");
    iprintf("time zone\r\n");

    displayMenu();   // Display menu options on start-up

    char cmdOption;
    while (1)
    {
        iprintf("\r\nEnter numeric option:  ");
        cmdOption = getchar();
        processCommand(cmdOption);
    }
}
