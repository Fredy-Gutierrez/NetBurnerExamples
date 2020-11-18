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
#include <dns.h>
#include <init.h>
#include <startnet.h>
#include <stdio.h>
#include <string.h>
#include <tcp.h>

#ifdef _DEBUG
#include <NetworkDebug.h>
#endif

// Modify this defintion to specify your target SSL server by name or IP address
#define SSL_SERVER_NAME "10.1.1.193"
#define SSL_SERVER_PORT 4433

const char *AppName = "SSL Client Example";

/*-----------------------------------------------------------------------------
 * UserMain
 *----------------------------------------------------------------------------*/
void UserMain(void *pd)
{
    init();
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

    int nTimes = 1;
    int nFailed = 0;
    uint32_t startTick = 0;
    while (1)
    {
        IPADDR ipAddr;
        GetHostByName(SSL_SERVER_NAME, &ipAddr, IPADDR::NullIP(), TICKS_PER_SECOND * 15);

        iprintf("Attempting to connect to: %s:%d ...\r\n", SSL_SERVER_NAME, SSL_SERVER_PORT);
        iprintf("Connect start timer = %ld\r\n", TimeTick);
        startTick = TimeTick;

        // SSL Server IP, Local Port, Dest Port, Timeout (in timeticks), command name
        int fds = SSL_connect(ipAddr, 0, SSL_SERVER_PORT, TICKS_PER_SECOND * 20, SSL_SERVER_NAME);

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
                case SSL_ERROR_FAILED_NEGOTIATION:
                {
                    iprintf(" SSL_ERROR_FAILED_NEGOTIATION        \r\n");
                    break;
                }
                case SSL_ERROR_HASH_FAILED:
                {
                    iprintf(" SSL_ERROR_HASH_FAILED               \r\n");
                    break;
                }
                case SSL_ERROR_CERTIFICATE_UNKNOWN:
                {
                    iprintf(" SSL_ERROR_CERTIFICATE_UNKNOWN       \r\n");
                    break;
                }
                case SSL_ERROR_WRITE_FAIL:
                {
                    iprintf(" SSL_ERROR_WRITE_FAIL                \r\n");
                    break;
                }
                case SSL_ERROR_CERTIFICATE_NAME_FAILED:
                {
                    iprintf(" SSL_ERROR_CERTIFICATE_NAME_FAILED   \r\n");
                    break;
                }
                case SSL_ERROR_CERTIFICATE_VERIFY_FAILED:
                {
                    iprintf(" SSL_ERROR_CERTIFICATE_VERIFY_FAILED \r\n");
                    break;
                }
                default:
                {
                    iprintf("Other error\r\n");
                    break;
                }
            }

            nFailed++;
            nTimes++;
            iprintf("Hit any key to attempt another connection\r\n");
            getchar();
        }
    }

    iprintf("TICKS_PER_SECOND: %ld\r\n", TICKS_PER_SECOND);
    iprintf("Total time: %ld sytem ticks, approx %0.2f seconds\r\n", TimeTick - startTick,
            (float)(TimeTick - startTick) / (float)TICKS_PER_SECOND);
    iprintf("Reset to repeat test\r\n");

    while (1)
    {
        OSTimeDly(2);
    }
}
