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
 * @brief This code module contains the web functions for the send mail
 * example program.
 */

#include <stdlib.h>
#include <startnet.h>
#include <mailto.h>
#include <dns.h>
#include <httppost.h>
#include <crypto/ssl_mailto.h>

#include "FileSystemUtils.h"

 // Email variables
#define USER_VAR_SIZE     80
#define PASSWORD_VAR_SIZE 80
#define SERVER_VAR_SIZE   80
#define SERVER_PORT_SIZE  6
#define FROM_VAR_SIZE     80
#define TO_VAR_SIZE       80
#define SUBJECT_VAR_SIZE 256
#define BODY_VAR_SIZE  16000
#define ATTACHMENT_VAR_SIZE 16000

#define READ_BUFSIZE 1000

char var_user[USER_VAR_SIZE];
char var_pass[PASSWORD_VAR_SIZE];
char var_svrip[SERVER_VAR_SIZE];
char var_svrport[SERVER_PORT_SIZE];
char var_from[FROM_VAR_SIZE];
char var_to[TO_VAR_SIZE];
char var_sub[SUBJECT_VAR_SIZE];
char var_body[BODY_VAR_SIZE];
char var_create_text_attach[ATTACHMENT_VAR_SIZE];
char var_attach_sd_filename[ATTACHMENT_VAR_SIZE];
int SendmailResult = 0;
int DNSResult = 0;

//----- Function Prototypes -----
extern "C"
{
    void WebShowUserValue(int sock, PCSTR url);
    void WebShowUserPass(int sock, PCSTR url);
    void WebShowUserValue(int sock, PCSTR url);
    void WebShowServer(int sock, PCSTR url);
    void WebShowServerPort(int sock, PCSTR url);
    void WebShowFrom(int sock, PCSTR url);
    void WebShowTo(int sock, PCSTR url);
    void WebShowSubject(int sock, PCSTR url);
    void WebShowBody(int sock, PCSTR url);
    void WebShowCreateTextAttachment(int sock, PCSTR url);
    void WebShowAttachSDFile(int sock, PCSTR url);

    void LastResult(int sock, PCSTR url);
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
* @brief Displays the text attachment to create.
*
* @param sock HTTP Socket
* @param url Calling page
*/
void WebShowCreateTextAttachment(int sock, PCSTR url)
{
    writestring(sock, var_create_text_attach);
}

/**
* @brief Displays attached file from the SD card.
*
* @param sock HTTP Socket
* @param url Calling page
*/
void WebShowAttachSDFile(int sock, PCSTR url)
{
    writestring(sock, var_attach_sd_filename);
}

/**
* @brief Displays the results of the request and any associated information with the request.
*
* @param sock HTTP Socket
* @param url Calling page
*/
void LastResult(int sock, PCSTR url)
{
    static char buf[1024];

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
        sniprintf(buf, 1024, "\r\n<br>DNS lookup of \"%s\" succeeded\r\n", var_svrip);
        writestring(sock, buf);
    }
    else
    {
        sniprintf(buf, 1024, "\r\n<br>DNS lookup of \"%s\" FAILED\r\n", var_svrip);
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
        var_user[0] = '\0';
        var_pass[0] = '\0';
        var_svrip[0] = '\0';
        var_svrport[0] = '\0';
        var_from[0] = '\0';
        var_to[0] = '\0';
        var_sub[0] = '\0';
        var_body[0] = '\0';
        var_create_text_attach[0] = '\0';
        var_attach_sd_filename[0] = '\0';

        iprintf("\r\n===== Initiating extraction of post data =====\r\n");
    }
    else if (event == eVariable)
    {
        if (strcmp(pName, "user") == 0) { snprintf(var_user, USER_VAR_SIZE, pValue); }
        else if (strcmp(pName, "pass") == 0) { snprintf(var_pass, PASSWORD_VAR_SIZE, pValue); }
        else if (strcmp(pName, "svrip") == 0) { snprintf(var_svrip, SERVER_VAR_SIZE, pValue); }
        else if (strcmp(pName, "svrport") == 0) { snprintf(var_svrport, SERVER_PORT_SIZE, pValue); }
        else if (strcmp(pName, "from") == 0) { snprintf(var_from, FROM_VAR_SIZE, pValue); }
        else if (strcmp(pName, "to") == 0) { snprintf(var_to, TO_VAR_SIZE, pValue); }
        else if (strcmp(pName, "sub") == 0) { snprintf(var_sub, SUBJECT_VAR_SIZE, pValue); }
        else if (strcmp(pName, "body123") == 0) { snprintf(var_body, BODY_VAR_SIZE, pValue); }
        else if (strcmp(pName, "create_text_attach") == 0) { snprintf(var_create_text_attach, ATTACHMENT_VAR_SIZE, pValue); }
        else if (strcmp(pName, "attach_sd_filename") == 0) { snprintf(var_attach_sd_filename, ATTACHMENT_VAR_SIZE, pValue); }
        else { iprintf("Unknown field of \"%s\" with value \"%s\"", pName, pValue); }
    }
    else if (event == eEndOfPost)
    {
        iprintf("End of post data extraction\r\n");

        IPADDR SMTP_server;
        DNSResult = GetHostByName(var_svrip, &SMTP_server, IPADDR::NullIP(), TICKS_PER_SECOND * 20);
        if (DNSResult != DNS_OK)
        {
            iprintf("Error - DNS failed for %s\r\n", var_svrip);
        }

        int svrPort = strtol(var_svrport, NULL, 0);
        if (svrPort == 0) { svrPort = -1; }
        iprintf("Connecting to server: %s (%I) on port %ld\n", var_svrip, SMTP_server, svrPort);
        int fd = 0;

        // If web field Create Attachment From Text is empty, just send the email body
        if ( (var_create_text_attach[0] == '\0') && (var_attach_sd_filename[0] == '\0'))
        {
            iprintf("Sending email with authentication, but no attachment\r\n");
            SendmailResult = SSL_SendMail(SMTP_server, var_user, var_pass, var_from, var_to, var_sub, var_body, false, svrPort, var_svrip);
        }
        else  // If web field Create Attachment From Text has data, then create a .txt file and attach
        {
            // Start SMTP session with authorization
            SendmailResult = SSL_SendMailStartMIME(SMTP_server, var_user, var_pass,
                var_from, var_to, var_sub, false, svrPort, var_svrip);
            fd = SendmailResult;

            // Send Body of email
            SendMailAuthAddMIME(fd, CONTENT_TYPE_PLAIN_TEXT, var_body, NULL);

            // The SD card attachment file name field is empty
            if (var_create_text_attach[0] != '\0')
            {
                iprintf("Sending email with authentication, creating TextFile.txt from \"%s\"\r\n", var_create_text_attach);


                // Send Attachment. For this example we will just name it TextFile.txt
                SendMailAuthAddMIME(fd, CONTENT_TYPE_PLAIN_TEXT_ATTACH, var_create_text_attach, "TextFile.txt");
            }
            if (var_attach_sd_filename[0] != '\0')  // Attach specified file from SD card and send
            {
                iprintf("Sending email with authentication, and SD attachment \"%s\"\r\n", var_attach_sd_filename);

                F_FILE *fp = f_open(var_attach_sd_filename, "r");
                if (!fp)
                {
                    iprintf("Error - could not open file: \"%s\"\r\n", var_attach_sd_filename);
                }
                else
                {
                    // Send Attachment
                    SendMailAuthAddMIME(fd, CONTENT_TYPE_BINARY_ATTACH, NULL, var_attach_sd_filename);

                    int total_byt = 0;
                    while (!f_eof(fp))
                    {
                        /* Read bytes from the current position in the target file. File has
                         *  to be opened with “r”, "r+", "w+" or "a+".
                         *  f_read ( const void *buffer,  // pointer to buffer to store data
                         *           long size,           // size of items to be read
                         *           long size size_st,   // number of items to be read
                         *           F_FILE )             // handle of target file
                         *  Returns the number of items read.
                         */
                        char read_buf[READ_BUFSIZE];
                        int bytes_read = f_read(read_buf, 1, READ_BUFSIZE - 1, fp);
                        read_buf[bytes_read] = '\0';  // terminate string
                        total_byt += bytes_read;

                        writeall(fd, (const char *)read_buf, bytes_read);
                        OSTimeDly(2);
                    }

                    int rv = f_close(fp);  // Close a previously opened file of type F_FILE
                    if (rv != F_NO_ERROR)
                    {
                        iprintf("*** Error in f_close(): ");
                        DisplayEffsErrorCode(rv);
                    }
                }
            }

            // End SMTP session
            SendMailAuthEndMIME(fd, var_user);
        }
        RedirectResponse(sock, "sent.html");
    }
}

HtmlPostVariableListCallback gHandleSendMailPost("sendmail.html", HandleSendMailPost);
