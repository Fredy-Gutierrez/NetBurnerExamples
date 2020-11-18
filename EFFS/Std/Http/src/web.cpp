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

/**
 *  This module contains code for the web server interface.
 */

#include <startnet.h>

#include "effs_std.h"
#include "http.h"

#define HTTP_BUFFER_SIZE (32 * 1024)   // Make a 32kB buffer
static char HTTP_buffer[HTTP_BUFFER_SIZE] __attribute__((aligned(16)));

/**
 *  @brief Sends the length specified of a file to the given socket.
 */
void SendFragment(int sock, FS_FILE *f, long len)
{
    int lread = 0;
    while (lread < len)
    {
        int ltoread = (len - lread);
        int lr;

        if (ltoread > HTTP_BUFFER_SIZE) { ltoread = HTTP_BUFFER_SIZE; }

        lr = fs_read(HTTP_buffer, 1, HTTP_BUFFER_SIZE, f);

        if (lr == 0) { return; }

        lread += lr;
        writeall(sock, HTTP_buffer, lr);
    }
}

/**
 * @brief Displays a list of directories and files to the web browser.
 */
void WebListDir(int sock, const char *dir)
{
    writestring(sock, "HTTP/1.0 200 OK\r\n");
    writestring(sock, "Pragma: no-cache\r\n");
    writestring(sock, "MIME-version: 1.0\r\n");
    writestring(sock, "Content-Type: text/html\r\n\r\n");
    writestring(sock, "<html>\r\n");
    writestring(sock, "   <body>\r\n");
    writestring(sock, "      <h2><font face=\"Arial\">Directory of ");
    writestring(sock, dir);
    writestring(sock, "</font></h2>\r\n");
    writestring(sock, "      <hr>\r\n");
    writestring(sock, "      <ul><font face=\"Courier New\" size=\"2\">\r\n");

    FS_FIND f;
    int rc = fs_findfirst("*.*", &f);

    while (rc == 0)
    {
        if (f.attr & FS_ATTR_DIR)
        {
            writestring(sock, "         <li><img src=\"/folder.gif\"><a href=\"");
            writestring(sock, f.filename);
            writestring(sock, "/\">");
            writestring(sock, f.filename);
            writestring(sock, "</a>\r\n");
        }
        else
        {
            writestring(sock, "         <li><img src=\"/text.gif\"><a href=\"");
            writestring(sock, f.filename);
            writestring(sock, "\">");
            writestring(sock, f.filename);
            writestring(sock, "</a>\r\n");
        }

        rc = fs_findnext(&f);
    }

    writestring(sock, "      </font></ul>\r\n");
    writestring(sock, "      <hr>\r\n");
    writestring(sock, "   </body>\r\n");
    writestring(sock, "</html>");
}

/**
 * @brief A callback function that determines if the URL has a match or not. The processing
 * is as follows:
 *     - No URL -> Look for index.ht*
 *     - URL -> Look for the file on the SD card
 *     - Look for "DIR" in the URL to see if we should send the file system information
 *
 * If we are able to find the file, we return true, and the HandleGet() callback will get called.
 * If we return false, the compiled resources will get checked to see if there is anything
 * specified by the application.
 *
 * @param pr The HTTP request object associated with the request.
 *
 * @retval true If the file was found.
 * @retval false If the file was not found.
 */
bool HandleMatch(HTTP_Request &pr)
{
    char name_buffer[257] = {};
    char dir_buffer[256] = {};
    char ext_buffer[10] = {};

    fs_chdrive(NOR_DRV_NUM);
    fs_chdir("\\");

    iprintf("Processing HandleMatch()\r\n");

    // Parse and store file extension portion of URL
    iprintf("  URL: \"%s\"\r\n", pr.pURL);
    char *pext = pr.pURL + strlen(pr.pURL);

    while ((*pext != '.') && (*pext != '/') && (*pext != '\\') && (pext > pr.pURL))
    {
        pext--;
    }

    if ((*pext == '.') || (*pext == '\\') || (*pext == '/')) { pext++; }

    strncpy(ext_buffer, pext, 9);
    iprintf("  URL extension: \"%s\"\r\n", ext_buffer);

    // Parse and store file name portion of URL
    char *pName = pr.pURL + strlen(pr.pURL);

    while ((*pName != '/') && (*pName != '\\') && (pName > pr.pURL))
    {
        pName--;
    }

    if ((*pName == '\\') || (*pName == '/')) { pName++; }

    strncpy(name_buffer, pName, 256);
    iprintf("  URL file name: \"%s\"\r\n", name_buffer);

    // Store directory portion of URL
    strncpy(dir_buffer + 1, pr.pURL, (pName - pr.pURL));
    dir_buffer[0] = '/';
    dir_buffer[(pName - pr.pURL) + 1] = 0;
    iprintf("  URL directory portion: \"%s\"\r\n", dir_buffer);

    FS_FIND f;
    bool fnd = false;
    if (fs_chdir(dir_buffer) == F_NO_ERROR)
    {
        // If nothing was specified, try for index.ht*
        if ((name_buffer[0] == 0) && (dir_buffer[1] == 0)) { fnd = (fs_findfirst("index.ht*", &f) == 0); }
        else
        {
            fnd = (fs_findfirst(pName, &f) == 0);
        }

        // See if we have a DIR in the URL
        // httpstricmp returns 0 if they don't match, and 1 if they do, but we
        if (!fnd) { fnd = (bool)httpstricmp(pName, "DIR"); }

        iprintf("Found the file on the SD card: %s\r\n", fnd ? "true" : "false");
        return fnd;
    }

    return false;
}

/**
 * @brief A GET callback function that displays the SD file system in the browser. Note that
 * this function only gets called if HandleMatch() returns true, otherwise the compiled resources
 * are checked. This order is dictated by the parameters when gHandleGet is constructed.
 *
 * @param sock HTTP socket.
 * @param pr The HTTP request object associated with the request.
 *
 * @retval 0 If the file was found and sent.
 * @retval -1 If the file was not found or unable to be sent.
 */
int HandleGet(int sock, HTTP_Request &pr)
{
    char name_buffer[257] = {};   // Extra byte reserved for null termination
    char dir_buffer[256] = {};
    char ext_buffer[10] = {};

    fs_chdrive(NOR_DRV_NUM);
    fs_chdir("\\");

    iprintf("Processing HandleGet()\r\n");

    // Parse and store file extension portion of URL
    iprintf("  URL: \"%s\"\r\n", pr.pURL);
    char *pext = pr.pURL + strlen(pr.pURL);

    while ((*pext != '.') && (*pext != '/') && (*pext != '\\') && (pext > pr.pURL))
    {
        pext--;
    }

    if ((*pext == '.') || (*pext == '\\') || (*pext == '/')) { pext++; }

    strncpy(ext_buffer, pext, 9);
    iprintf("  URL extension: \"%s\"\r\n", ext_buffer);

    // Parse and store file name portion of URL
    char *pName = pr.pURL + strlen(pr.pURL);

    while ((*pName != '/') && (*pName != '\\') && (pName > pr.pURL))
    {
        pName--;
    }

    if ((*pName == '\\') || (*pName == '/')) { pName++; }

    strncpy(name_buffer, pName, 256);
    iprintf("  URL file name: \"%s\"\r\n", name_buffer);

    // Store directory portion of URL
    strncpy(dir_buffer + 1, pr.pURL, (pName - pr.pURL));
    dir_buffer[0] = '/';
    dir_buffer[(pName - pr.pURL) + 1] = 0;
    iprintf("  URL directory portion: \"%s\"\r\n", dir_buffer);

    // Try to locate the specified file on the flash card. If no file name is
    // given, then search for an html file begining with index.ht*:
    if (fs_chdir(dir_buffer) == FS_NO_ERROR)
    {
        FS_FIND fnd;
        if (name_buffer[0] == 0)
        {
            if (dir_buffer[1] == 0)
            {
                // Root file try index.ht* first
                if (fs_findfirst("index.ht*", &fnd) == 0) { pName = fnd.filename; }
            }
        }

        // A file name was specified in the URL, so attempt to open it
        FS_FILE *f = fs_open(pName, "r");
        if (f != nullptr)
        {
            long len = fs_filelength(pName);
            SendFragment(sock, f, len);
            fs_close(f);
            close(sock);
            iprintf(" File sent to browser\r\n");
            return 1;
        }

        // If the work "DIR" is specified in the URL at any directory level,
        // then this code will result in a directory listing
        if (httpstricmp(pName, "DIR"))
        {
            WebListDir(sock, dir_buffer);
            return 1;
        }
    }

    return 0;
}

// Set the callback for GET. We want the SD cards' resource to get checked first, so the last
// parameter is set to true. Note that this is not ideal, and will decrease the performance of the
// web server due to the time required to search the file system. It would be better for
// the resources to either be compiled into the application image, or for there to be a
// representation of the file system stored in memory that can be used to determine whether
// or not resources are present.
CallBackFunctionPageHandler gHandleGet("*", HandleGet, HandleMatch, tGet, 0, true);
