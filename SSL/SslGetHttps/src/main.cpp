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
#include <crypto/ssl.h>
#include <ctype.h>
#include <dhcpclient.h>
#include <dns.h>
#include <init.h>
#include <iointernal.h>
#include <startnet.h>
#include <stdio.h>
#include <string.h>
#include <tcp.h>

#ifdef _DEBUG
#include <NetworkDebug.h>
#endif

const char *AppName = "SSL HTTPS GET Example";

#warning You must either include a CA certificate list in the project,
#warning or you must recompile the cryptolib with the NB_SSL_CLIENT_CERTIFICATE_CHECKING_ENABLED uncommented
#warning defined in sslclient.cpp

char rx_buffer[20000];
char name[80];

/**
 *  UserMain
 *
 *  Main entry point of example
 */
void UserMain(void *pd)
{
    init();                                       // Initialize network stack
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

    while (1)
    {
        iprintf("Enter the destination HTTPS server name: ");
        gets(name);
        iprintf("Getting [%s]\r\n", name);
        IPADDR ipa = IPADDR::NullIP();
        int result = GetHostByName(name, &ipa, IPADDR::NullIP(), 40);

        if ((result == DNS_OK) && (!ipa.IsNull()))
        {
            iprintf("\r\n%s = ", name);
            ShowIP(ipa);
            iprintf("\r\n");

            int fds = SSL_connect(ipa, 0, 443, 400, name);
            if (fds <= 0)
            {
                iprintf("Retry\r\n");
                fds = SSL_connect(ipa, 0, 443, 400, name);
            }

            int n;
            int total;
            if (fds > 0)
            {
                iprintf("We connected \n");
                writestring(fds, "GET  /\r\n\r\n");
                total = 0;
                do
                {
                    n = ReadWithTimeout(fds, rx_buffer + total, 20000 - total, 100);
                    if (n > 0) { total += n; }
                    else
                    {
                        iprintf("N response = %d\r\n", n);
                    }
                } while (n > 0);

                iprintf("Read %d bytes\r\n", total);
                rx_buffer[total] = 0;
                rx_buffer[512] = 0;
                iprintf("[%s]\r\n", rx_buffer);
                OSTimeDly(TICKS_PER_SECOND);
                close(fds);
            }
            else
            {
                iprintf("Connect failed with %d :", fds);
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

                iprintf("Hit any key to continue\r\n");
                getchar();
            }
        }
        else
        {
            iprintf("Failed to get DNS entry for %s, verify the DNS server IP address is correct\r\n", name);
        }
    }
}
