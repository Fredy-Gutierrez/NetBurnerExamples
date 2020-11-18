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

/*-------------------------------------------------------------------
  This program demonstrates the EFFS FAT file system and FTP operations
  for SD/MMC/SDHC and Compact Flash cards on 5441x platforms:
   - Mounting a flash drive
   - Determining amount of used and free file space
   - Using FTP to upload and download files

   NetBurner 5441x based platforms have two types of flash card interfaces:
   1. Native SPI
   2. SDHC

   These types are mutually exclusive and the flash card connector on the device
   must be wired one way or the other. As of the writing of this example:
   MOD54415, MOD54417:
      Onboard microSD connector is SPI.
      Development board is wired for SPI.
      Signals are available on the module headers to wire for SDHC or SPI

   NANO54415:
      Onboard microSD wired for SDHC
      Development board wired for SPI

   SB800EX
      Onboard microSD wired for SDHC

  Any SPI connected card should also be able to run any of the EFFS FAT
  examples located in the EFFS directory of \nburn\examples.

  To run the speed test you will need to place a file named TEST.BIN on the
  flash card. You can do this with FTP to the device directly, or copy it
  onto the flash drive from another device.

  When the program executes it will display program status information
  through the debug serial port.

  Modules with an onboard microSD flash socket should use the multi mmc header
  files and functions because the modules are capable of supporting both onboard
  and external flash cards (even if you application only uses one).
 ------------------------------------------------------------------------------*/
#include <predef.h>
#include <stdio.h>
#include <init.h>
#include <sim.h>           /*on-chip register definitions*/
#include <pins.h>
#include <nbrtos.h>
#include <math.h>
#include <serial.h>
#include <iosys.h>

#include <taskmon.h>
#include <smarttrap.h>
#include <serial.h>
#include "cardtype.h"
#include <effs_fat/fat.h>
#include <ftpd.h>
#include <networkdebug.h>
#include "ftp_f.h"
#include "effs_time.h"
#include "FileSystemUtils.h"
#include <nbtime.h>
#include "dev_test.h"

#if ( defined(MOD5441X) )
#define MULTI_MMC TRUE  // For modules with onboard flash sockets, even if you are using external flash cards
#include <effs_fat/multi_drive_mmc_mcf.h>
const char* AppName = "EFFS FTP MultiDrive";

#elif ( defined(USE_MMC) )
#include <effs_fat/mmc_mcf.h>
const char* AppName = "EFFS FTP MMC/SD";

#elif ( defined(USE_CFC) )
#include <effs_fat/cfc_mcf.h>
const char* AppName = "EFFS FTP CFC";

#elif (defined(USE_SDHC) && ( defined(NANO54415) || defined(SB800EX) ))
#include <effs_fat/sdhc_mcf.h>
const char* AppName = "EFFS FTP SD/SDHC";

#else
#error Undefined Platform
#endif

#if (defined SB800EX)
#include <serial_config.h>
#include <cpu_pins.h>
#else
#include <pins.h>
#endif

#define FTP_PRIO ( MAIN_PRIO - 2 )

extern unsigned long CPU_CLOCK;
extern unsigned long XTAL_CLOCK;


/*-------------------------------------------------------------------
 Display command menu
 ------------------------------------------------------------------*/
void DisplayMenu()
{
   iprintf("\r\n\n--- Main Menu ---\r\n" );
   iprintf("D - Display Directory\r\n");
   iprintf("E - Display TestFile.txt\r\n");
   iprintf("F - Format SD Flash card (warning: data will be lost!)\r\n" );
   iprintf("S - Display Space usage\r\n");
   iprintf("T - Display system Time\r\n");
   iprintf("W - Run File Read/Write Speed Test\r\n");
   iprintf( "? - Display Menu\r\n\n\n" );
}


/*-------------------------------------------------------------------
 Handle commands with  a trivially simple command dispatcher
 ------------------------------------------------------------------*/
void ProcessCommand( char *buffer )
{
   switch ( toupper( buffer[0] ) )
   {
      case 'D':
         iprintf("Directory Contents:\r\n");
         DumpDir();       // Display flash card files and directories
         iprintf("End of listing\r\n\n");
         break;

      case 'E':
         iprintf("Displaying TestFile.txt:\r\n" );
         DisplayTextFile( "TestFile.txt" );
         break;

      case 'F':
      {
           iprintf("Proceed with format? ('Y' to execute)");
           char c = getchar();
           if ( (c == 'y') || (c == 'Y') )
           {
               iprintf("\r\n");
               FormatExtFlash(F_FAT32_MEDIA);
               iprintf("Format complete\r\n");
           }
           else
           {
               iprintf("\r\nFormat command aborted\r\n");
           }
           iprintf("\r\n");
       }
       break;

      case 'S':
         DisplayEffsSpaceStats();  // Display file space usage
         iprintf("\r\n");
         break;

      case 'T':
         DisplaySystemTime();  // Display file space usage
         iprintf("\r\n");
         break;
      case 'W':
    	  {
    		  int res = SpeedTest("TEST.BIN", 10*1024*1024);
    		  if( res == 0)
    			  iprintf("Test successfully completed\n\r");
			  else
				  iprintf("Test execution failed! Error %d\n\r", res);
    	  }
    	  break;

      default:
         DisplayMenu();
   }
}


//#if (defined SB800EX)
///*-----------------------------------------------------------------------------
// * Just an easy way to configure the boot port from within an application
// *----------------------------------------------------------------------------*/
//void SB800EXSetSerialBootPort( uint32_t uartNumber )
//{
//    ConfigRecord newConfigRec;
//    memcpy(&newConfigRec, &gConfigRec, sizeof(newConfigRec));
//    newConfigRec.ser_boot = uartNumber;
//    UpdateConfigRecord(&newConfigRec);
//}
//
///*-----------------------------------------------------------------------------
// * Configure RS-232 debug port for status messages.
// * This function will also change the boot port in the global configuration
// * record. Valid uart numbers are 1 or 2.
// *----------------------------------------------------------------------------*/
//void SB800EXConfigureSerialPort(uint32_t rs232BootPort)
//{
//    uint32_t fdDataUart = 0;
//
//    SetSerialMode(rs232BootPort, SERIAL_XCVR_RS232);
//    SerialClose(rs232BootPort);
//    fdDataUart = SimpleOpenSerial(rs232BootPort, 115200);
//
//    ReplaceStdio(0, fdDataUart);
//    ReplaceStdio(1, fdDataUart);
//    ReplaceStdio(2, fdDataUart);
//
//    if (gConfigRec.ser_boot != rs232BootPort)   // change config rec for boot port
//        SB800EXSetSerialBootPort(rs232BootPort);
//}
//#endif


/*-------------------------------------------------------------------
  UserMain()
 -------------------------------------------------------------------*/
void UserMain( void* pd )
{
    init();

//#if (defined SB800EX)
//    SB800EXConfigureSerialPort(2);
//#endif

   EnableTaskMonitor();
   EnableSmartTraps();
//   iprintf( "\r\n===== Starting %s Program =====\r\n", AppName );
   iprintf( "\r\n===== Starting Program =====\r\n" );

   /* The following call to f_enterFS() must be called in every task that accesses
      the file system.  This must only be called once in each task and must be done before
      any other file system functions are used.  Up to 10 tasks can be assigned to use
      the file system. Any task may also be removed from accessing the file system with a
      call to the function f_releaseFS(). */
   f_enterFS();

   // We now must also enter the file system for the FTP task
   OSChangePrio( FTP_PRIO );
   f_enterFS();
   OSChangePrio( MAIN_PRIO );


#ifdef _DEBUG
   InitializeNetworkGDB_and_Wait();
   OSTimeDly(1*TICKS_PER_SECOND);
#endif

   InitExtFlash();  // Initialize the CFC or SD/MMC external flash drive

   // Try setting the time via a NTP Server, if that fails, set it manually.
   // There are many more time zone definitaions in timezone.h
   if ( SetTimeNTP() )
   {
       //tzsetchar((char*)"EST5EDT4,M3.2.0/01:00:00,M11.1.0/02:00:00");
       //tzsetchar((char*)"CST6CDT5,M3.2.0/01:00:00,M11.1.0/02:00:00");
       //tzsetchar((char*)"MST7MDT6,M3.2.0/01:00:00,M11.1.0/02:00:00");
       tzsetchar((char*)"PST8PDT7,M3.2.0/01:00:00,M11.1.0/02:00:00");
   }
   else
   {
      iprintf("NTP failed, setting time manually\r\n");
      // month, day, dow, year, hour, min, sec
      SetTimeManual(5, 14, 3, 2006, 11, 35, 0 );
   }
   DisplaySystemTime();
   iprintf( "\r\n" );

   DisplayEffsSpaceStats();  // Display file space usage

   // Start FTP server with task priority higher than UserMain()
   int status = FTPDStart( 21, FTP_PRIO );
   if ( status == FTPD_OK )
   {
      iprintf("Started FTP Server\r\n");
      if( F_LONGFILENAME == 1 )
        iprintf("Long file names are supported\r\n");
      else
        iprintf("Long file names are not supported- only 8.3 format\r\n");
   }
   else
   {
      iprintf( "** Error: %d. Could not start FTP Server\r\n", status);
   }

   ReadWriteTest("TestFile.txt");

   DisplayMenu();
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
   }
}


