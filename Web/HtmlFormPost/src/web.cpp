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

#include <http.h>
#include <iosys.h>
#include <httppost.h>
#include <string.h>

// Buffers to hold data posted from form
#define MAX_BUF_LEN 1024
char textForm1[MAX_BUF_LEN];
char textForm2[MAX_BUF_LEN];

// Functions called from a web page must be declared extern C
extern "C"
{
    void WebTextForm1(int sock, PCSTR url);
    void WebTextForm2(int sock, PCSTR url);
};

/**
 *  WebTextForm1
 *
 *  Function to display value in web page
 */
void WebTextForm1(int sock, PCSTR url)
{
    writestring(sock, textForm1);
}

/**
 *  WebTextForm2
 *
 *  Function to display value in web page
 */
void WebTextForm2(int sock, PCSTR url)
{
    writestring(sock, textForm2);
}

void form1PostCallBack(int sock, PostEvents event, const char * pName, const char * pValue)
{
    // Received a call back with an event, check for event type
    switch (event)
    {
    case eStartingPost:     // Called at the beginning of the post before any data is sent
        break;

    case eVariable:     // Called once for each variable in the form
        if (strcmp("textForm1", pName) == 0 )
        {
            strncpy(textForm1, pValue, MAX_BUF_LEN-1);
            iprintf("textForm1 set to: \"%s\"\r\n", pValue);
        }
        break;

    //Called back with a file handle if the post had a file
    case eFile:
        break; //No file type here so we do nothing

    // Called back when the post is complete. You should send your response here.
    case eEndOfPost:
        {
            RedirectResponse(sock, "page2.html");
        }
        break;
    } //Switch
}


void form2PostCallBack(int sock, PostEvents event, const char * pName, const char * pValue)
{
    // Received a call back with an event, check for event type
    switch (event)
    {
    case eStartingPost:     // Called at the beginning of the post before any data is sent
        break;

    case eVariable:     // Called once for each variable in the form
        if (strcmp("textForm2", pName) == 0)
        {
            strncpy(textForm2, pValue, MAX_BUF_LEN-1);
            iprintf("textForm2 set to: \"%s\"\r\n", pValue);
        }            
        break;

    //Called back with a file handle if the post had a file
    case eFile:
        break; //No file type here so we do nothing

    // Called back when the post is complete. You should send your response here.
    case eEndOfPost:
        {
            RedirectResponse(sock, "complete.html");
        }
        break;
    } //Switch
}


// Create a global static post handeling object that responds to the specified html page.
// A separate post handler can be created for each form in your application.
HtmlPostVariableListCallback postForm1("form1*", form1PostCallBack);
HtmlPostVariableListCallback postForm2("form2*", form2PostCallBack);


