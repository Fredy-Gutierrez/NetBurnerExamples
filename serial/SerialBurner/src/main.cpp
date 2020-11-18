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
#include <http.h>
#include <init.h>
#include <iosys.h>
#include <serial.h>
#include <stdlib.h>
#include <system.h>
#include <tcp.h>
#include <utils.h>

#include "datapump.h"
#include "nvsettings.h"

const char *AppName = "SerialBurner Example";

#ifdef MCF5441X
void fShowSerialData(FILE *fp);
#endif

#define DATA_SERIAL_PORT (1)

/* Debug Notes:
   Debug/informational messages can be sent to the DATA_SERIAL_PORT or to
   the DEBUG_SERIAL_PORT.
   - To send debug messages out the debug serial port, set DEBUG_SERIAL_PORT
     to 0 or 1; whatever port is not the data port.
   - To send debug messages out the same port as the data port, set
     DEBUG_SERIAL_PORT to -1.
   - To send debug messages in either of the above cases, you must also set DEBUG to 1.
*/
#define DEBUG_SERIAL_PORT (0)
#define DEBUG (1)   // 1 = allocate a debug port, 0 = no debug port at all

#define DEF_DATA_BAUDRATE (115200)
#define STOP_BITS (1)
#define DATA_BITS (8)

#define SERIAL_FLOWCONTROL_NONE (0)
#define SERIAL_FLOWCONTROL_SOFTWARE (1)
#define SERIAL_FLOWCONTROL_HARDWARE (2)

#define TELNET_PORT (23)
#define DEF_SERVER_LISTEN_PORT (9221)

//----- Global Vars -----
int fddebug = 0;    // file descriptor for serial debug output
int fdserial = 0;   // file descriptor for serial data I/O

extern bool bChangeServerListenPort;

char wbuffer[CLIENT_WRITE_BUF_SIZE];

int clientfd = 0;

/*------------------------------------------------------------------------------
 *   Configure serial port flow control
 *----------------------------------------------------------------------------*/
void ConfigureSerialFlowControl()
{
#ifdef MOD5441X
    switch (NV_Settings.SerialDataFlowControl)
    {
        case SERIAL_FLOWCONTROL_NONE:
            // port = 0 or 1, enab = 0 or 1
            SerialEnableTxFlow(DATA_SERIAL_PORT, 0);
            SerialEnableRxFlow(DATA_SERIAL_PORT, 0);
            iprintf("Serial data flow control disabled\r\n");
            break;

        case SERIAL_FLOWCONTROL_SOFTWARE:
            SerialEnableTxFlow(DATA_SERIAL_PORT, 1);
            SerialEnableRxFlow(DATA_SERIAL_PORT, 1);
            iprintf("Serial data flow control enabled\r\n");
            break;

        case SERIAL_FLOWCONTROL_HARDWARE: break;
    }
#endif
}

/*-------------------------------------------------------------------------------------
 *   Initialize serial ports. Executed on boot and upon web page configuration changes
 *------------------------------------------------------------------------------------*/
void InitializeSerialPorts()
{
    // Open the data serial port
    SerialClose(DATA_SERIAL_PORT);
    fdserial = OpenSerial(DATA_SERIAL_PORT, NV_Settings.DataBaudRate, STOP_BITS, DATA_BITS, eParityNone);

    // Open the debug serial port
    if (DEBUG_SERIAL_PORT != DATA_SERIAL_PORT)
    {
        SerialClose(DEBUG_SERIAL_PORT);
        fddebug = OpenSerial(DEBUG_SERIAL_PORT, DEF_DATA_BAUDRATE, STOP_BITS, DATA_BITS, eParityNone);
    }
    else
    {
        fddebug = fdserial;
    }

    ReplaceStdio(0, fddebug);
    ReplaceStdio(1, fddebug);
    ReplaceStdio(2, fddebug);

    ConfigureSerialFlowControl();

    iprintf("Serial Data Port:  %d, fd = %d\r\n", DATA_SERIAL_PORT, fdserial);
    iprintf("Serial Debug Port: %d, fd = %d\r\n", DEBUG_SERIAL_PORT, fddebug);
}

/*-----------------------------------------------------------------------
 *  Check NV Settings.  Assign default values if VerifyKey is not valid
 *-----------------------------------------------------------------------*/
void CheckNVSettings()
{
    NV_SettingsStruct *pData = (NV_SettingsStruct *)GetUserParameters();
    if (pData != NULL)
    {
        NV_Settings.VerifyKey = pData->VerifyKey;
        NV_Settings.ServerListenPort = pData->ServerListenPort;
        NV_Settings.ClientTimeout = pData->ClientTimeout;
        NV_Settings.ClientOverrideTimeout = pData->ClientOverrideTimeout;
        NV_Settings.DataBaudRate = pData->DataBaudRate;
        NV_Settings.SerialDataFlowControl = pData->SerialDataFlowControl;
    }
    else
    {
        iprintf("*** ERROR *** Could not retrieve User Parameters\r\n");
        return;
    }

    if (NV_Settings.VerifyKey != VERIFY_KEY)
    {
        NV_Settings.VerifyKey = VERIFY_KEY;
        NV_Settings.ServerListenPort = DEF_SERVER_LISTEN_PORT;
        NV_Settings.ClientTimeout = DEF_INACTIVITY_TIMEOUT;
        NV_Settings.ClientOverrideTimeout = DEF_OVERRIDE_TIMEOUT;
        NV_Settings.DataBaudRate = DEF_DATA_BAUDRATE;
        NV_Settings.SerialDataFlowControl = SERIAL_FLOWCONTROL_NONE;

        SaveUserParameters(&NV_Settings, sizeof(NV_Settings));
    }
}

/*-----------------------------------------------------------------------------
 *  ProcessDebug
 *----------------------------------------------------------------------------*/
void ProcessDebug()
{
    char c = getchar();

    switch (toupper(c))
    {
        case 'C':
        {
            ShowCounters();
            break;
        }

        case 'S': {
#ifdef MCF5441X
            fShowSerialData(stdout);
#endif
            break;
        }

        default:
        {
            iprintf("Invalid Command\r\n");
            break;
        }
    }
}

/*-----------------------------------------------------------------------------
 *  UserMain
 *----------------------------------------------------------------------------*/
void UserMain(void *pd)
{
    init();
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);
    StartHttp();

    CheckNVSettings();   // will populate the NV_Settings structure
    bChangeServerListenPort = false;

    InitializeSerialPorts();

    iprintf("Application: %s\r\nNNDK Revision: %s\r\n", AppName, GetReleaseTag());

    // Listen on specified port number and connect from any address.
    iprintf("Listening on port %d\r\n", NV_Settings.ServerListenPort);
    int serverfd = listen(INADDR_ANY, NV_Settings.ServerListenPort, 5);

    if (serverfd > 0)
    {
        IPADDR client_addr;
        uint16_t port;

        iprintf(MTS_WHEN_NOTCONNECTED);

        while (1)
        {
            if (bChangeServerListenPort)
            {
                close(serverfd);
                writestring(fdserial, MTS_WHEN_CONNECTION_CLOSED);
                serverfd = listen(INADDR_ANY, NV_Settings.ServerListenPort, 5);
                if (serverfd < 0) { iprintf("Error changing Server Listen Port: %d\r\n", serverfd); }
                else
                {
                    iprintf("Changed to Server Listen Port: %d, fd = %d\r\n", NV_Settings.ServerListenPort, serverfd);
                }
                bChangeServerListenPort = FALSE;
            }

            clientfd = accept(serverfd, &client_addr, &port, TICKS_PER_SECOND);

            if (clientfd > 0)
            {
                writestring(fdserial, MTS_WHEN_CONNECTION_OPENED);
                int pump_rtn = 0;
                pump_rtn = DataPump(clientfd, fdserial, serverfd);
                if (pump_rtn == clientfd)
                {
                    close(clientfd);
                    writestring(fdserial, MTS_WHEN_CONNECTION_CLOSED);
                    writestring(fdserial, MTS_WHEN_NOTCONNECTED);
                    clientfd = 0;
                }
            }

            // if (dataavail(fddebug)) { ProcessDebug(); }
            if (charavail()) ProcessDebug();
        }
    }
    else
    {
        iprintf("System Error - could not open listening port. Program halted\r\n");
        while (1)
            OSTimeDly(TICKS_PER_SECOND);
    }
}
