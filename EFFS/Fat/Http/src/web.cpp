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

#include <effs_fat/fat.h>
#include <startnet.h>

#include "cardtype.h"
#include "http_f.h"

#if (defined(USE_MMC) && defined(MOD5441X))
#define MULTI_MMC TRUE   // For modules with onboard flash sockets, even if you are using external flash cards
#include <effs_fat/multi_drive_mmc_mcf.h>
#elif (defined(USE_MMC))
#include <effs_fat/mmc_mcf.h>
#elif (defined(USE_CFC))
#include <effs_fat/cfc_mcf.h>
#endif

#define HTTP_BUFFER_SIZE (32 * 1024)   // Make a 32KB BUFFER
static char HTTP_buffer[HTTP_BUFFER_SIZE] __attribute__((aligned(16)));

#define tmp_buffer_size (256)
char tmp_buffer[tmp_buffer_size];
int tmp_buffer_end = 0;
int tmp_buffer_start = 0;

/**
 *  @brief Sends the length specified of a file to the given socket.
 */
void SendFragment(int sock, F_FILE *f, long len)
{
    int lread = 0;
    while (lread < len)
    {
        int ltoread = len - lread;
        int lr;

        if (ltoread > HTTP_BUFFER_SIZE) { ltoread = HTTP_BUFFER_SIZE; }

        lr = f_read(HTTP_buffer, 1, HTTP_BUFFER_SIZE, f);

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

    F_FIND f;
    volatile int rc = f_findfirst("*.*", &f);

    while (rc == F_NO_ERROR)
    {
        if (f.attr & F_ATTR_DIR)
        {
            writestring(sock, "         <li><img src=\"/folder.gif\"><a href=\"");
            writestring(sock, f.filename);
            writestring(sock, "/DIR\">");
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

        rc = f_findnext(&f);
    }

    writestring(sock, "      </font></ul>\r\n");
    writestring(sock, "      <hr>\r\n");
    writestring(sock, "   </body>\r\n");
    writestring(sock, "</html>");
}

/**
 * @brief Reads and returns 1 line from the file FP.
 */
int my_f_read_line(char *buffer, int buf_siz, F_FILE *fp)
{
    int nr = 0;
    do
    {
        if (tmp_buffer_end <= tmp_buffer_start)
        {
            if (f_eof(fp)) { return 0; }

            int n = f_read(tmp_buffer, 1, tmp_buffer_size, fp);
            tmp_buffer_start = 0;
            tmp_buffer_end = n;

            if (n == 0)
            {
                buffer[nr + 1] = 0;
                return nr;
            }
        }

        *(buffer + nr) = tmp_buffer[tmp_buffer_start++];

        if ((buffer[nr] == '\r') || (buffer[nr] == '\n'))
        {
            if (nr != 0)
            {
                buffer[nr + 1] = 0;
                return nr;
            }
        }
        else
        {
            nr++;
        }
    } while (nr < buf_siz);

    buffer[nr + 1] = 0;
    return nr;
}

/**
 * @brief Takes a file type and sends a header response with the specified MIME type.
 *
 * Normally the NetBurner HTTP code will send the proper header response based on the MIME_magic.txt
 * file located in `\nburn\pcbin`. However, if the file is read from the flash card, the HTTP server code
 * does not know about the type.
 *
 * This example function adds support for a subset of common file types in 2 ways. First, it tries to find
 * a file called `MIME.txt` on the SD card. This file looks like the following:
 *     -------MIME.txt--------
 *     jpg     image/jpeg
 *     html    text/html
 *     ...
 *     xml     text/xml
 *
 * Note that MIME types are not cached, and numerous lookups on a large `MIME.txt` file can be costly.
 * Only include the minimum number of file types you wish to support. Order matters as well, so
 * common file types should be located at the top
 *
 * If the file type is located, then the specified MIME type is sent.
 *
 * If the MIME lookup fails, a secondary MIME lookup occurs with hard-coded values.
 *
 * @returns The number of bytes written to socket.
 */
int SendEFFSCustomHeaderResponse(int sock, char *fType)
{
    char mime_type[64];
    bool found = false;

    // Check for MIME.txt file, which lists support mime types
    F_FILE *f = f_open("MIME.txt", "r");
    if (f != nullptr)
    {
        char line[255] = "";
        while (my_f_read_line(line, 255, f) != 0 && !found)
        {
            if (line[0] == '#' || line[0] == ' ' || line[0] == '\0')
            {
                continue;   // Comment
            }
            char *pch = strtok(line, " \t\n\r");
            if (strcasecmp(fType, pch) == 0)
            {
                // Found file type
                pch = strtok(nullptr, " \t\n\r");
                sniprintf(mime_type, 64, pch);
                found = true;
            }
        }
    }
    if (!found)
    {
        // no MIME.txt found or extension type not found, fall back to hard-coded list
        found = true;   // Set to true. Revert to false if not found in default else.
        if (strcasecmp(fType, "jpg") == 0) { sniprintf(mime_type, 64, "image/jpeg"); }
        else if (strcasecmp(fType, "gif") == 0)
        {
            sniprintf(mime_type, 64, "image/gif");
        }
        else if (strcasecmp(fType, "htm") == 0)
        {
            sniprintf(mime_type, 64, "text/html");
        }
        else if (strcasecmp(fType, "html") == 0)
        {
            sniprintf(mime_type, 64, "text/html");
        }
        else if (strcasecmp(fType, "xml") == 0)
        {
            sniprintf(mime_type, 64, "text/xml");
        }
        else if (strcasecmp(fType, "css") == 0)
        {
            sniprintf(mime_type, 64, "text/css");
        }
        else if (strcasecmp(fType, "mp4") == 0)
        {
            sniprintf(mime_type, 64, "video/mp4");
        }
        else
        {
            found = false;
        }
    }

    char buffer[255];
    if (found)
    {
        sniprintf(buffer, 255,
                  "HTTP/1.0 200 OK\r\n"
                  "Pragma: no-cache\r\n"
                  "MIME-version: 1.0\r\n"
                  "Content-Type: %s\r\n\r\n",
                  mime_type);
    }
    else
    {
        // If MIME type is not found, don't send any MIME type. This allows the browser to
        // make a best guess.
        sniprintf(buffer, 255, "HTTP/1.0 200 OK\r\nPragma: no-cache\r\n\r\n");
    }
    int bytes = writestring(sock, buffer);
    return bytes;
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

#if (defined(USE_MMC))
    f_chdrive(MMC_DRV_NUM);
#elif (defined(USE_CFC))
    f_chdrive(CFC_DRV_NUM);
#endif

    f_chdir("\\");
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

    F_FIND f;
    bool fnd = false;
    if (f_chdir(dir_buffer) == F_NO_ERROR)
    {
        // If nothing was specified, try for index.ht*
        if ((name_buffer[0] == 0) && (dir_buffer[1] == 0)) { fnd = (f_findfirst("index.ht*", &f) == 0); }
        else
        {
            fnd = (f_findfirst(pName, &f) == 0);
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
    char name_buffer[257] = {};
    char dir_buffer[256] = {};
    char ext_buffer[10] = {};

#if (defined(USE_MMC))
    f_chdrive(MMC_DRV_NUM);
#elif (defined(USE_CFC))
    f_chdrive(CFC_DRV_NUM);
#endif

    f_chdir("\\");
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
    if (f_chdir(dir_buffer) == F_NO_ERROR)
    {
        F_FIND fnd;
        if (name_buffer[0] == 0)
        {
            if (dir_buffer[1] == 0)
            {
                // Root file try index.ht* first
                if (f_findfirst("index.ht*", &fnd) == 0) { pName = fnd.filename; }
            }
        }

        // A file name was specified in the URL, so attempt to open it
        F_FILE *f = f_open(pName, "r");
        if (f != nullptr)
        {
            long len = f_filelength(pName);
            SendEFFSCustomHeaderResponse(sock, ext_buffer);
            SendFragment(sock, f, len);
            f_close(f);
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
