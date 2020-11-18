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

// NB library definitions
#include <hal.h>
#include <dns.h>
#include <http.h>   // startnet.h
#include <init.h>
#include <nbtime.h>
#include <netinterface.h>

//#include <syslog.h>
#define SysLog(...) (void)0;

// NB SSL library
#include <crypto/ssl.h>

// Product definitions
#include "nbfactory.h"

// NB EFFS-STD library

#include "effs_std.h"
#include "effs_time.h"

// Ethernet to serial application definitions
#include "serialburnerdata.h"

extern MonitorRecord monitor_config;

#define SSL_SERVER_PORT 443

void RescanCACerts();

extern const char *default_page;   // Default HTTP startup page
extern void DisplayNvSettings(void);

const char FirmwareVersion[] = NB_FACTORY_MODULE_BASE_NAME NB_FACTORY_FEATURE_NAME __DATE__ " OS:" NB_VERSION_TEXT " ";
bool bShowDebug = false;   // Debug display flag

extern SharkSsl clientCtx;
extern SharkSslCAList gCAList;

/**
 * @brief Attempts to make an SSL connection.
 */
void TrySslConnection()
{
    char domainBuffer[255];
    iprintf("\r\nEnter the domain you would like to connect to: ");
    gets(domainBuffer);
    iprintf("\r\n");

    // Handle backspace characters
    int j = 0;
    for (int i = 0; i < 255 && domainBuffer[i] != 0; i++)
    {
        if (domainBuffer[i] != 8)
        {
            if (j >= 0) { domainBuffer[j] = domainBuffer[i]; }
            j++;
        }
        else
        {
            j--;
        }
    }
    // Terminate the string
    domainBuffer[j] = 0;
    iprintf("\r\n");
    IPADDR ip_addr;
    GetHostByName(domainBuffer, &ip_addr, IPADDR::NullIP(), TICKS_PER_SECOND * 15);
    iprintf("Attempting to connect to: %s:%d (", domainBuffer, SSL_SERVER_PORT);
    ShowIP(ip_addr);
    iprintf(")\r\n");

    // Scan the CA List
    RescanCACerts();

    // SSL Server IP, Local Port, Dest Port, Timeout (in timeticks), common_name, verifyPeer
    int fds = SSL_connect(ip_addr, 0, SSL_SERVER_PORT, TICKS_PER_SECOND * 60, domainBuffer, true, true, gCAList);

    if (fds > 0)
    {
        iprintf("Good SSL connection\r\n");
        writestring(fds, "GET / HTTP/1.0\r\nUser-Agent: The-worlds-most-basic-HTTP-client\r\n\r\n");
        OSTimeDly(TICKS_PER_SECOND * 2);

        int n;
        int total = 0;
        const int buf_size = 20000;
        static char rx_buffer[buf_size];
        do
        {
            n = SSLReadWithTimeout(fds, rx_buffer + total, buf_size - total, TICKS_PER_SECOND * 5);
            if (n == 0)
            {
                iprintf("Timeout reading from socket\r\n");
                break;
            }
            else if (n > 0)
            {
                iprintf("Read %d bytes\r\n", n);
                total += n;
            }
            else if (n < 0)
            {
                iprintf("Connection closed\r\n");
            }
        } while ((n > 0) && (total < buf_size));

        iprintf("Read %d total bytes\r\n", total);
        close(fds);
    }
    else
    {
        iprintf("Connect failed with error %d, ", fds);
        switch (fds)
        {
            case SSL_ERROR_FAILED_NEGOTIATION: iprintf(" SSL_ERROR_FAILED_NEGOTIATION        \r\n"); break;
            case SSL_ERROR_HASH_FAILED: iprintf(" SSL_ERROR_HASH_FAILED               \r\n"); break;
            case SSL_ERROR_CERTIFICATE_UNKNOWN: iprintf(" SSL_ERROR_CERTIFICATE_UNKNOWN       \r\n"); break;
            case SSL_ERROR_WRITE_FAIL: iprintf(" SSL_ERROR_WRITE_FAIL                \r\n"); break;
            case SSL_ERROR_CERTIFICATE_NAME_FAILED: iprintf(" SSL_ERROR_CERTIFICATE_NAME_FAILED   \r\n"); break;
            case SSL_ERROR_CERTIFICATE_VERIFY_FAILED: iprintf(" SSL_ERROR_CERTIFICATE_VERIFY_FAILED \r\n"); break;
            default: iprintf("Other error\r\n");
        }
    }
}

/**
 * @brief Display debug commands
 */
void DisplayDebugMenu()
{
    iprintf("\r\nDebug commands :\r\n");
    iprintf("0 Debug messages off\r\n");
    iprintf("1 Debug messages on\r\n");
    iprintf("C Attempt SSL Connection\r\n");
    iprintf("D reset to factory default parameters\r\n");
    iprintf("F show file system info\r\n");
    iprintf("G format file system - caution!\r\n");
    iprintf("N display NV Settings\r\n");
    iprintf("T display system time\r\n");
}

extern void SetDefaults();

/**
 * @brief Processes debug commands.
 *
 * @param c The command to process.
 */
void ProcessDebugCommand(char c)
{
    switch (toupper(c))
    {
        case '0':
            bShowDebug = false;
            iprintf("Debug is off\r\n");
            break;
        case '1':
            bShowDebug = true;
            iprintf("Debug is on\r\n");
            break;
        case 'B': ForceReboot(); break;
        case 'C': TrySslConnection(); break;
        case 'D':
            iprintf("Setting default values\r\n");
            SetDefaults();
            break;
        case 'F':
            EffsListCurrentDirectory(NV_Settings.DeviceName);
            EffsDisplayStatistics(NV_Settings.DeviceName);
            break;
        case 'G': EffsFormat(); break;
        case 'N': DisplayNvSettings(); break;
        case 'T':
            DisplaySystemTime();   // Display current system date and time
            iprintf("\r\n");
            break;
        default: DisplayDebugMenu();
    }
}

// Application name
const char *AppName = NB_FACTORY_MODULE_BASE_NAME NB_FACTORY_FEATURE_NAME " " NB_FACTORY_FEATURE_DESC;

unsigned int DB_FLAGS = DB_SSL;

/**
 * @brief Main task
 */
void UserMain(void *notUsedPtr)
{
    init();

    // EFFS-STD
    EffsStart((char *)"EffsStart");

    CheckNVSettings();

    bShowDebug = true;

    if (SetTimeNTP())
    {
        // tzsetchar((char*)"EST5EDT4,M3.2.0/01:00:00,M11.1.0/02:00:00");
        // tzsetchar((char*)"CST6CDT5,M3.2.0/01:00:00,M11.1.0/02:00:00");
        // tzsetchar((char*)"MST7MDT6,M3.2.0/01:00:00,M11.1.0/02:00:00");
        tzsetchar((char *)"PST8PDT7,M3.2.0/01:00:00,M11.1.0/02:00:00");
    }
    else
    {
        iprintf("NTP failed, setting time manually\r\n");
    }
    DisplaySystemTime();
    iprintf("\r\n");

    SysLog("Starting up\r\n");

    StartHttps();

    default_page = "CaCert.html";

    // Publish NetBIOS name
    NetbiosEnableNameService(NV_Settings.NetBIOSName, true);

    // Display product name
    if (!monitor_config.Quiet)
    {
        iprintf("%s : ", NV_Settings.DeviceName);
        iprintf("Product->    %s%s", NB_FACTORY_MODULE_BASE_NAME, NB_FACTORY_FEATURE_NAME);
        iprintf(" %s\r\n", NB_FACTORY_FEATURE_DESC);
    }

    boot_iprintf("Device->     %s", NV_Settings.DeviceName);
    InterfaceBlock *pifb = GetInterfaceBlock();
    if ((pifb != nullptr) && (!monitor_config.Quiet == 0))
    {
        iprintf("%s : ", NV_Settings.DeviceName);
        iprintf("IP Address-> %hI\r\n", pifb->ip4.cur_addr.i4);
    }

    boot_iprintf("Version->    %s", NB_FACTORY_DEFAULTS_VERSION_STRING " Created:" __DATE__);
    debug_iprintf("Settings size->    %ld", sizeof(NV_Settings));
    boot_iprintf("? for debug commands");
    OSTimeDly(TICKS_PER_SECOND / 2);

    // EFFS-STD
    // EffsListCurrentDirectory(NV_Settings.DeviceName);
    // EffsDisplayStatistics(NV_Settings.DeviceName);

    while (1)
    {
        char c = getchar();
        ProcessDebugCommand(c);
    }
}
