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

#include <ftp.h>
#include <init.h>
#include <startnet.h>

#define SERVER_IP_ADDR "10.1.1.193"
#define FTP_PORT 21
#define USER "username"
#define PASS "password"
#define WRITE_TEST_FILE "WriteTest.txt"

const char *AppName = "FTPClient Example";

char tmp_resultbuff[255];

/**
 *  ShowFileContents
 *
 *  This function reads the data stream from the fd and displays it to
 *  stdout, the debug serial port on the NetBurner device.
 */
void ShowFileContents(int fdr)
{
    iprintf("\r\n[\r\n");
    int rv = 0;
    do
    {
        rv = ReadWithTimeout(fdr, tmp_resultbuff, 255, 20);
        iprintf("Bytes Read: %d", rv);
        if (rv >= 0)
        {
            tmp_resultbuff[rv] = 0;
            iprintf(", %s", tmp_resultbuff);
        }
    } while (rv > 0);
    iprintf("\r\n]\r\n");
}

/**
 *  UserMain
 *
 *  Main entry point to the program
 */
void UserMain(void *pd)
{
    init();

    // Open FTP session at specified IP Address with specified user name
    // and password. There is a 5 second timeout.
    int ftp = FTP_InitializeSession(AsciiToIp4(SERVER_IP_ADDR), FTP_PORT, USER, PASS, 5 * TICKS_PER_SECOND);

    if (ftp > 0)   // if the var ftp is > 0, it is the session handle
    {
        int rv = 0;

        // Change to the test directory, "test1".  It must exist for the example to run.
        rv = FTPSetDir(ftp, "test1", 5 * TICKS_PER_SECOND);
        if (rv == FTP_OK)
        {
            iprintf("Reading file names from test1 directory\r\n");
            int fdr = FTPGetList(ftp, nullptr, 5 * TICKS_PER_SECOND);
            if (fdr > 0)
            {
                ShowFileContents(fdr);
                close(fdr);
                // Read the command result code from the FTPGetList command
                rv = FTPGetCommandResult(ftp, tmp_resultbuff, 255, 5 * TICKS_PER_SECOND);
                if (rv != 226) { iprintf("Error Command result = %d %s\r\n", rv, tmp_resultbuff); }
            }
            else
            {
                iprintf("Failed to get file list\r\n");
            }

            // Create sample file
            iprintf("Now creating the sample file %s\r\n", WRITE_TEST_FILE);
            int fdw = FTPSendFile(ftp, WRITE_TEST_FILE, false, 5 * TICKS_PER_SECOND);
            if (fdw > 0)
            {
                writestring(fdw, "This is a test file\r\n");
                writestring(fdw, "This is line 2 of the test file\r\n");
                writestring(fdw, "Last Line\r\n");
                close(fdw);

                rv = FTPGetCommandResult(ftp, tmp_resultbuff, 255, 5 * TICKS_PER_SECOND);
                if (rv != 226) { iprintf("Error Command result = %d %s\r\n", rv, tmp_resultbuff); }

                iprintf("Now trying to read back the file we created \r\n");
                fdr = FTPGetFile(ftp, WRITE_TEST_FILE, false, 5 * TICKS_PER_SECOND);
                if (fdr > 0)
                {
                    ShowFileContents(fdr);
                    close(fdr);
                    // Read the command result code from the FTPGetFile command
                    rv = FTPGetCommandResult(ftp, tmp_resultbuff, 255, 5 * TICKS_PER_SECOND);
                    if (rv != 226) { iprintf("Error Command result = %d %s\r\n", rv, tmp_resultbuff); }
                }
                else
                {
                    iprintf("Failed to get file %s\r\n", WRITE_TEST_FILE);
                }
            }
            else
            {
                iprintf("Failed to create file %s\r\n", WRITE_TEST_FILE);
            }
        }
        else
        {
            iprintf("Failed to change to test directory");
        }

        FTP_CloseSession(ftp);
    }
    else
    {
        iprintf("Failed to open FTP Session\r\n");
        iprintf("The FTP Server IP address is set to: %s, with error %d\r\n", SERVER_IP_ADDR, ftp);
    }

    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND);
    }
}
