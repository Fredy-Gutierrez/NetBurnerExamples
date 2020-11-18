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

#include <predef.h>
#include <htmlfiles.h>
#include <string.h>
#include <http.h>
#include <iosys.h>

extern "C"
{
    int ShowCookie(int sock, const char *url);
}


/**
 *  ShowCookie
 *
 *  This function is used in this example to display a cookie. It is called 
 *  when the web browser requests the page showcookie.html. It parses the HTTP
 *  header sent to the NetBurner web server to locate the header keyword 
 *  "Cookie:". Once found, it reads the subsequent cookie characters.
 */
int ShowCookie(int sock, const char *url)
{
    iprintf("URL: %s\r\n", url);            // Display the url to debug port
    
    const char *pCookie = url;              // Create a pointer to the URL and cookie data
    pCookie += strlen(url);                 // Move pointer past the URL portion of the requse 
    pCookie++;                              // Move past the URL null to get to the HTTP receive buffer
    pCookie = strstr(pCookie, "Cookie:");   // Search the resultant string for the text: "Cookie: "
    if (pCookie != nullptr)                 // Check to verify the substring was found
    {
        pCookie += 7;                       // Move past "Cookie: "
        int i = 0;
        while (pCookie[i] >= ' ')           // Increment until a space is detected
            i++;
        
        // At this point the cookie is from p to p+i. Write the cookie value to the active socket.
        write(sock, pCookie, i);
    }

    return 0;
}


/*
 * Callback function to process the HTTP GET request to set a cookie.  
 * The function will send a header to set a cookie, then send the 
 * sendcookie.html page, since this function is taking responsibility
 * for responding to the GET request, instead of the system handling
 * it by sending a html file.
 *  
 * Any page associated with the "site" (in this case the NetBurner device 
 * IP address) will receive the cookie.
 */
int setCookieGetReqCallback( int sock, HTTP_Request &httpRequestInfo )
{
    iprintf( "HTTP request: %s\r\n", httpRequestInfo.pURL );
    
    // Set the cookie value to "MyCookie ". Note that you need a trailing space.
    SendHTMLHeaderWCookie(sock, "MyCookie ");

    // Since we are handling the GET request, we need to send the web page
    SendFileFragment("setcookie.html", sock);
    
    return 1;   // Notify the system GET handler we handled the request
}


// Create a HTTP GET handeling object
CallBackFunctionPageHandler hIndex( "setcookie.html",           // Web page to intercept 
                                     setCookieGetReqCallback,   // Pointer to callback function 
                                     tGet,                      // Type of request, GET 
                                     0,                         // Password level, none 
                                     true );                    // Take responsibility for entire response to setcookie.html
                                                                //   instead of web server sending a html file


