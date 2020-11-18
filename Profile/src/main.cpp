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

#include <ethervars.h>
#include <init.h>
#include <iosys.h>
#include <ipshow.h>

const char *AppName = "Profiler Example";

/**
 *  DisplayMenu
 *
 *  Displays the debug menu
 */
void DisplayMenu(void)
{
    iprintf("\r\n----- Main Menu -----\r\n");
    iprintf("C - Get stats on current task\r\n");
    iprintf("S - Show OS Seconds Counter\r\n");
    iprintf("T - Get stats on all tasks\r\n");
    iprintf("? - Display Menu\r\n");
}

#ifdef NBRTOS_TIME
void displayCurrentTaskTime()
{
    uint32_t taskTicks, totalSystemTicks;

    taskTicks = GetCurrentTaskTime(&totalSystemTicks);
    printf("\r\nCurrent Task Ticks: %lu, Total System Ticks: %lu, Percent: %5.2f\r\n", taskTicks, totalSystemTicks,
           (float)taskTicks / (float)totalSystemTicks);
}
#else
#warning Task time reporting requires the system be built with UCOS_TIME defined
#endif

/**
 *  ProcessCommand
 *
 *  Handle commands with a simple command dispatcher
 */
void ProcessCommand(char cmd)
{
    switch (toupper(cmd))
    {
        case 'C':
#ifdef NBRTOS_TIME
            displayCurrentTaskTime();
#else
            iprintf("\r\nFeature requires that the system be built with NBRTOS_TIME defined.\r\n");
#endif
            iprintf("   Press a key...\r\n");
            getchar();
            break;

        case 'S':
            iprintf("\r\nTick Count = 0x%lX = %ld (%ld seconds)\r\n", TimeTick, TimeTick, TimeTick / TICKS_PER_SECOND);
            iprintf("   Press a key...\r\n");
            getchar();
            break;

        case 'T':
#ifdef NBRTOS_TIME
            iprintf("\r\n");
            ShowTaskTimes();
#else
            iprintf("\r\nFeature requires that the system be built with NBRTOS_TIME defined.\r\n");
#endif
            iprintf("   Press a key...\r\n");
            getchar();
            break;

        default:   // '?'
            iprintf("\r\nInvalid command\r\n");
            break;
    }

    DisplayMenu();
}

/**
 *   IncrementCount
 *
 *   A simple function that increments a count displayed through the serial port
 */
void IncrementCount()
{
    // The Timer parameter is not used here since we do not need to determine which timer interrupted
    static uint32_t gCount = 0;
    static uint32_t lastSecs = Secs;

    if (lastSecs < Secs)
    {
        lastSecs = Secs;
        iprintf("Counter: %5lu", gCount++);
        iprintf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8);
    }
}

/**
 *  UserMain
 */
void UserMain(void *pd)
{
    init();                                       // Initialize network stack
    StartHttp();                                  // Start web server, default port 80
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

    // Wait until DHCP is complete so that we will see valid IP addresses
    InterfaceBlock *pifb = GetInterfaceBlock();
    if (pifb != nullptr)
    {
        while (pifb->dhcpClient.GetDHCPState() != SDHCP_CMPL)
        {
            OSTimeDly(1);
        }
    }

    // Show the connection information
    showIpAddresses();
    DisplayMenu();

    // Wait for a user to input a command, and display our updating counter while we wait.
    while (1)
    {
        if (charavail())
        {
            char c = getchar();
            ProcessCommand(c);
        }

        IncrementCount();
    }
}
