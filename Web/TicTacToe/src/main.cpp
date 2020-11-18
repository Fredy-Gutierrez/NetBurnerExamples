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
#include <netinterface.h>
#include <serial.h>
#include <startnet.h>

extern const char PlatformName[];   // Name of NetBurner platform

const char *AppName = "TicTacToe Example";

/* Diagnostic Functions used in the command dispatcher */
void dEnet();
void ShowArp();
void DumpTcpDebug();
void EnableTcpDebug(uint16_t);

extern "C"
{
    void OSDumpTCBs();
}

/**
 *  ProcessPing
 *
 *  This function pings the address given in buffer
 */
void ProcessPing(char *buffer)
{
    IPADDR addr_to_ping;
    char *cp = buffer;

    // Trim leading white space
    while ((*cp) && (isspace(*cp)))
    {
        cp++;
    }

    // Get the address or use the default
    if (cp[0]) { addr_to_ping = AsciiToIp(cp); }
    else
    {
        // Get the first interface block to determine IP address, etc.
        InterfaceBlock *pIFB = GetInterfaceBlock();
        if (pIFB != nullptr) { addr_to_ping = pIFB->ip4.cur_gate; }
    }

    iprintf("\nPinging : %I\r\n", addr_to_ping);

    // Send the ping request and check the response
    int rv = Ping(addr_to_ping.Extract4(), 1 /*Id */, 1 /*Seq */, 100 /*Max Ticks*/);
    if (rv == -1) { iprintf(" Failed! \n"); }
    else
    {
        iprintf(" Response Took %d ticks\n", rv);
    }
}

/**
 *  DoManfTest
 *
 *  This function is used in manufacturing test
 */
void DoManfTest()
{
    int fd1 = OpenSerial(1, 115200, 2, 8, eParityNone);
    char buffer[10];
    buffer[0] = ' ';
    do
    {
        read(fd1, buffer, 1);
        write(fd1, buffer, 1);
    } while (buffer[0] != 'x');
    close(fd1);
}

/**
 *  ProcessCommand
 *
 *  A very simple command parser to handle user input from
 *  the debug serial port.
 */
void ProcessCommand(char *buffer)
{
    switch (toupper(buffer[0]))
    {
        case 'A':
        {
            ShowArp();
            break;
        }
        case 'C':
        {
            ShowCounters();
            break;
        }
        case 'E':
        {
            dEnet();
            break;
        }
#ifdef _DEBUG
        case 'T':
        {
            DumpTcpDebug();
            break;
        }
#endif
        case 'P':
        {
            ProcessPing(buffer + 1);
            break;
        }
        case '~':
        {
            DoManfTest();
            break;
        }
#ifdef _DEBUG
        case 'L':
        {
            SetLogLevel();
            break;
        }
#endif
        case 'W':
        {
            printf("Tick count = %lX = %ld  = %ld secs\n", TimeTick, TimeTick, (TimeTick / TICKS_PER_SECOND));
            break;
        }
        case '?':
        {
            printf(" 'A' ShowArp\n");
            printf(" 'C' ShowCounters\n");
            printf(" 'E' DumpEthernet\n");
#ifdef _DEBUG
            printf(" 'L' Setup debug log enable(s)\n");
#endif
            printf(" 'P' x.x.x.x Do Ping\n");
#ifdef _DEBUG
            printf(" 'T' DumpTcpDebug\n");
#endif
#ifdef NBRTOS_STACKCHECK
            printf(" 'U' Dump UCOS Tasks\n");
            printf(" 'S' Dump UCOS Stacks\n");
#endif
#ifdef NBRTOS_TASKLIST
            printf(" 'H' Show task changes\n");
#endif
            printf(" 'W' When is it (Time)\n");
            break;
        }
#ifdef NBRTOS_STACKCHECK
        case 'U':
        {
            OSDumpTasks();
            break;
        }
        case 'S':
        {
            OSDumpTCBStacks();
            break;
        }
#endif
#ifdef NBRTOS_TASKLIST
        case 'H':
        {
            ShowTaskList();
            break;
        }
#endif
        default: break;
    }   // Switch
}

/**
 *  UserMain
 *
 *  Main entry point for the example
 */
void UserMain(void *pd)
{
    init();                                       // Initialize network stack
    StartHttp();                                  // Start web server, default port 80
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

    iprintf("Application: %s\r\nNNDK Revision: %s\r\n", AppName, GetReleaseTag());

#ifdef _DEBUG
    DB_FLAGS = 0;
#endif

    while (1)
    {
        char buffer[255];
        buffer[0] = '\0';

        // If we have any characters available on the serial channel,
        // let's process them as commands
        if (charavail())
        {
            fgets(buffer,255,stdin);
            iprintf("\r\n");
            ProcessCommand(buffer);
            buffer[0] = '\0';
        }
    }
}
