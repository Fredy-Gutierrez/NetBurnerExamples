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

// NB Libs
#include <crypto/ssl.h>
#include <ftpd.h>
#include <init.h>
#include <nbtime.h>
#include <netinterface.h>

#include "FileSystemUtils.h"
#include "effs_time.h"
#include "ftp_f.h"

#define FTP_PRIO (MAIN_PRIO - 2)

extern "C"
{
    void HTTPS_Ref(int sock, const char *url);
    void HTTP_Ref(int sock, const char *url);
    void DoCounter(int sock, PCSTR url);
    void webFormatFlashCard(int sock, const char *url);
}

const char *AppName = "HTTPS Dual Cert";

/**
 * @brief A trivial function that shows how many times a page has been reloaded!
 *
 * @param sock HTTPS socket.
 * @param url The calling URL.
 */
void DoCounter(int sock, PCSTR url)
{
    static uint32_t dw;
    char buffer[40];

    if (IsSSLfd(sock)) { sniprintf(buffer, 40, "%ld (SSL)", dw++); }
    else
    {
        sniprintf(buffer, 40, "%ld (HTTP)", dw++);
    }

    // Write out the value into the HTML page.
    writestring(sock, buffer);
}

/**
 * @brief Create web page link to HTTPS web page
 *
 * @param sock HTTPS socket.
 * @param url The calling URL.
 */
void HTTPS_Ref(int sock, const char *url)
{
    char buf[80];
    writestring(sock, "\"https://");
    InterfaceBlock *pifb = GetInterfaceBlock();
    if (pifb != nullptr)
    {
        sniprintf(buf, 80, "%hI", pifb->ip4.cur_addr.i4);
        writestring(sock, buf);
        writestring(sock, "/index.html\"");
    }
}

/**
 * @brief Create web page link to HTTP web page
 *
 * @param sock HTTPS socket.
 * @param url The calling URL.
 */
void HTTP_Ref(int sock, const char *url)
{
    char buf[80];
    writestring(sock, "\"http://");
    InterfaceBlock *pifb = GetInterfaceBlock();
    if (pifb != nullptr)
    {
        sniprintf(buf, 80, "%hI", pifb->ip4.cur_addr.i4);
        writestring(sock, buf);
        writestring(sock, "/index.html\"");
    }
}

/**
 * @brief Format SD Flash card
 *
 * To use this utility, you must type in the entire url:
 * http://<ip address>/format.htm.
 */
void webFormatFlashCard(int sock, const char *url)
{
    // iprintf("Called from task id: %d, Name: %s\r\n", OSTaskID(), OSTaskName() );
    FormatExtFlash(F_FAT32_FORMAT);
    DisplayEffsSpaceStats();
}

/**
 * UserMain
 */
void UserMain(void *pd)
{
    init();
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

    /**
     * The following call to f_enterFS() must be called in every task that accesses
     * the file system.  This must only be called once in each task and must be done before
     * any other file system functions are used.  Up to 10 tasks can be assigned to use
     * the file system. Any task may also be removed from accessing the file system with a
     * call to the function f_releaseFS().
     */

    OSChangePrio(FTP_PRIO);
    f_enterFS();   // Init file system for FTP task

    OSChangePrio(HTTP_PRIO);
    f_enterFS();

    OSChangePrio(MAIN_PRIO);
    f_enterFS();   // Init file system for UserMain task

    iprintf("Starting HTTPS, listening on ports 443 and 80\r\n");
    StartHttps(443, 80);

    InitExtFlash();   // Initialize the CFC or SD/MMC external flash drive

    // Try setting the time via a NTP Server, if that fails, set it manually
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
        // month, day, dow, year, hour, min, sec
        SetTimeManual(5, 14, 3, 2006, 11, 35, 0);
    }
    DisplaySystemTime();
    iprintf("\r\n");

    DisplayEffsSpaceStats();   // Display file space usage
    DumpDir();                 // Display flash card files and directories

    // This is where the read/write file system calls are made
    ReadWriteTest();

    // Start FTP server with task priority higher than UserMain()
    int status = FTPDStart(21, FTP_PRIO);
    if (status == FTPD_OK)
    {
        iprintf("Started FTP Server\r\n");
        InterfaceBlock *pifb = GetInterfaceBlock();
        if (pifb != nullptr)
        {
            iprintf("You can use a FTP client or Windows File Manager to drag and drop files to url \"ftp://%I\"\r\n",
                    pifb->ip4.cur_addr.i4);
        }
        else
        {
            iprintf("Unable to update get device IP address.\r\n");
        }

        if (F_LONGFILENAME == 1) { iprintf("Long file names are supported\r\n"); }
        else
        {
            iprintf("Long file names are not supported- only 8.3 format\r\n");
        }
    }
    else
    {
        iprintf("** Error: %d. Could not start FTP Server\r\n", status);
    }

    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND);
    }
}
