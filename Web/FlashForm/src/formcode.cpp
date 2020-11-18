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
#include <system.h>
#include <string.h>
#include <httppost.h>

/**
 *  The FlashForm example demonstrates three different capabilities:
 *  1. Dynamic HTML
 *  2. HTML Form processing
 *  3. User Flash parameter storage.
 *
 *  DYNAMIC HTML
 *  -------------
 *  Look in the file html/INDEX.HTM under this project. Inside that
 *  file you will notice two HTML comments:
 *      <!--FUNCTIONCALL DoMessageName -->
 *      <!--FUNCTIONCALL DoMessageBody -->
 *
 *  When this HTML page is requested by a web browser, the NetBurner
 *  web server will begin streaming out the data. When a FUNCTIONCALL tag
 *  is encountered, the web server will execute the specified function.
 *  The function is passed a file descriptor to the socket and can then
 *  send whatever dynamic content to the web browser. In this example the
 *  functions are:
 *
 *     void DoMessageName(int sock, PCSTR url);
 *     void DoMessageBody(int sock, PCSTR url);
 *
 *  These functions use the following I/O calls to send the dynamic
 *  data:
 *
 *     int  write(int fd, const char * buf, int nbytes); // Write any data
 *     void writestring(int fd, const char * str);       // Write a null terminated string
 *     void writesafestring(int fd, const char * str);   // Write a string escaping any special HTML chars
 *
 *  That is all there is to it!
 *
 *  FORM PROCESSING
 *  ---------------
 *  At the bottom of this page we
 * have the following global static variable
 * HtmlPostVariableListCallback poster("formpost.html",PostCallBack);
 *
 *
 * This tells the system that any post aimed at formpost.html should send the results
 * to the PostCallBack function.
 *
 * The post call back gets called back at the start of the post, again for each name,
 * value pair in the post and finaly at the end of the post.
 *
 * For each valiable we check to see if its one we care about and then store its value in the
 * aproriate place.
 *
 *
 *  FLASH PARAMETER STORAGE IN USERPARAMERS
 *  ---------------------------------------
 *  The system stores and retrieves user flash parameters using two simple functions.
 *  The following function returns a pointer to the User Parameter area.
 *  THIS POINTER IS READONLY!
 *
 *     void * GetUserParameters();  // Read-only pointer to stored values
 *
 *  The following function returns non-zero if the User Parameter region
 *  was programmed successfully:
 *
 *     int SaveUserParameters( void * pCopyFrom, int len );  // Write values to flash
 *
 *  All of the functions exposed to the HTML subsystem must be declared as extern C
 *  to prevent C++ name mangling. The system code is all C++, but we want to
 *  retain functionality for the straight C Traditionalists.
 */

extern "C"
{
    void DoMessageName(int sock, PCSTR url);
    void DoMessageBody(int sock, PCSTR url);
}

// Structure for storing and retrieving from user Flash
#define SIZEOF_NAME (40)
#define SIZEOF_MSG (128)
#define VERIFY_VALUE (0x48666050)   // A random value picked from the phone book....

struct MyOwnDataStore
{
    uint32_t verify_key = 0;
    char name[SIZEOF_NAME];
    char msg[SIZEOF_MSG];
};

/**
 *  DoMessageName
 *
 *  FUNCTIONCALL function to display the "name" structure member on the web page.
 */
void DoMessageName(int sock, PCSTR url)
{
    // Read the stored data
    MyOwnDataStore *pData = (MyOwnDataStore *)GetUserParameters();

    // Verify it has the right key value. In a "Real" application, a
    // checksum would be more robust.
    if (pData->verify_key == VERIFY_VALUE)
    {
        // If the data is ok, write it out as "safe" escaped HTML
        writesafestring(sock, pData->name);
    }
    else
    {
        writestring(sock, "?");
    }
}

/**
 *  DoMessageBody
 *
 *  FUNCTIONCALL function to display the "msg" structure member on the web page.
 */
void DoMessageBody(int sock, PCSTR url)
{
    // Read the stored data
    MyOwnDataStore *pData = (MyOwnDataStore *)GetUserParameters();

    // Verify it has the right key value.
    // In a "Real" application checksum would be more robust
    if (pData->verify_key == VERIFY_VALUE)
    {
        // If the data is ok, write it out as "safe" escaped HTML
        writesafestring(sock, pData->msg);
    }
    else
    {
        writestring(sock, "No Message Stored");
    }
}




void PostCallBack(int sock, PostEvents event, const char * pName, const char * pValue)
{
    // It is very important that this variable is static, since this function will
    // be called multiple times and needs to access the data each of those times.
    static MyOwnDataStore TempStore; //Store the data we want to save.

    // Received a call back with an event, check for event type
    switch (event)    {
    case eStartingPost:     // Called at the beginning of the post before any data is sent
        // Initialize data storage TempStore.
        TempStore.verify_key = VERIFY_VALUE;
        TempStore.msg[0]     = 0;
        TempStore.name[0]    = 0;
        break;

    case eVariable:     // Called once for each variable in the form
        if (strcmp(pName, "name") == 0)
        {
            strncpy(TempStore.name, pValue, SIZEOF_NAME);
        }
        else if (strcmp(pName, "comment") == 0)
        {
            strncpy(TempStore.msg, pValue, SIZEOF_NAME);
        }
        break;

    //Called back with a file handle if the post had a file
    case eFile:
        break; //No file type here so we do nothing

    // Called back when the post is complete. You should send your response here.
    case eEndOfPost:
        {
            // Ok the post is done so save the data
            if (SaveUserParameters(&TempStore, sizeof(TempStore)) != 0)
            {
                iprintf("Parameters saved in User Parameter area.\r\n");
            }
            else
            {
                iprintf("Programming failed \r\n");
            }

            // Our response is to redirect to the index page.
            RedirectResponse(sock, "INDEX.HTML");
        }
        break;
    } //Switch
}


// Create a global static post handeling object that only responds to the specified html page.
// A separate post handler can be created for each form in your application.
HtmlPostVariableListCallback poster("formpost.html",PostCallBack);


