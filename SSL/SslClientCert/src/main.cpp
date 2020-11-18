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
#include <base64.h>
#include <crypto/ssl.h>
#include <ctype.h>
#include <dhcpclient.h>
#include <init.h>
#include <startnet.h>
#include <stdio.h>
#include <string.h>
#include <tcp.h>

#ifdef _DEBUG
#include <NetworkDebug.h>
#endif

/* Modify this definition to specify your target SSL server. */
#define SSL_SERVER_IP "10.1.1.193"

const char *AppName = "SSL Client Cert Example";

extern "C"
{
    void UserMain(void *pd);
}

/**************** Programmer must modify the following code **********************/

/* These point to the linked in files that contain the private key and certificate
 * to use as a client
 */
extern const unsigned char comp_key[];
extern const unsigned char comp_cert[];

void SSL_ClientReadyCert(char *certPEM, char *keyPEM);

unsigned int DB_FLAGS = DB_SSL;

/**
 *  UserMain
 *
 *  Main entry point for the example
 */
void UserMain(void *pd)
{
    init();                                       // Initialize network stack
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

    /****************Tell the system where to get client keys ***********/
    // Assigns your local function as the one to use when getting a key
    SSL_ClientReadyCert((char *)comp_cert, (char *)comp_key);

    iprintf("Hit a key to attempt a connection\r\n");
    getchar();

    int nTimes = 1;
    int nFailed = 0;
    while (1)
    {
        iprintf("Attempting to connect to: %s:4433 ...\r\n", SSL_SERVER_IP);
        iprintf("Connect start timer = %ld\r\n", TimeTick);

        // SSL Server IP, Local Port, Dest Port, Timeout (in timeticks), command name
        int fds = SSL_connect(AsciiToIp(SSL_SERVER_IP), 0, 4433, 200, SSL_SERVER_IP);

        if (fds > 0)
        {
            char buf[80];

            // send message to network connection
            sniprintf(buf, 80, "We connected %d times, Failed %d times \r\n", nTimes++, nFailed);
            writestring(fds, buf);
            close(fds);

            // send message to serial debug port
            iprintf("We have connected %d times, Failed: %d times\r\n", nTimes - 1, nFailed);
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

            nFailed++;
            char c;
            do
            {
                iprintf("Hit 'C' to attempt another connection\r\n");
                c = getchar();
            } while (c != 'C');
        }
    }
}
