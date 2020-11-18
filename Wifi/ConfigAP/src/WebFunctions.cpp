#include <predef.h>
#include <basictypes.h>
#include <stdio.h>
#include <nettypes.h>
#include <constants.h>
#include <ip.h>
#include <utils.h>
#include <http.h>
#include <tcp.h>
#include <string.h>
#include <iosys.h>
#include <fdprintf.h>
#include <netinterface.h>
#include <httppost.h>
#include <wifi/wifi.h>
#include "WebFunctions.h"

bool    WifiInitComplete;  // Flag to know when to display data on web page
char    tbuf[1024];        // global buffer
extern int gWifiInterface;        // handle of wireless interface
extern int gEthernetInterface;    // handle of Ethernet interface

extern "C"
{
   void WebDisplayInterfaces( int sock, PCSTR url );
   void WebReqSource( int sock, PCSTR url );
   void WebDisplayScan( int sock, PCSTR url );
   void InitHtmlHandlers();
}

// NULL if the wifi interface has not been initialized
extern NB::Wifi *pNBWifiObject;

// Buffers to hold data posted from form
#define MAX_BUFLEN 80
char passwordBuf[MAX_BUFLEN];
char ssidBuf[MAX_BUFLEN];


/*-------------------------------------------------------------------
 * Display all network interface information
 *------------------------------------------------------------------*/
void WebDisplayInterfaces( int sock, PCSTR url )
{
    int32_t ifNum = GetFirstInterface();    // Get first interface identifier
    while (ifNum > 0)
    {
        InterfaceBlock *pIfBlock = GetInterfaceBlock(ifNum);     // Get interface data
        fdprintf(sock, "Name: %s, IP Address: %hI", pIfBlock->pName, pIfBlock->ip4.cur_addr.i4);

        if ( (strstr(pIfBlock->pName, "Wifi") != NULL ) && (pNBWifiObject != NULL) )
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

/*-------------------------------------------------------------------
 * Display Wifi scan
 *------------------------------------------------------------------*/
void WebDisplayScan( int sock, PCSTR url )
{
    nbWifiScanResult *pScanResult;      // nbWifiScanResult structure is in wifiDriver.h

    iprintf("Starting scan.....");
    pScanResult = WifiInitScan_SPI();
    iprintf("complete\r\n");

    while ( pScanResult != NULL )
    {
        char buf[80];
        snShowMac(buf, 80, &pScanResult->bssid);
        if ( pScanResult->ssid[0] ) {
            fdprintf(sock, "BSSID: %s, SSID: <a href=\"INDEX.HTML?C,%s\">%s</a>\r\n", buf, pScanResult->ssid, pScanResult->ssid );
        }
        else {
            fdprintf(sock, "BSSID: %s, SSID: \r\n", buf, pScanResult);
        }
        pScanResult = pScanResult->next;
    }
}


/*-------------------------------------------------------------------
 Display the source of the HTTP request
 ------------------------------------------------------------------*/
void WebReqSource( int sock, PCSTR url )
{
    // Get IP address of local interface that received the request
   IPADDR ifIp = GetSocketLocalAddr( sock );
   fdprintf(sock, "%hI<br>\r\n", ifIp.Extract4() );
}


/*-------------------------------------------------------------------
 *
 *------------------------------------------------------------------*/
void connectToSsid(char *ssid, char *password)
{
    // If the interface already exists, check for a connection
    if (pNBWifiObject != NULL)
    {
        if (pNBWifiObject->Connected())
        {
            iprintf("Disconnecting current connection.....");
            pNBWifiObject->Disconnect();
            OSTimeDly(TICKS_PER_SECOND * 2);
            if (! pNBWifiObject->Connected())
                iprintf("Success\r\n");
            else
                iprintf("Failed\r\n");
        }
    }

    int ifnumWifi = InitWifi_SPI(ssid, password); // Init if necessary, then connect
    if (ifnumWifi > 0)
    {
        pNBWifiObject = NB::Wifi::GetDriverByInterfaceNumber( ifnumWifi );
    }
    else
    {
        iprintf("Failed to initialize wifi interface\r\n");
    }

    if (pNBWifiObject != NULL)
    {
        if (pNBWifiObject->Connected())
            iprintf("Connected with interface number: %d\r\n", ifnumWifi);
        else
            iprintf("Failed to connect\r\n");
    }
}


void processPostVariables( const char *pName, const char *pValue )
{
    iprintf("Processing: %s\r\n", pName);

    if( strcmp(pName, "password") == 0 )
    {
        iprintf("Password set to %s\r\n", pValue);
        iprintf("Connecting to SSID: \"%s\", Password: \"%s\"\r\n", ssidBuf, passwordBuf);
        connectToSsid(ssidBuf, passwordBuf);
    }
    else if( strcmp( pName, "store") == 0 )
    {
        if (strcasecmp(pValue, "yes") == 0) 
        {
            iprintf("Storing SSID: \"%s\", Password: \"%s\" to ConfigRecord\r\n", ssidBuf, passwordBuf);
            pNBWifiObject->StoreSSIDPWToConfig(ssidBuf, passwordBuf);
        }
    }
    else
    {
        iprintf("Error processing %s\r\n", pName);
    }
}


void postPasswordCallback(int sock, PostEvents event, const char * pName, const char * pValue)
{
    // Received a call back with an event, check for event type
    switch (event)
    {
    case eStartingPost:     // Called at the beginning of the post before any data is sent
        break;

    case eVariable:     // Called once for each variable in the form
        processPostVariables(pName, pValue);
        break;

    //Called back with a file handle if the post had a file
    case eFile:
        break; //No file type here so we do nothing

    // Called back when the post is complete. You should send your response here.
    case eEndOfPost:
        {
            RedirectResponse(sock, "INDEX.HTML");  // Our response is to redirect to the index page
        }
        break;

    } 
}


// Create a global static post handeling object that responds to the specified html page.
// A separate post handler can be created for each form in your application.
HtmlPostVariableListCallback postPassword("password.html", postPasswordCallback);



/*
 * Callback function to process the HTTP GET for index.html  
 */
int indexGetCallback( int sock, HTTP_Request &httpRequestInfo )
{
    iprintf( "HTTP request: %s\r\n", httpRequestInfo.pURL );
    
    uint32_t urlLen = strlen(httpRequestInfo.pURL);
    uint32_t i = 0;
    while (i < urlLen)
    {
        if (httpRequestInfo.pURL[i++] == '?')
            break;
    }

    if (httpRequestInfo.pURL[i] == 'C')
    {
        i += 2;     // index past "C,"
        decodeURI( &httpRequestInfo.pURL[i] );
        iprintf("Parsed SSID: %s\r\n", &httpRequestInfo.pURL[i]);
        strncpy(ssidBuf, &httpRequestInfo.pURL[i], MAX_BUFLEN);
        ssidBuf[MAX_BUFLEN - 1] = '\0';
        RedirectResponse( sock, "password.html" );
    }
    
    return 1;   // Notify the system GET handler we handled the request
}


// Create a HTTP GET handeling object
CallBackFunctionPageHandler hIndex( "index.html",           // Web page to intercept 
                                     indexGetCallback,      // Pointer to callback function 
                                     tGet,                  // Type of request, GET 
                                     0,                     // Password level, none 
                                     true );                // Take responsibility for entire response to setcookie.html
                                                            //   instead of web server sending a html file
