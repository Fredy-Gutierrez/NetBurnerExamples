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
 * This code module contains the web functions for the send mail
 * example program.
 */

#include <stdlib.h>
#include <system.h>
#include <crypto/ssl_mailto.h>
#include <mailto.h>
#include <dns.h>
#include <httppost.h>
#include <netinterface.h>

 // Email variables
char var_user[80];
char var_pass[80];
char var_svrip[80];
char var_svrport[6];
char STARTTLS[5];
char var_from[80];
char var_to[80];
char var_sub[80];
char var_body[80];
int SendmailResult;
int DNSResult;

// Function Prototypes
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
    void WebShowServerPort(int sock, PCSTR url);
    void WebShowFrom(int sock, PCSTR url);
    void WebShowTo(int sock, PCSTR url);
    void WebShowSubject(int sock, PCSTR url);
    void WebShowBody(int sock, PCSTR url);

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
* @brief Displays the current destination server port form value.
*
* @param sock HTTP Socket
* @param url Calling page
*/
void WebShowServerPort(int sock, PCSTR url)
{
    writestring(sock, var_svrport);
}

/**
* @brief Displays the current from form value.
*
* @param sock HTTP Socket
* @param url Calling page
*/
void WebShowFrom(int sock, PCSTR url)
{
    writestring(sock, var_from);
}

/**
* @brief Displays the current from to value.
*
* @param sock HTTP Socket
* @param url Calling page
*/
void WebShowTo(int sock, PCSTR url)
{
    writestring(sock, var_to);
}

/**
* @brief Displays the current email subject value.
*
* @param sock HTTP Socket
* @param url Calling page
*/
void WebShowSubject(int sock, PCSTR url)
{
    writestring(sock, var_sub);
}

/**
* @brief Displays the current email body value.
*
* @param sock HTTP Socket
* @param url Calling page
*/
void WebShowBody(int sock, PCSTR url)
{
    writestring(sock, var_body);
}

/**
* @brief Displays the results of the request and any associated information with the request.
*
* @param sock HTTP Socket
* @param url Calling page
*/
void LastResult(int sock, PCSTR url)
{
    char buf[80];

    if (SendmailResult)  // Result from SendMailAuth()
    {
        writestring(sock, "<h1>Mail Sent Successfully</h1>\r\n");
    }
    else
    {
        writestring(sock, "<h1>Error - Could Not Send Mail</h1>\r\n");
    }

    writestring(sock, "DETAILS:");
    if (DNSResult == DNS_OK)
    {
        sniprintf(buf, 80, "\r\n<br>DNS lookup of \"%s\" succeeded\r\n", var_svrip);
        writestring(sock, buf);
    }
    else
    {
        sniprintf(buf, 80, "\r\n<br>DNS lookup of \"%s\" FAILED\r\n", var_svrip);
        writestring(sock, buf);
    }
    if (IsMailError())
    {
        writestring(sock, "<pre>");  //preformat to properly handle \r\n in string
        PrintNBError(sock);
        PrintServerLog(sock);
        writestring(sock, "<pre>");  //preformat to properly handle \r\n in string
    }
    writestring(sock, "\r\n<br>To: ");
    writestring(sock, var_to);
    writestring(sock, "\r\n<br>From: ");
    writestring(sock, var_from);
    writestring(sock, "\r\n<br>Subject: ");
    writestring(sock, var_sub);
    writestring(sock, "\r\n<br>Body: ");
    writestring(sock, var_body);
}


/**
* @brief A POST callback for the send mail post request. This callback is called for each field
* of a post form.
*
* @param sock HTTP socket.
* @param event The kind of post event that is currently being handled with this callback.
* @param pName The name of the post element that is currently being handled.
* @param pValue The value of the post element that is currently being handled.
*/
void HandleSendMailPost(int sock, PostEvents event, const char *pName, const char *pValue)
{
    if (event == eStartingPost)
    {
        var_user[0] = 0;
        var_pass[0] = 0;
        var_svrip[0] = 0;
        var_svrport[0] = 0;
        var_from[0] = 0;
        var_to[0] = 0;
        var_sub[0] = 0;
        var_body[0] = 0;
        STARTTLS[0] = 0;
        iprintf("Initiating extraction of post data\r\n");
    }
    else if (event == eVariable)
    {
        if (strcmp(pName, "user") == 0) { snprintf(var_user, 80, pValue); }
        else if (strcmp(pName, "pass") == 0) { snprintf(var_pass, 80, pValue); }
        else if (strcmp(pName, "svrip") == 0) { snprintf(var_svrip, 80, pValue); }
        else if (strcmp(pName, "svrport") == 0) { snprintf(var_svrport, 6, pValue); }
        else if (strcmp(pName, "STARTTLS") == 0) { snprintf(STARTTLS, 5, pValue); }
        else if (strcmp(pName, "from") == 0) { snprintf(var_from, 80, pValue); }
        else if (strcmp(pName, "to") == 0) { snprintf(var_to, 80, pValue); }
        else if (strcmp(pName, "sub") == 0) { snprintf(var_sub, 80, pValue); }
        else if (strcmp(pName, "body") == 0) { snprintf(var_body, 80, pValue); }
        else { iprintf("Unknown field of \"%s\" with value \"%s\"", pName, pValue); }
    }
    else if (event == eEndOfPost)
    {
        iprintf("End of post data extraction\r\n");
        IPADDR SMTP_server;
        DNSResult = GetHostByName(var_svrip, &SMTP_server, IPADDR::NullIP(), TICKS_PER_SECOND * 20);
        if (DNSResult == DNS_OK)
        {
            int svrPort = strtol(var_svrport, NULL, 0);
            if (svrPort == 0) { svrPort = -1; }
            iprintf("Connecting to server: %s (%I) on port %ld\n", var_svrip, SMTP_server, svrPort);
            if (var_pass[0] == '\0')
            {
                iprintf("Error: no password was entered\r\n");
            }
            else
                SendmailResult = SSL_SendMail(SMTP_server,
                    var_user,
                    var_pass,
                    var_from,
                    var_to,
                    var_sub,
                    var_body,
                    (BOOL)STARTTLS[0],
                    svrPort,
                    var_svrip);
        }
        else
        {
            iprintf("Error - DNS failed for %s\r\n", var_svrip);
        }

        OSTimeDly(TICKS_PER_SECOND * 2);

        RedirectResponse(sock, "sent.html");
    }
}

HtmlPostVariableListCallback gHandleSendMailPost("sendmail.html", HandleSendMailPost);

