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
#include <predef.h>

// NB Libs
#include <dns.h>
#include <init.h>
#include <tcp.h>

// NB SSL Libs
#include <crypto/SharkSsl.h>
#include <crypto/private/ssl_internal.h>
#include <crypto/ssl.h>

// Holds the certificate data
#include "caList.h"

// Application name
const char *AppName = "VerifyPeerBasic";

#define SSL_SERVER_PORT 443

extern SharkSsl clientCtx;
SharkSslCAList gCAList = nullptr;
SharkSslCertStore gCAStore;
bool createdCaList = false;

void CreateCaList()
{
    // Construct the object that will actually store the data
    SharkSslCertStore_constructor(&gCAStore);

    // Add first CA to list
    SharkSslCertStore_add(&gCAStore, (const char *)DuckDuckCert, DuckDuckCertLen);

    // Add second CA to list
    SharkSslCertStore_add(&gCAStore, (const char *)NetBurnerCert, NetBurnerCertLen);

    // Associate the storage object with the list
    if (!SharkSslCertStore_assemble(&gCAStore, &gCAList)) { iprintf("Unable to assemble the CA List.  Verify peer will not work.\r\n"); }

    createdCaList = true;
}

void TrySslConnection()
{
    char domainBuffer[255];
    iprintf("\r\nEnter the domain you would like to connect to: ");
    fgets(domainBuffer, 255, stdin);
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

    // Printout what we're trying to connect to
    iprintf("\r\n");
    IPADDR ip_addr;
    GetHostByName(domainBuffer, &ip_addr, IPADDR::NullIP(), TICKS_PER_SECOND * 15);
    iprintf("Attempting to connect to: %s:%d (", domainBuffer, SSL_SERVER_PORT);
    ShowIP(ip_addr);
    iprintf(")\r\n");

    // We can only build the CA List one time
    if (!createdCaList) { CreateCaList(); }

    // SSL Server IP, Local Port, Dest Port, Timeout (in timeticks), common_name, verifyPeer, wait for negotiation, CA List
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
 *  Display debug commands
 *
 *  Shows the debug menu
 */
void DisplayDebugMenu()
{
    iprintf("\r\nCommands :\r\n");
    iprintf("C: Attempt SSL Connection\r\n");
}

/**
 * Process debug commands
 *
 *  Takes entered commands and then processes them
 */
void ProcessDebugCommand(char c)
{
    switch (toupper(c))
    {
        case 'C': TrySslConnection(); break;
        default: DisplayDebugMenu(); break;
    }
}

/*-----------------------------------------------------------------------------
 * Main task
 *-----------------------------------------------------------------------------*/
void UserMain(void *notUsedPtr)
{
    init();

    iprintf("VerifyPeer Basic Application Started...\r\n");
    DisplayDebugMenu();
    OSTimeDly(TICKS_PER_SECOND / 2);

    while (1)
    {
        char c = getchar();
        ProcessDebugCommand(c);
    }
}
