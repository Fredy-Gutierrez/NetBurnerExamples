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
 * File:         webfuncs.cpp
 * Description:  Dynamic web server content functions.
 */

#include <nbrtos.h>
#include <iosys.h>
#include <http.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <tcp.h>
#include <udp.h>
#include <pins.h>
#include <utils.h>
#include <buffers.h>
#include <json_lexer.h>
#include <websockets.h>
#include <nbstring.h>

#include "webfuncs.h"

/**
 * Functions linked to web FUNCTIONCALL tags.
 */
extern "C"
{
    void WebLeds(int sock, PCSTR url);
    void DoSwitches(int sock, PCSTR url);
    void WebTickTacToe(int sock, PCSTR url);
    void DisplayFirmwareVersion(int sock, PCSTR url);
}

extern http_wshandler *TheWSHandler = nullptr;

volatile int wsFdList[WS_MAX_SOCKS] = {-1, -1, -1, -1};

char buffer[255];

// These are buffers that we use to send JSON data to clients
const int ReportBufSize = 512;
char ReportBuffer[ReportBufSize];

// WebSocket File Descriptor
int ws_fd = 0;

// Show web debug statements
bool gWebDebug = false;
void ToggleWebDebug()
{
    gWebDebug = !gWebDebug;
    if (gWebDebug) { iprintf("Web debug statements enabled.\r\n"); }
    else
    {
        iprintf("Web debug statements disabled.\r\n");
    }
}

/**
 * @brief This function displays a set of LED indicators on a web page that
 * can allow a user to turn the LEDs on and off through a web browser.
 * The state of the LED's is communicated from one HTML page to
 * another through the number appended on the base URL.
 */
void WebLeds(int sock, PCSTR url)
{
    int32_t urlEncodeOffset = 9;

    // The URL will have the format: "LED.HTM?number", where the value
    // of "number" will store the LED information. The following code
    // will test for the '?' character. If one exists, then the
    // number following the '?' is assumed to contain a value
    // indicating which LEDs are lit. If no '?' is in the URL, then
    // the LEDs are initialized to a known state.
    int v;
    if ((*(url + urlEncodeOffset)) == '?') { v = atoi((char *)(url + urlEncodeOffset + 1)); }
    else
    {
        v = 0xAA;
    }

    putleds((uint8_t)v & 0xFF);

    // Now step through each LED position and write out an HTML table element
    for (int i = 128; i > 0; i = i >> 1)
    {
        char buffer[80];

        if (v & i)
        {
            // The LED is currently on, so we want to output the following HTML code:
            // <td><A href="LED.HTML?#"><img src="ON.GIF"></A></td>, where '#' is the
            // value of the URL that would be identical to the current URL, but with
            // this particular LED off. That way, when the viewer of the Web page
            // clicks on this LED, it will reload a page with this LED off and all
            // other LEDs remain unchanged.
            sniprintf(buffer, 80, "<td><A href=\"LED.HTML?%d\" ><img src=\"images/on.png\"> </a></td>", v & (~i));
        }
        else
        {
            // The LED is currently off, so we want to output the following HTML code:
            // <td><A href="LED.HTM?#"><img src="OFF.GIF"></A></td>, where '#' is the
            // value of the URL that would be identical to the current URL, but with
            // this particular LED on. That way, when the viewer of the web page
            // clicks on this LED, it will reload a page with this LED on and all
            // other LEDs remain unchanged.
            sniprintf(buffer, 80, "<td><A href=\"LED.HTML?%d\" ><img src=\"images/off.png\"> </a></td>", v | i);
        }
        writestring(sock, buffer);
    }
}

/**
 * @brief This function displays a set of dip switch indicators on a web page.
 * The state of each switch is represented by a bit in a 8-bit
 * register. A bit value of 0 = on, and 1 = off.
 */
void DoSwitches(int sock, PCSTR url)
{
    // Get the value of the switches
    const int NumSwitches = 8;

    // Returns an unsigned char, where each bit represents one switch
    unsigned char sw = getdipsw();

    // Write out each row of the table
    for (int i = 1; i <= NumSwitches; i++)
    {
        char buffer[80];
        if (sw & (0x80 >> (i - 1)))
        {
            // Switch is on
            sniprintf(buffer, 80, "<tr><td>Switch %d: <b>OFF</b> </td></tr>", i);
        }
        else
        {
            // Switch is off
            sniprintf(buffer, 80, "<tr><td>Switch %d: <b>ON</b> </td></tr>", i);
        }
        writestring(sock, buffer);
    }

    // Put in a link that reloads the page
    writestring(sock, "<A HREF=\"");
    writestring(sock, url);
    writestring(sock, "\"> Refresh Switches </A>");
}

/******************************************************************************
 *                TICTACTOE from here on
 *
 *
 *General Notes:
 * The comments all refer to he and I he is the human playing the game and
 * I is this program
 * The state of the Game is stored in the URL. The url will read like:
 *
 *           TT.HTM?number
 *
 * Where is a number that encodes the state of the Game...
 * This number is an or of all the square values...
 * X's (His values)
 *           0x01    0x04   0x10
 *           0x40    0x100  0x400
 *           0x1000  0x4000 0x10000
 *
 *
 *
 * O's (My values)
 *
 *           0x02    0x08  0x20
 *           0x80    0x200  0x800
 *           0x2000  0x8000 0x20000
 *
 *
 *
 *
 ******************************************************************************/

/**
 * @brief Returns true if the User wins.... Should never happen!
 */
int hewins(int i)
{
    return (((i & 0x1041) == 0x1041) || ((i & 0x4104) == 0x4104) || ((i & 0x10410) == 0x10410) || ((i & 0x15) == 0x15) ||
            ((i & 0x540) == 0x540) || ((i & 0x15000) == 0x15000) || ((i & 0x10101) == 0x10101) || ((i & 0x1110) == 0x1110));
}

/**
 * @brief Test to see if I (The computer wins)
 */
int iwin(int i)
{
    return hewins(i >> 1);
}

// The preferred moves....
const int perfmovesm[] = {512, 8, 128, 2048, 32768, 2, 32, 8192, 0x20000};

/*The cell numbers
1,2          4,8           16,32

64,128     256,512,      1024,2048

4096,8192 16384,32768, 0x10000,0x20000


x..             x.x
.*.      -> .*.  65794 -> |=32
..*             ..*

x..             x.*
.*.     and .*.  Automatic block
*..             ...


*/

/**
 * @brief Given the present board state return the next board state... (Our move)
 */
int move(int now)
{
    // Opening book
    switch (now)
    {
        case 1:
        case 64:
        case 4096:
        case 0x10000: return 512 | now;
        case 4612:
        case 1540:
        case 65794: return now |= 32;
        case 16984: return now |= 8192;
        case 16960: return now |= 8192;
        case 16912:
        case 17929: return now |= 0x20000;
        case 17920: return now |= 0x20000;
        case 66049: return now |= 32768;
        case 4672:
        case 256:
        case 532:
        case 580: return 2 | now;
    }

    int bm = 0;
    int pm;
    for (int im = 0; im < 9; im++)
    {
        pm = perfmovesm[im];
        if (((pm & now) == 0) && (((pm >> 1) & now) == 0))
        {
            // The move is legal
            int hw = 0;
            int tnow = (now | pm);
            if (iwin(tnow)) { return (tnow); }
            bm = pm;
            for (int hm = 1; hm < (1 << 18); hm = hm << 2)
            {
                if (((hm & tnow) == 0) && (((hm << 1) & tnow) == 0))
                {
                    if (hewins(hm | tnow)) { hw = 1; }
                }
            }   // For hm
            if (hw == 0) { return tnow; }
        }   // for pm
    }
    return bm | now;
}

/**
 * @brief This function fills in an HTML table with the TicTacToe game in it.
 */
void WebTickTacToe(int sock, PCSTR url)
{
    int v;
    if (gWebDebug) { iprintf("Doing TTT for [%s]\r\n", url); }

    int32_t urlEncodeOffset = 8;

    // Get a buffer to store outgoing data....
    PoolPtr pp = GetBuffer();
    pp->usedsize = 0;
    char *po = (char *)pp->pData;

    if ((*(url + urlEncodeOffset)) == '?') { v = atoi((char *)(url + urlEncodeOffset + 1)); }
    else
    {
        v = 0;
    }

    // There are nine squares each with three states yours,mine,empty
    // 00 ==empty
    // 01 == yours
    // 10 == mine
    // 11 ==bad

    if (gWebDebug) { iprintf("Tick Tack Toe V= from %d to ", v); }
    int orgv = v;
    if ((v != 0) && (!hewins(v))) { v = move(v); }
    if (gWebDebug) { iprintf("%d\n", v); }

    int vshift = v;
    int won = 0;

    // Write out the HTML Table header stuff...
    if (hewins(v))
    {
        won = 1;
        writestring(sock, "<tr><td colspan=3>You win! <A HREF=\"TT.HTML\">Play again?</A></td></tr>\n");
    }
    else if (iwin(v))
    {
        won = 1;
        writestring(sock, "<tr><td colspan=3>I win! <A HREF=\"TT.HTML\">Play again?</A></td></tr>\n");
    }
    else if ((orgv) && (orgv == v))
    {
        won = 1;
        writestring(sock, "<tr><td colspan=3>No one wins! <A HREF=\"TT.HTML\">Play again?</A></td></tr>\n");
    }

    // Now write out the board
    for (int i = 0; i < 9; i++)
    {
        // The beginning of a row
        if ((i % 3) == 0) { append(po, (char *)"<TR>"); }
        append(po, (char *)"<TD class='ttt'>");

        switch (vshift & 3)
        {
            case 0:
                // IF the game is not over include the HREF for the next move...
                // Only if the square is blank
                if (!won)
                {
                    siprintf(po, "<A HREF = \"TT.HTML?%d\"><IMG SRC=\"images/BLANK.GIF\"></A>", v + (1 << (2 * i)));
                    while (*po)
                    {
                        po++;
                    }
                }
                else
                {
                    append(po, (char *)"<IMG SRC=\"images/BLANK.GIF\">");
                }
                break;
            case 1:
                // The square belongs to the evil human!
                append(po, (char *)"<IMG SRC=\"images/cross.png\">");
                break;
            case 2:
                // The square is Mine all mine
                append(po, (char *)"<IMG SRC=\"images/not.png\">");
                break;
        }   // Switch
        // Now shift the next square into view...
        vshift = vshift >> 2;
        // Add the element end
        append(po, (char *)"</TD>");
        // If it is the end of the row add the row end...
        if ((i % 3) == 2) { append(po, (char *)"</TR>\n"); }
    }

    // We have written all of this stuff into the Pool buffer we allocated.
    // No we actually write it to the socket...
    pp->usedsize = ((unsigned char *)po - (pp->pData));
    write(sock, (char *)pp->pData, pp->usedsize);
    // Now free our storage...
    FreeBuffer(pp);
    return;
}

/**
 * @brief Display the factory demo application revision number and compilation date
 * in index.html.
 */
void DisplayFirmwareVersion(int sock, PCSTR url)
{
    writestring(sock, FirmwareVersion);
}

/**
 * @brief Displays socket state info
 */
void PrintSocketStateInfo()
{
    for (int i = 0; i < WS_MAX_SOCKS; i++)
    {
        NB::WebSocket *webSock = NB::WebSocket::GetWebSocketRecord(wsFdList[i]);
        iprintf("     Checking socket %d", wsFdList[i]);
        if (webSock != nullptr) { iprintf("     with state : %d", webSock->GetState()); }
        else
        {
            iprintf(" It's null");
        }
        iprintf("\r\n");
    }
}

/**
 * @brief Handles a WebSocket upgrade request
 */
int MyDoWSUpgrade(HTTP_Request *req, int sock, PSTR url, PSTR rxBuffer)
{
    if (httpstricmp(url, "/LED.HTML"))
    {
        int rv = WSUpgrade(req, sock);
        if (rv >= 0)
        {
            if (gWebDebug) { iprintf("WebSocket Upgrade Successful: %d\r\n", rv); }
            bool foundSocket = false;
            for (int i = 0; (i < WS_MAX_SOCKS) && !foundSocket; i++)
            {
                if (wsFdList[i] == -1)
                {
                    wsFdList[i] = rv;
                    NB::WebSocket::ws_setoption(wsFdList[i], WS_SO_TEXT);
                    foundSocket = true;
                }
            }
            return foundSocket ? 2 : 0;
        }
        else
        {
            if (gWebDebug) { iprintf("Upgrade failed with error code: %d\r\n", rv); }
            return 0;
        }
    }

    NotFoundResponse(sock, url);
    return 0;
}

/**
 * @brief Send our switch position data over any open WebSocket connections
 * we have.
 */
void SendWebSocketData()
{
    // Get the value of the switches
    for (int i = 0; i < WS_MAX_SOCKS; i++)
    {
        if (wsFdList[i] < 0) { continue; }

        // Get the value of the switches
        const int NumSwitches = 8;

        // Returns an unsigned char, where each bit represents one switch
        unsigned char sw = getdipsw();

        // Our JSON blob that we will send
        ParsedJsonDataSet jsonOutObj;

        // Build the JSON blob
        jsonOutObj.StartBuilding();
        jsonOutObj.AddObjectStart("DipSwitches");

        // Write out each row of the table
        for (int i = 1; i <= NumSwitches; i++)
        {
            char buffer[10];
            sniprintf(buffer, 10, "%d", i);
            jsonOutObj.Add(buffer, ((sw & (0x80 >> (8 - i))) != 0));
        }

        jsonOutObj.EndObject();
        jsonOutObj.DoneBuilding();

        int dataLen = jsonOutObj.PrintObjectToBuffer(ReportBuffer, ReportBufSize);
        ReportBuffer[dataLen - 1] = 0;
        if (writeall(wsFdList[i], ReportBuffer, dataLen - 1) < 0)
        {
            if (gWebDebug) { iprintf("Closing WebSocket.\r\n", wsFdList[i]); }
            close(wsFdList[i]);
            wsFdList[i] = -1;
        }
    }
}

/**
 * @brief Register our WebSocket upgrade handler.
 */
void RegisterWebFuncs()
{
    TheWSHandler = MyDoWSUpgrade;
}
