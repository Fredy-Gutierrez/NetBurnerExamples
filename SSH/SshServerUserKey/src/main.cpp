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
 * Before you can compile and run this example, you need to modify the linker
 * to replace FatFile.a and use the StdFFile.a library for on-chip flash.  The
 * compcode flags will also have to be modified so that the application will
 * only occupy a specific space in flash, leaving room for the on-chip flash
 * file system.
 *
 * 1. Changes to compcode flags:  In NBEclipse or your command line makefile,
 *    change the following line so the application will only occupy the
 *    specified application space.  The first parameter is the start of the
 *    application space, and the second is the address just below the flash file
 *    system space.  What range you use depends on the flash chip your module is
 *    using.
 *
 *    If you are using the Spansion AM29LV160B, S29AL016D, or Atmel AT49BV163D
 *    flash chip, use the following memory address range:
 *
 *       COMPCODEFLAGS = 0xFFC08000 0xFFD80000
 *
 *    If you are using the SST39VF040 flash chip, use the following memory
 *    address range:
 *
 *       COMPCODEFLAGS = 0xFFC08000 0xFFC70000
 *
 *    If you are using NBEclipse, do the following to make the COMPCODEFLAGS
 *    modification (assuming you have imported this example into an existing
 *    NBEclipse project and already open):
 *
 *    1) Right-click on your opened project in the Project Explorer and select
 *       "Properties" from the pop-up list box.
 *    2) Expand the "C/C++ Build" tree on the left-hand side of the "Properties"
 *       dialog box and select "Settings".
 *    3) In the "Settings" section on the right-hand side, select the "Tool
 *       Settings" tab and scroll down to "NetBurner Compcode".  Select its
 *       "General" sub-section.
 *    4) On the right-hand side that appears for the "General" sub-section,
 *       modify the "Memory range" text box with the appropriate values for your
 *       flash chip as indicated above.
 *    5) Press the "OK" button when done.
 *
 * 2. If you are using NBEclipse, then you will also need to tell the linker to
 *    include the C:\Nburn\lib\StdFFile.a library.  To do this:
 *
 *    1) Right-click on your opened project in the Project Explorer and select
 *       "Properties" from the pop-up list box.
 *    2) Expand the "C/C++ Build" tree on the left-hand side of the "Properties"
 *       dialog box and select "Settings".
 *    3) In the "Settings" section on the right-hand side, select the "Tool
 *       Settings" tab and scroll down to "GNU C/C++ Linker".  Select its
 *       "Libraries" sub-section.
 *    4) On the right-hand side that appears for the "Libraries" sub-section,
 *       select "C:\Nburn\lib\FatFile.a" and remove it by clicking on the small
 *       icon of a white paper with a red 'X' on it.
 *    5) Click on the adjacent icon of a white paper with a green '+' on it to
 *       add the "C:\Nburn\lib\StdFFile.a" library.  When the pop-up box appears
 *       to search for a file to add, use the "File System..." button.
 *    5) Press the "OK" button when done.
 *----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 * This example program sets up HTTP access to the EFFS-STD on-chip flash file
 * system.  To run this example:
 *
 * 1. When the program runs, status messages will be displayed on the module
 *    debug serial port.
 *
 * 2. You can view the files on the flash card with any web browser by typing in
 *    the IP address of the module, followed by "/DIR" (not case-sensitive).
 *
 *    Example:  http://10.1.1.57/DIR
 *
 *----------------------------------------------------------------------------*/
#include <predef.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <taskmon.h>
#include <smarttrap.h>
#include <nbtime.h>
#include <time.h>
#include <init.h>
#include <nbrtos.h>
#include <netinterface.h>
#include <tcp.h>
#include "http_f.h"
#include "effs_time.h"
#include "FileSystemUtils.h"
#include "nvsettings.h"
#include "sshuser.h"

#include <nbssh/nbssh.h>

#define MULTIPART_FORM_BUF_SIZE (50000)
#define SSH_LISTEN_PORT 22       // server listen port number
#define CONNECTION_TIMEOUT 120   // connection timeout in seconds

#define BUFSIZE (4096)
char gRxBuffer[BUFSIZE];
char gTxBuffer[BUFSIZE];
int fdnet = 0;

extern void fs_main();
extern void OnChipFlash_GetFlashName( char *pName );

BOOL bShowDebug = TRUE;

const char *AppName = "SSH User Key Example";

/*-------------------------------------------------------------------
 * Display TCP Err state
 *-----------------------------------------------------------------*/
void DisplayTcpErrState(int code)
{
    switch (code)
    {
        case TCP_ERR_NORMAL: iprintf("TCP Closed\r\n"); break;
        case TCP_ERR_TIMEOUT: iprintf("TCP Timeout\r\n"); break;
        case TCP_ERR_NOCON: iprintf("TCP No Connection\r\n"); break;
        case TCP_ERR_CLOSING: iprintf("TCP Closing\r\n"); break;
        case TCP_ERR_NOSUCH_SOCKET: iprintf("TCP No such socket\r\n"); break;
        case TCP_ERR_NONE_AVAIL: iprintf("TCP No aviable free scoketd\r\n"); break;
        case TCP_ERR_CON_RESET: iprintf("TCP Reset\r\n"); break;
        case TCP_ERR_CON_ABORT: iprintf("TCP Abort\r\n"); break;
        case SSH_ERROR_FAILED_NEGOTIATION: iprintf("SSH FAILED NEGOTIATION\r\n"); break;
        case SSH_ERROR_FAILED_SESSION_FAILED: iprintf("SSH SESSION FAILURE\r\n"); break;
        default: iprintf("Unknown code: %d\r\n", code);
    }
}


/*-------------------------------------------------------------------
 Convert IP address to a string
 -------------------------------------------------------------------*/
void IPtoString(IPADDR ia, char *s)
{
    puint8_t ipb = (puint8_t)&ia;
    siprintf(s, "%d.%d.%d.%d", (int)ipb[0], (int)ipb[1], (int)ipb[2], (int)ipb[3]);
}


/*-------------------------------------------------------------------
 * SSH Server Task
 * Listen for incoming SSH Client connections.
 * ------------------------------------------------------------------*/
// Allocate task stack for UDP listen task
#define SSHSERVER_TASK_STACK_SIZE (USER_TASK_STK_SIZE * 4)
uint32_t SshServerTaskStack[SSHSERVER_TASK_STACK_SIZE];

void SshServerTask(void *pd)
{
    // The listen port is passed to the task when it is created.
    int ListenPort = (int)pd;

    // Set up the listening TCP socket
    int fdListen = listen(INADDR_ANY, ListenPort, 5);

    if (fdListen > 0)
    {
        IPADDR client_addr;
        uint16_t port;

        while (1)
        {
            // The SSL_accept() function will block until a TCP client requests
            // a connection. Once a client connection is accepted, the
            // file descriptor fdnet is used to read/write to it.
            iprintf("\r\nWaiting for connection on port %d...\n", ListenPort);
            fdnet = SshAccept(fdListen, &client_addr, &port, TICKS_PER_SECOND * 300);

            if (fdnet < 0)
            {
                iprintf("SshAccept() timeout: ");
                DisplayTcpErrState(fdnet);
                iprintf("\r\n");
            }
            else
            {
                iprintf("Connected to: ");
                ShowIP(client_addr);
                iprintf(":%d\n", port);

                writestring(fdnet, "Welcome to the NetBurner SSH Server\r\n");

                char s[20];
                IPtoString(InterfaceIP(GetFirstInterface()), s);
                sniprintf(gTxBuffer, BUFSIZE, "You are connected to IP Address %s, port %d\r\n", s, SSH_LISTEN_PORT);
                writestring(fdnet, gTxBuffer);

                while (fdnet > 0)
                {
                    /* Loop while connection is valid. The read() function will return
                     0 or a negative number if the client closes the connection, so we
                     test the return value in the loop. Note: you can also use
                     ReadWithTimout() in place of read to enable the connection to
                     terminate after a period of inactivity.
                     */
                    int n = 0;
                    do
                    {
                        n = ReadWithTimeout(fdnet, gRxBuffer, BUFSIZE - 1, TICKS_PER_SECOND * CONNECTION_TIMEOUT);
                        if (n > 0)
                        {
                            gRxBuffer[n] = '\0';
                            iprintf("%s", gRxBuffer);
                        }
                        else if (n == TCP_ERR_TIMEOUT)   // If the client closes the connection, ReadWithTimeout() will time out
                        {
                            iprintf("TCP ReadWithTimout() Error %d: ", n);
                            DisplayTcpErrState(n);
                        }
                    } while (n > 0);

                    iprintf("Closing client connection: ");
                    ShowIP(client_addr);
                    iprintf(":%d\n", port);
                    close(fdnet);
                    fdnet = 0;
                }
            }
            OSTimeDly(2);
        }   // while(1)
    }       // if fdListen > 0
    else
    {
        iprintf("Error: could not open listen port in SslSocketServerTask\r\n");
    }
}


/*-------------------------------------------------------------------
 * Display command menu.
 *------------------------------------------------------------------*/
void DisplayMenu( void )
{
   iprintf( "\r\n\n--- Main Menu ---\r\n" );
   iprintf( "[1] - EFFS Read/Write Test\r\n" );
   iprintf( "[2] - Display Directory\r\n" );
   iprintf( "[3] - Display TestFile.txt\r\n" );
   iprintf( "[4] - Format SD Card (Warning: Data Will Be Lost)\r\n" );
   iprintf( "[5] - Display EFFS Space Usage\r\n" );
   iprintf( "[6] - Display System Time\r\n" );
   iprintf( "[7] - Reset to Factory Defaults\r\n" );
   iprintf( "[?] - Display Menu\r\n\n" );
}


/*-------------------------------------------------------------------
 * Handle commands with a trivially simple command dispatcher.
 *------------------------------------------------------------------*/
void ProcessCommand( char *buffer )
{
   switch ( buffer[0] )
   {
      case '1':
         ReadWriteTest();
         break;
      case '2':
         iprintf( "Directory Contents:\r\n" );
         DumpDir();
         iprintf( "End of listing\r\n\n" );
         break;
      case '3':
         iprintf( "Displaying TestFile.txt:\r\n" );
         DisplayTextFile( "TestFile.txt" );
         break;
      case '4':
         {
            iprintf( "Proceed with format? (Y/N) ");
            char c = getchar();
            if ( ( c == 'Y' ) || ( c == 'y' ) )
            {
               iprintf( "\r\n" );
               FormatEffsStdFlash();
               iprintf( "Format complete\r\n" );
            }
            else
            {
               iprintf( "\r\nFormat command aborted\r\n" );
            }
            iprintf( "\r\n" );
         }
         break;
      case '5':
         DisplayEffsSpaceStats();   // Display file space usage
         iprintf( "\r\n" );
         break;
      case '6':
         DisplaySystemTime();   // Display current system date and time
         iprintf( "\r\n" );
         break;
      case '7':
         CheckNVSettings( TRUE );
         iprintf( "\r\n" );
         break;
      default: // ?
         DisplayMenu();
   }
}


/*-------------------------------------------------------------------
 * The main task.
 *------------------------------------------------------------------*/
void UserMain( void *pd )
{
   init();
   StartHttp();
   WaitForActiveNetwork(5);

   // Enable multi part forms posting with a 50K max data block size
   //iprintf("Enabling MultiPart forms with a %d KB buffer space\r\n", MULTIPART_FORM_BUF_SIZE / 1000 );
   //if ( !EnableMultiPartForms( MULTIPART_FORM_BUF_SIZE ) )
   //   iprintf( "EnableMultiPartForms() initialization failed\r\n" );
  


   char FlashChipName[80];
   OnChipFlash_GetFlashName( FlashChipName );

   iprintf( "Application started\r\n" );
   iprintf( "\r\nWARNING:\r\n" );
   iprintf( "1. This application requires the compcode memory address\r\n" );
   iprintf( "   range parameters in your project be set to match the\r\n" );
   iprintf( "   file system size.\r\n" );
   iprintf( "2. Verify the flash memory chip you are using is a:  %s\r\n", FlashChipName );
   iprintf( "\r\nIncorrect setting could completely erase the flash!\r\n" );
   iprintf( "Press <Enter> to continue" );
   getchar();

   fs_main();   // Initialize the flash file system

   iprintf("Checking non-volatile settings...\r\n");
   CheckNVSettings( FALSE );  // Note: needs access to EFFS-STD file system

   if ( SetTimeNTP() )
   {
          //tzsetchar((char*)"EST5EDT4,M3.2.0/01:00:00,M11.1.0/02:00:00");
       //tzsetchar((char*)"CST6CDT5,M3.2.0/01:00:00,M11.1.0/02:00:00");
       //tzsetchar((char*)"MST7MDT6,M3.2.0/01:00:00,M11.1.0/02:00:00");
       tzsetchar((char*)"PST8PDT7,M3.2.0/01:00:00,M11.1.0/02:00:00");
 
   }
   else
   {
      iprintf( "NTP failed, setting time manually\r\n" );
      SetTimeManual( 5, 14, 3, 2010, 11, 35, 0 );
   }
   DisplaySystemTime();
   iprintf( "\r\n" );


   // Set SSH username/passuint16_t authentication callback function
   SshSetUserAuthenticate( SshUserAuthenticate );

   // Set SSH callback function to return active SSH key
   SshSetUserGetKey( SshUserGetKey );

   /*
    *   SSH task priority
    *
    *      The SSH server will create by default a task at priority SSH_TASK_PRIORITY
    *      This can be changed by calling SshSetTaskPriority with another priority
    *      greater than MAIN_PRIO and less than 63.
    *
    *      Priorities used the library are defined in constants.h
    *
    */
   if (SshSetTaskPriority(SSH_TASK_PRIORITY) == FALSE) { iprintf("SSH task priority not changed\r\n"); }
   // Create TCP Server task
   iprintf("Starting SSH Server Task, Listening for SSH connection on port %d\r\n", SSH_LISTEN_PORT);
   int status = OSTaskCreatewName(SshServerTask, (void *)SSH_LISTEN_PORT, &SshServerTaskStack[SSHSERVER_TASK_STACK_SIZE],
                                 SshServerTaskStack, MAIN_PRIO - 1, "SshServer");   // higher priority than UserMain

   if (status != OS_NO_ERR) iprintf("*** Error creating SSH Server Task: %d\r\n", status);

   DisplayMenu(); // Display serial debug menu
   while ( 1 )
   {
      char buffer[255];
      buffer[0] = '\0';

      if ( charavail() )
      {
         gets( buffer );
         ProcessCommand( buffer );
         buffer[0] = '\0';
      }

      OSTimeDly(2);
   }
}
