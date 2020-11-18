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

#include <crypto/ssl.h>
#include <init.h>
#include <netinterface.h>
#include <startnet.h>

// Function declarations for C compatibility
extern "C"
{
    void HTTPS_Ref(int sock, const char *url);
    void SSL_Image(int sock, const char *url);
    void DoCounter(int sock, PCSTR url);
}

const char *AppName = "SSL Web Demo";

/*
 * A Trivial function that shows how many times a page has been reloaded.
 *
 * param sock 	HTTP Socket
 * param url 	Calling page
 */
void DoCounter(int sock, PCSTR url)
{
    static uint32_t dw;
    char buffer[40];

    if (IsSSLfd(sock))
    {
        sniprintf(buffer, 40, "%ld(SSL)", dw++);
    }
    else
    {
        sniprintf(buffer, 40, "%ld(HTTP)", dw++);
    }

    // Write the value to the HTML page...
    writestring(sock, buffer);
}

/*
 * Writes the image location to a given socket.
 *
 * param sock 	HTTP Socket
 * param url 	Calling page
 */
void SSL_Image(int sock, const char *url)
{
    if (IsSSLfd(sock))
    {
        writestring(sock, "images/SSL-Good.gif");   // write index page data
    }
    else
    {
        writestring(sock, "images/SSL-Bad.gif");   // write index page data
    }
}

/*
 * This function is called in `index.html` and provides the hyperlink to the SSL
 * page. The idea is to provide a hyperlink of the following format:
 * https://x.x.x.x/index.htm, where the x's are replaced with the ip address
 * of the NetBurner device.
 *
 * param sock 	HTTP Socket
 * param url 	Calling page
 */
void HTTPS_Ref(int sock, const char *url)
{
    char buf[80];
    InterfaceBlock *pifb = GetInterfaceBlock();
    if (pifb != nullptr)
    {
        writestring(sock, "\"https://");   // start of the url
        sniprintf(buf, 80, "%hI", pifb->ip4.cur_addr.i4);
        writestring(sock, buf);               // write link data
        writestring(sock, "/index.html\"");   // write index page data
    }
}

/*
 * Function to check the access level for each page. This overrides a
 * weak reference from the system that returns `HTTP_OK_TO_SERVE` for every page
 * and allows us to check for things like an SSL connection.
 *
 * Access levels are set either in the html page of the resource itself, in a
 * .nbaccess file in the directory, or in the case of callback functions, as a
 * parameter.
 *
 * param sock 			The HTTP socket.
 * param access_level 	The access level of the requested resource.
 * param Req 			The HTTP request initiating the callback.
 *
 * This function will override the system function of the same name that simply
 * checks for just a password.
 *
 */
HTTP_ACCESS CheckHttpAccess(int sock, int access_level, HTTP_Request &Req)
{
    if (access_level == 0)
    {
        return HTTP_OK_TO_SERVE;
    }
    else if (access_level == 1)
    {
        if (IsSSLfd(sock))
        {
            return HTTP_OK_TO_SERVE;
        }
        else
        {
            return HTTP_FORBIDEN;
        }
    }
}

/*
 *  Main entry point for examples
 */
void UserMain(void *pd)
{
    init();                                       // Initialize network stack
    StartHttps();                                 // Start web server, default port 80
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

    InterfaceBlock *pifb = GetInterfaceBlock();
    if (pifb != nullptr)
    {
        iprintf("DHCP assigned the IP address of : %hI\r\n", pifb->ip4.cur_addr.i4);
    }

    // loop forever, since all action happens on HTTP requests
    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND);
    }
}
