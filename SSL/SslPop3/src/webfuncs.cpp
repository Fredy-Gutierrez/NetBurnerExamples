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
 * @file SSL\SslPop3\src\webfuncs.cpp
 *
 * @brief This code module contains the web functions for the POP3 mail using SSL example program.
 */

// NB Libs
#include <crypto/ssl_pop3.h>
#include <dns.h>
#include <effs_fat/fat.h>
#include <httppost.h>
#include <mailto.h>
#include <netinterface.h>
#include <stdlib.h>

#include "FileSystemUtils.h"
#include "cardtype.h"

// Email variables
char var_user[80];
char var_pass[80];
char var_svrip[80];
char var_port[4];
char var_nummsg[4];
int GetMailResult;
int DNSResult;
int emailsSaved = 0, emailsTooBig = 0;

#define HTTP_BUFFER_SIZE (32 * 1024)   // Make a 32KB BUFFER
static char HTTP_buffer[HTTP_BUFFER_SIZE] __attribute__((aligned(16)));
char html_string[HTTP_BUFFER_SIZE + 30];

extern "C"
{
    void WebDisplayDhcpSelect(int sock, PCSTR url);
    void WebShowDhcpDeviceIpAddress(int sock, PCSTR url);
    void WebShowStaticDeviceIpAddress(int sock, PCSTR url);
    void WebShowDhcpDeviceIpMask(int sock, PCSTR url);
    void WebShowStaticDeviceIpMask(int sock, PCSTR url);
    void WebShowDhcpDeviceGateway(int sock, PCSTR url);
    void WebShowStaticDeviceGateway(int sock, PCSTR url);
    void WebShowDhcpDeviceDnsServer(int sock, PCSTR url);
    void WebShowStaticDeviceDnsServer(int sock, PCSTR url);
    void WebShowDeviceName(int sock, PCSTR url);

    void WebShowUserValue(int sock, PCSTR url);
    void WebShowUserPass(int sock, PCSTR url);
    void WebShowUserValue(int sock, PCSTR url);
    void WebShowServer(int sock, PCSTR url);
    void LastResult(int sock, PCSTR url);
}

/**
 * @brief Displays if the network values are statically set or
 * assigned by DHCP.
 *
 * @param sock HTTP Socket
 * @param url Calling page
 */
void WebDisplayDhcpSelect(int sock, PCSTR url)
{
    int ifNum = GetFirstInterface();
    InterfaceBlock *pIfB = GetInterfaceBlock(ifNum);
    if (pIfB != nullptr)
    {
        if (pIfB->ip4.mode.IsSelected("DHCP"))
        {
            writestring(sock, " DHCP\r\n");
            return;
        }
    }

    writestring(sock, " STATIC\r\n");
}

/**
 * @brief Displays the active IP value.
 *
 * @param sock HTTP Socket
 * @param url Calling page
 */
void WebShowDhcpDeviceIpAddress(int sock, PCSTR url)
{
    char buf[80];
    int ifNum = GetFirstInterface();
    sniprintf(buf, 80, " value=\"");
    writestring(sock, buf);
    InterfaceIP(ifNum).sprintf(buf, 80);
    writestring(sock, buf);
    sniprintf(buf, 80, "\"");
    writestring(sock, buf);
}

/**
 * @brief Displays the static IP value.
 *
 * @param sock HTTP Socket
 * @param url Calling page
 */
void WebShowStaticDeviceIpAddress(int sock, PCSTR url)
{
    char buf[80];
    int ifNum = GetFirstInterface();
    InterfaceBlock *pIfB = GetInterfaceBlock(ifNum);
    if (pIfB != nullptr)
    {
        sniprintf(buf, 80, " value=\"");
        writestring(sock, buf);
        ((IPADDR4)pIfB->ip4.addr).sprintf(buf, 80);
        writestring(sock, buf);
        sniprintf(buf, 80, "\"");
        writestring(sock, buf);
    }
}

/**
 * @brief Displays the active IP mask value.
 *
 * @param sock HTTP Socket
 * @param url Calling page
 */
void WebShowDhcpDeviceIpMask(int sock, PCSTR url)
{
    char buf[80];
    int ifNum = GetFirstInterface();
    sniprintf(buf, 80, " value=\"");
    writestring(sock, buf);
    InterfaceMASK(ifNum).sprintf(buf, 80);
    writestring(sock, buf);
    sniprintf(buf, 80, "\"");
    writestring(sock, buf);
}

/**
 * @brief Displays the static IP mask value.
 *
 * @param sock HTTP Socket
 * @param url Calling page
 */
void WebShowStaticDeviceIpMask(int sock, PCSTR url)
{
    char buf[80];
    int ifNum = GetFirstInterface();
    InterfaceBlock *pIfB = GetInterfaceBlock(ifNum);
    if (pIfB != nullptr)
    {
        sniprintf(buf, 80, " value=\"");
        writestring(sock, buf);
        ((IPADDR4)pIfB->ip4.mask).sprintf(buf, 80);
        writestring(sock, buf);
        sniprintf(buf, 80, "\"");
        writestring(sock, buf);
    }
}

/**
 * @brief Displays the active gateway value.
 *
 * @param sock HTTP Socket
 * @param url Calling page
 */
void WebShowDhcpDeviceGateway(int sock, PCSTR url)
{
    char buf[80];
    int ifNum = GetFirstInterface();
    sniprintf(buf, 80, " value=\"");
    writestring(sock, buf);
    InterfaceGate(ifNum).sprintf(buf, 80);
    writestring(sock, buf);
    sniprintf(buf, 80, "\"");
    writestring(sock, buf);
}

/**
 * @brief Displays the static gateway value.
 *
 * @param sock HTTP Socket
 * @param url Calling page
 */
void WebShowStaticDeviceGateway(int sock, PCSTR url)
{
    char buf[80];
    int ifNum = GetFirstInterface();
    InterfaceBlock *pIfB = GetInterfaceBlock(ifNum);
    if (pIfB != nullptr)
    {
        sniprintf(buf, 80, " value=\"");
        writestring(sock, buf);
        ((IPADDR4)pIfB->ip4.gate).sprintf(buf, 80);
        writestring(sock, buf);
        sniprintf(buf, 80, "\"");
        writestring(sock, buf);
    }
}

/**
 * @brief Displays the active DNS server value.
 *
 * @param sock HTTP Socket
 * @param url Calling page
 */
void WebShowDhcpDeviceDnsServer(int sock, PCSTR url)
{
    char buf[80];
    int ifNum = GetFirstInterface();
    sniprintf(buf, 80, " value=\"");
    writestring(sock, buf);
    InterfaceDNS(ifNum).sprintf(buf, 80);
    writestring(sock, buf);
    sniprintf(buf, 80, "\"");
    writestring(sock, buf);
}

/**
 * @brief Displays the static DNS server value.
 *
 * @param sock HTTP Socket
 * @param url Calling page
 */
void WebShowStaticDeviceDnsServer(int sock, PCSTR url)
{
    char buf[80];
    int ifNum = GetFirstInterface();
    InterfaceBlock *pIfB = GetInterfaceBlock(ifNum);
    if (pIfB != nullptr)
    {
        sniprintf(buf, 80, " value=\"");
        writestring(sock, buf);
        ((IPADDR4)pIfB->ip4.dns1).sprintf(buf, 80);
        writestring(sock, buf);
        sniprintf(buf, 80, "\"");
        writestring(sock, buf);
    }
}

/**
 * @brief Displays the device's name form value.
 *
 * @param sock HTTP Socket
 * @param url Calling page
 */
void WebShowDeviceName(int sock, PCSTR url)
{
    int ifNum = GetFirstInterface();
    InterfaceBlock *pIfB = GetInterfaceBlock(ifNum);
    if (pIfB != nullptr) { writestring(sock, pIfB->device_name.c_str()); }
}

/**
 * @brief Displays the current username form value.
 *
 * @param sock HTTP Socket
 * @param url Calling page
 */
void WebShowUserValue(int sock, PCSTR url)
{
    writestring(sock, var_user);
}

/**
* @brief Displays the current user password form value.
*
* @param sock HTTP Socket
* @param url Calling page
*/
void WebShowUserPass(int sock, PCSTR url)
{
    writestring(sock, var_pass);
}

/**
 * @brief Displays the current destination server form value.
 *
 * @param sock HTTP Socket
 * @param url Calling page
 */
void WebShowServer(int sock, PCSTR url)
{
    writestring(sock, var_svrip);
}

/**
 * @brief Displays the results of the request and any associated information with the request.
 * If email messages were successfully returned, then links to those pages will be added to the
 * page.
 *
 * @param sock HTTP Socket
 * @param url Calling page
 */
void LastResult(int sock, PCSTR url)
{
    char buf[100];

    if (GetMailResult)   // Result from SendMailAuth()
    {
        writestring(sock, "<h1>Mail Retrieved Successfully</h1>\r\n");

        int ifNum = GetFirstInterface();

        for (int n = 1; n <= emailsSaved; n++)
        {
            sniprintf(buf, 100, "<br><a href=\"http://");
            writestring(sock, buf);
            InterfaceIP(ifNum).sprintf(buf, 100);
            writestring(sock, buf);
            sniprintf(buf, 100, "/email%d.html\">email%d</a><br>", n, n);
            writestring(sock, buf);
        }
    }
    else
    {
        writestring(sock, "<h1>Error - Could Not Retrieve Mail</h1>\r\n");
        writestring(sock, "DETAILS:");
        if (DNSResult == DNS_OK)
        {
            sniprintf(buf, 100, "\r\n<br>DNS lookup of \"%s\" succeeded\r\n", var_svrip);
            writestring(sock, buf);
        }
        else
        {
            sniprintf(buf, 100, "\r\n<br>DNS lookup of \"%s\" FAILED\r\n", var_svrip);
            writestring(sock, buf);
        }
    }

    if (IsMailError())
    {
        writestring(sock, "<pre>");   // pre-format to properly handle \r\n in string
        PrintNBError(sock);
        PrintServerLog(sock);
        writestring(sock, "<pre>");   // pre-format to properly handle \r\n in string
    }
}

/**
 * @brief POP3_GetMessages
 *
 * @param session The session number associated with the connection.
 */
void POP3_GetMessages(int session)
{
    uint32_t num_mess;
    uint32_t num_bytes;

    iprintf("Message buffer size set to %ld bytes\r\n", HTTP_BUFFER_SIZE);
    OSTimeDly(TICKS_PER_SECOND / 2);

    if (session > 0)
    {
        int rv = POP3_StatCmd(session, &num_mess, &num_bytes, 10 * TICKS_PER_SECOND);
        if (rv == POP_OK)
        {
            iprintf("The server has %ld messages and %ld bytes of data\r\n", num_mess, num_bytes);
            if (num_mess == 0) return;
            if (num_mess > (uint32_t)atoi(var_nummsg)) num_mess = atoi(var_nummsg);

            iprintf("Number of msgs to retrieve: %ld\r\n", num_mess);

            for (uint32_t nmsg = 1; emailsSaved < (int)num_mess; nmsg++)
            {
                char *pSubject;
                char *pBody;
                uint32_t predict_size;

                rv = POP3_ListCmd(session, nmsg, &predict_size, TICKS_PER_SECOND * 10);
                if (rv != POP_OK)
                {
                    iprintf("pop list cmd error line 267");
                    break;
                }
                iprintf("Predicted message size is %ld\r\n", predict_size);

                if (predict_size <= HTTP_BUFFER_SIZE)
                {
                    iprintf("Retrieving message...\n\r");
                    rv = POP3_RetrieveMessage(session, nmsg, HTTP_buffer, &pSubject, &pBody, HTTP_BUFFER_SIZE - 1, TICKS_PER_SECOND * 20);

                    if (rv > 0)
                    {
                        iprintf("\r\n===== Processing message %d =====\r\n", nmsg);
                        HTTP_buffer[rv] = '\0';

                        // write email to sd card
                        emailsSaved++;
                        char filename[80];
                        sniprintf(filename, 80, "email%d.html", emailsSaved);
                        sniprintf(html_string, HTTP_BUFFER_SIZE + 30, "<html><body>\n<pre>%s</pre>\n</body></html>", HTTP_buffer);
                        WriteFile((uint8_t *)html_string, filename, rv + 30);
                        iprintf("Saved email as file name %s\n\r", filename);
                        DumpDir();

                        /*if ( pSubject )
                         {
                            iprintf("Subject Size: %ld bytes\r\n<Start of Subject:>\r\n %s\r\n<End of Subject>\r\n",
                            strlen(pSubject), pSubject);
                         }
                         else
                         {
                            iprintf("Unable to locate Subject.\r\nPrinting the entire message \r\n<Start of Subject:>\r\n %s\r\n<End of
                            Message>\r\n", pBody);
                         }
                         HTTP_buffer[rv] = 0;
                         if ( pBody )
                         {
                            iprintf("Body Size: %ld bytes\r\n<Start of Body:>\r\n %s\r\n<End of Body>\r\n",
                            strlen(pBody), pBody);
                         }
                         else
                         {
                            iprintf("Unable to locate Body.\r\nPrinting the entire message \r\n<Start of Body:>\r\n %s\r\n<End of
                         Body>\r\n", pBody);
                         }*/

                        iprintf("===== End of processing for message %d =====\r\n", nmsg);
                    }

                    /*
                     char c;
                     do
                     {
                        iprintf("Delete this message (Y/N?)");
                        c = toupper( getchar() );
                     } while ( (c != 'N') && (c != 'Y') );

                     if (c=='Y')
                     {
                        rv = POP3_DeleteCmd( session, nmsg, TICKS_PER_SECOND*10 );
                        if ( rv == POP_OK){ iprintf("Message deleted\r\n"); }
                        else
                        {
                            iprintf("Delete command processing failed with error:%s\r\n",GetPOPErrorString(rv));
                        }
                     }
                     else{ iprintf("Message not deleted\r\n"); }
                     */

                    else
                        iprintf("Retrieve command processing failed with error:%s\r\n", GetPOPErrorString(rv));
                }
                else
                {
                    iprintf("This message is bigger than the allocated buffer size of %d bytes\r\n\n", HTTP_BUFFER_SIZE);
                    emailsTooBig++;
                }
            }
        }
        else
        {
            iprintf("STAT command processing failed with error:%s\r\n", GetPOPErrorString(rv));
        }

        POP3_CloseSession(session);
        iprintf("message retrieval complete");
    }
    else
    {
        iprintf("Failed to create session with error:%s\r\n", GetPOPErrorString(session));
    }
}

/**
 * @brief Handles the post for the get mail post request. This callback is called for each field
 * of a post form.
 *
 * @param sock HTTP socket.
 * @param event The kind of post event that is currently being handled with this callback.
 * @param pName The name of the post element that is currently being handled.
 * @param pValue The value of the post element that is currently being handled.
 */
void HandleGetMailPost(int sock, PostEvents event, const char *pName, const char *pValue)
{
#ifdef USE_MMC
    f_chdrive(MMC_DRV_NUM);
#endif

#ifdef USE_CFC
    f_chdrive(CFC_DRV_NUM);
#endif

    f_chdir("\\");

    if (event == eStartingPost)
    {
        var_user[0] = 0;
        var_pass[0] = 0;
        var_svrip[0] = 0;
        var_port[0] = 0;
        var_nummsg[0] = 0;

        GetMailResult = 0;
    }
    else if (event == eVariable)
    {
        if (strcmp(pName, "user") == 0) { snprintf(var_user, 80, pValue); }
        else if (strcmp(pName, "pass") == 0) { snprintf(var_pass, 80, pValue); }
        else if (strcmp(pName, "svrip") == 0) { snprintf(var_svrip, 80, pValue); }
        else if (strcmp(pName, "port") == 0) { snprintf(var_port, 4, pValue); }
        else if (strcmp(pName, "nummsg") == 0) { snprintf(var_nummsg, 4, pValue); }
        else { iprintf("Unknown field of \"%s\" with value \"%s\"", pName, pValue); }
    }
    else if (event == eEndOfPost)
    {
        iprintf("End of post data extraction\r\n");
        IPADDR POP3_ServerIp;
        DNSResult = GetHostByName(var_svrip, &POP3_ServerIp, IPADDR::NullIP(), TICKS_PER_SECOND * 20);
        const uint16_t POP3_Port = atoi(var_port);   // default port number for POP3
        if (DNSResult == DNS_OK)
        {
            int POP3_Session = SSL_POP3_InitializeSession(POP3_ServerIp, POP3_Port, var_user, var_pass, TICKS_PER_SECOND * 10);
            if (POP3_Session > 0)
            {
                GetMailResult = 1;
                iprintf("calling: POP3_GetMessages\r\n");   // debug
                POP3_GetMessages(POP3_Session);
            }
            else
            {
                switch (POP3_Session)
                {
                    case POP_TIMEOUT: iprintf("Timeout\r\n"); break;
                    case POP_PASSWORDERROR: iprintf("Password Error: \"%s\"\r\n", var_pass); break;
                    case POP_CONNECTFAIL: iprintf("Connection Failed..\r\n"); break;
                    case POP_COMMANDFAIL: iprintf("Command Failed\r\n"); break;
                    case POP_BADSESSION: iprintf("Bad Session\r\n"); break;
                    case POP_NETWORKERROR: iprintf("Network Error\r\n"); break;
                    default: iprintf("Unknown Error: %d\r\n", POP3_Session);
                }
            }
        }
        else
        {
            iprintf("Error - DNS failed for %s\r\n", var_svrip);
        }

        RedirectResponse(sock, "sent.html");
    }
}

/**
 * @brief A GET callback function that displays the email saved on the file system in the browser.
 *
 * @param sock HTTP socket.
 * @param pr The HTTP request object associated with the request.
 *
 * @retval 0 If the file was found and sent.
 * @retval -1 If the file was not found or unable to be sent.
 */
int HandleMailGet(int sock, HTTP_Request &pr)
{
    char name_buffer[257] = {};   // Reserve last byte for null character
    char dir_buffer[256] = {};
    char ext_buffer[10] = {};
    char out_buffer[255] = {};

#ifdef USE_MMC
    f_chdrive(MMC_DRV_NUM);
#endif

#ifdef USE_CFC
    f_chdrive(CFC_DRV_NUM);
#endif

    f_chdir("\\");

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

    iprintf("  Attempting to open file \"%s\"...", pName);
    F_FILE *file = f_open(pName, "r");

    if (file != nullptr)
    {
        long len = f_filelength(pName);
        sniprintf(out_buffer, 255,
                  "HTTP/1.0 200 OK\r\n"
                  "Pragma: no-cache\r\n"
                  "MIME-version: 1.0\r\n"
                  "Content-Type: %s\r\n\r\n"
                  "text/html");
        writestring(sock, out_buffer);

        int lread = 0;
        while (lread < len)
        {
            int ltoread = len - lread;
            if (ltoread > HTTP_BUFFER_SIZE) { ltoread = HTTP_BUFFER_SIZE; }

            int lr = f_read(HTTP_buffer, 1, HTTP_BUFFER_SIZE, file);
            if (lr == 0) { break; }

            lread += lr;
            writeall(sock, HTTP_buffer, lr);
        }

        f_close(file);
        iprintf(" File sent to browser\r\n");
        return 0;
    }

    return -1;
}

HtmlPostVariableListCallback gHandleGetMailPost("getmail.html", HandleGetMailPost);
CallBackFunctionPageHandler gHandleEmailGet("email*", HandleMailGet);
