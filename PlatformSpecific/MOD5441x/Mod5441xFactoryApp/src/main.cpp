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

/*------------------------------------------------------------------------------
 * MOD54415 FACTORY DEMO PROGRAM
 *----------------------------------------------------------------------------*/

#include <init.h>
#include <nbrtos.h>
#include <stdio.h>
#include <http.h>
#include <iosys.h>
#include <ftpd.h>
#include <effs_fat/fat.h>
#include <serial.h>
#include "webfuncs.h"
#include "tests.h"
#include "ip_util.h"

const char *AppName = "MOD54415 Factory Demo";

/*
 * Diagnostic functions used in the command dispatcher
 */
void dEnet(void);
void ShowArp(void);
void ShowCounters(void);


/**
 * Display debug menu
 */
void displayMenu(void)
{
    iprintf("\r\n----- Main Menu -----\r\n");
    iprintf("A - Show ARP Cache\r\n");
    iprintf("B - Show IP Addresses\r\n");
    iprintf("C - Show Counters\r\n");
    iprintf("E - Show Ethernet Registers\r\n");
    iprintf("W - Show OS Seconds Counter\r\n");
    iprintf("? - Display Menu\r\n");
    showLinkStatus();
}


/**
 * Handle commands with a simple command dispatcher
 */
void processCommand(char *buffer)
{
    switch (toupper(buffer[0]))
    {
      case 'A':
        ShowArp();
        break;

      case 'B':
        showIpAddressesSerial();
        break;

      case 'C':
        ShowCounters();
        break;

      case 'E':
        dEnet();
        break;

      case '~':
        doManfTest();
        break;

      case '`':
        doDualMemCardTest();
        break;

      case 'W':
        iprintf("Tick Count = 0x%lX = %ld (%ld seconds)\r\n",
                TimeTick, TimeTick, TimeTick / TICKS_PER_SECOND);
        break;
      default: // '?'
        displayMenu();
    }
}

/**
 * The main task
 */
void UserMain(void *pd)
{
	init();
	WaitForActiveNetwork();

    showIpAddressesSerial();
    StartHttp();

    /*
     * The following call to f_enterFS() must be called in every task that
     * accesses the file system.  This must only be called once in each task
     * and must be done before any other file system functions are used.  Up to
     * 10 tasks can be assigned to use the file system.  Any task may also be
     * removed from accessing the file system with a call to the function
     * f_releaseFS().
     */
    f_enterFS();

    FTPDStart(21,  MAIN_PRIO - 2);

    iprintf("%s\r\n", FirmwareVersion);
    displayMenu();

    char buffer[255];
    buffer[0] = '\0';

    while (1)
    {
        if (charavail())
        {
            gets(buffer);
            processCommand(buffer);
            buffer[0] = '\0';
        }
    }
}
