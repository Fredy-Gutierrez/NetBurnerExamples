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

#include <fdprintf.h>
#include <tcp.h>
#include <http.h>
#include <httppost.h>
#include <wifi/wifi.h>

bool WifiInitComplete = false;   // Flag to know when to display data on web page
char tbuf[1024];                 // global buffer
extern int gWifiInterface;       // handle of wireless interface
extern int gEthernetInterface;   // handle of Ethernet interface

extern "C"
{
    void WebDisplayInterfaces(int sock, PCSTR url);
    void WebReqSource(int sock, PCSTR url);
    void WebDisplayScan(int sock, PCSTR url);
    void InitHtmlHandlers();
}

extern const char *default_page;

// nullptr if the wifi interface has not been initialized
extern NB::Wifi *pNBWifiObject;

// Buffers to hold data posted from form
#define MAX_BUFLEN 80
char passwordBuf[MAX_BUFLEN];
char ssidBuf[MAX_BUFLEN];

/**
 * @brief Display all network interface information
 */
void WebDisplayInterfaces(int sock, PCSTR url)
{
    int32_t ifNum = GetFirstInterface();   // Get first interface identifier
    while (ifNum > 0)
    {
        InterfaceBlock *pIfBlock = GetInterfaceBlock(ifNum);   // Get interface data
        fdprintf(sock, "Name: %s, IP Address: %hI", pIfBlock->pName, pIfBlock->ip4.cur_addr.i4);


        // if the interface block obtained has the same name as the wifi driver object name, print the connection status
        if ((pNBWifiObject != nullptr) && (strstr(pIfBlock->pName, pNBWifiObject->pName) != nullptr))
        {
            if (pNBWifiObject->Connected())
            {
                char ssidBuf[80];
                pNBWifiObject->GetCurSSID(ssidBuf, 80);
                fdprintf(sock, ", SSID: %s", ssidBuf);
            }
            else
            {
                fdprintf(sock, ", Not Connected");
            }
        }
        fdprintf(sock, "<br>\r\n");

        ifNum = GetNextInterface(ifNum);
    }
}

/**
 * @brief Display Wifi scan
 */
void WebDisplayScan(int sock, PCSTR url)
{
    nbWifiScanResult *pScanResult;   // nbWifiScanResult structure is in wifiDriver.h

    iprintf("Starting scan.....");
    pScanResult = WifiInitScan_SPI();
    iprintf("complete\r\n");

    while (pScanResult != nullptr)
    {
        char buf[80];
        snShowMac(buf, 80, &pScanResult->bssid);
        fdprintf(sock, "BSSID: %s, SSID: <a href=\"INDEX.HTML?C,%s\">%s</a>\r\n", buf, pScanResult->ssid, pScanResult->ssid);
        pScanResult = pScanResult->next;
    }
}

/**
 * @brief Display the source of the HTTP request
 */
void WebReqSource(int sock, PCSTR url)
{
    // Get IP address of local interface that received the request
    IPADDR ifIp = GetSocketLocalAddr(sock);
    fdprintf(sock, "%hI<br>\r\n", ifIp.Extract4());
}

/**
 * @brief Tries to connect to the given `ssid` with the provided `password`
 */
void connectToSsid(char *ssid, char *password)
{
    // If the interface already exists, check for a connection
    if (pNBWifiObject != nullptr)
    {
        if (pNBWifiObject->Connected())
        {
            iprintf("Disconnecting current connection.....");
            pNBWifiObject->Disconnect();
            OSTimeDly(TICKS_PER_SECOND * 2);
            if (!pNBWifiObject->Connected()) { iprintf("Success\r\n"); }
            else
            {
                iprintf("Failed\r\n");
            }
        }
    }

    int ifnumWifi = InitWifi_SPI(ssid, password);   // Init if necessary, then connect
    if (ifnumWifi > 0) { pNBWifiObject = NB::Wifi::GetDriverByInterfaceNumber(ifnumWifi); }
    else
    {
        iprintf("Failed to initialize wifi interface\r\n");
    }

    if (pNBWifiObject != nullptr)
    {
        if (pNBWifiObject->Connected()) { iprintf("Connected with interface number: %d\r\n", ifnumWifi); }
        else
        {
            iprintf("Failed to connect\r\n");
        }
    }
}

/**
 * @brief A callback function that determines if the URL has a match or not.
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
    if ((pr.pURL[0] != '\0') && (httpstricmp(pr.pURL, "/INDEX.HTML") == 1))
    {
        uint32_t urlLen = strlen(pr.pURL);
        uint32_t i = 0;
        while (i < urlLen)
        {
            if (pr.pURL[i++] == '?') { break; }
        }

        if (pr.pURL[i] == 'C') { return true; }
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
    if ((pr.pURL[0] != '\0') && (httpstricmp(pr.pURL, "/INDEX.HTML") == 1))
    {
        uint32_t urlLen = strlen(pr.pURL);
        uint32_t i = 0;
        while (i < urlLen)
        {
            if (pr.pURL[i++] == '?') { break; }
        }

        if (pr.pURL[i] == 'C')
        {
            i += 2;   // index past "C,"
            decodeURI(&pr.pURL[i]);
            iprintf("Parsed SSID: %s\r\n", &pr.pURL[i]);
            strncpy(ssidBuf, &pr.pURL[i], MAX_BUFLEN);
            ssidBuf[MAX_BUFLEN - 1] = '\0';
            RedirectResponse(sock, "password.html");
            return 1;
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

/**
 * @brief Handles a POST request
 *
 * @param sock HTTP Page
 * @param url URL of calling page
 * @param pData Data from the web page which should contain the cert data.
 * @param allDataPtr Not used
 *
 * @retval 0 Success
 * @retval !0 Errors
 *
 * <b>Notes:</b> Certificate and keys used for secure web site.
 */
void HandlePasswordPost(int sock, PostEvents event, const char *pName, const char *pValue)
{
    if (event == eVariable)
    {
        if (strcmp(pName, "password") == 0) { snprintf(passwordBuf, MAX_BUFLEN, pValue); }
        else
        {
            iprintf("Unknown field of \"%s\" with value \"%s\"", pName, pValue);
        }
    }
    else if (event == eEndOfPost)
    {
        connectToSsid(ssidBuf, passwordBuf);
        RedirectResponse(sock, "index.html");
    }
}

// GET Handler Objects
CallBackFunctionPageHandler gHandleGet("*", HandleGet, HandleMatch, tGet, 0, true);

// POST Handler Objects
HtmlPostVariableListCallback gHandlePasswordPost("password.html", HandlePasswordPost);
