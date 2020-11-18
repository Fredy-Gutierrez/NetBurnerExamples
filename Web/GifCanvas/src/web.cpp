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
#include <stdlib.h>
#include <string.h>
#include <httppost.h>

#include "drawimage.h"
#include "webFormValues.h"

extern "C"
{
    void DoCanHt(int sock, PCSTR url);
    void DoCanWd(int sock, PCSTR url);
    void DoFred(int sock, PCSTR url);
    void DoFGrn(int sock, PCSTR url);
    void DoFBlu(int sock, PCSTR url);
    void DoBRed(int sock, PCSTR url);
    void DoBGrn(int sock, PCSTR url);
    void DoBBlu(int sock, PCSTR url);
    void DoTRed(int sock, PCSTR url);
    void DoTGrn(int sock, PCSTR url);
    void DoTBlu(int sock, PCSTR url);
    void DoBox(int sock, PCSTR url);
    void DoGradBox(int sock, PCSTR url);
    void DoTextBox(int sock, PCSTR url);
    void DoRandomBox(int sock, PCSTR url);
};


extern const char GiantFont[];

/**
 *  GetParam
 *
 *  Extract a Parameter for a URL
 */
uint16_t GetParam(PCSTR url, uint8_t offset, uint8_t digits, uint16_t defaultv)
{
    const char *cp = url;
    uint16_t val = defaultv;

    while ((*cp) && (*cp != '?'))
    {
        cp++;
    }
    if ((*cp == '?') && ((int)strlen(cp) >= (offset + digits + 1)))
    {
        cp += offset + 1;
        val = 0;
        do
        {
            val = val * 16;

            if ((*cp >= '0') && (*cp <= '9')) { val = val + (*cp) - '0'; }
            else if ((*cp >= 'A') && (*cp <= 'F'))
            {
                val = val + 10 + (*cp) - 'A';
            }
            else if ((*cp >= 'a') && (*cp <= 'f'))
            {
                val = val + 10 + (*cp) - 'a';
            }
            cp++;
            digits--;
        } while (digits);
    }

    return val;
}

/**
 *  ShowParam
 *
 *  Show the value of a URL encoded parameter used for filling out the existing values in a form
 */
void ShowParam(int sock, PCSTR url, uint8_t offset, uint8_t digits, uint16_t defaultv)
{
    char buffer[20];
    sniprintf(buffer, 20, "%d", GetParam(url, offset, digits, defaultv));

    writestring(sock, buffer);
}

/**
 *  DoCanHt
 *
 *  Fill in the Ht value on the form
 */
void DoCanHt(int sock, PCSTR url)
{
    ShowParam(sock, url, 0, 3, DEF_HT);
}

/**
 *  DoCanWd
 *
 *  Fill in width value on the form
 */
void DoCanWd(int sock, PCSTR url)
{
    ShowParam(sock, url, 3, 3, DEF_WIDTH);
}

/**
 *  The following functions fill in various color values on the form
 */
void DoFred(int sock, PCSTR url)
{
    ShowParam(sock, url, 6, 2, DEF_FRED);
}

void DoFGrn(int sock, PCSTR url)
{
    ShowParam(sock, url, 8, 2, DEF_FGREEN);
}

void DoFBlu(int sock, PCSTR url)
{
    ShowParam(sock, url, 10, 2, DEF_FBLUE);
}

void DoBRed(int sock, PCSTR url)
{
    ShowParam(sock, url, 12, 2, DEF_BRED);
}

void DoBGrn(int sock, PCSTR url)
{
    ShowParam(sock, url, 14, 2, DEF_BGREEN);
}

void DoBBlu(int sock, PCSTR url)
{
    ShowParam(sock, url, 16, 2, DEF_BBLUE);
}

void DoTRed(int sock, PCSTR url)
{
    ShowParam(sock, url, 18, 2, DEF_TRED);
}

void DoTGrn(int sock, PCSTR url)
{
    ShowParam(sock, url, 20, 2, DEF_TGREEN);
}

void DoTBlu(int sock, PCSTR url)
{
    ShowParam(sock, url, 22, 2, DEF_TBLUE);
}

/**
 *  DoBox
 *
 *  Write out the URL for the GIF image that will correspond to the solid box
 */
void DoBox(int sock, PCSTR url)
{
    WebFormValues wfv(url);
    wfv.WriteUrl(sock, "BOX.GIF?");
}

/**
 *  DoGradBox
 *
 *  Write out the URL for the GIF image that will correspond to the box with the gradient in it
 */
void DoGradBox(int sock, PCSTR url)
{
    WebFormValues wfv(url);
    wfv.WriteUrl(sock, "GRADBOX.GIF?");
}

/**
 *  DoTextBox
 *
 *  Write out the URL for the GIF image that will correspond to the box with text in it
 */
void DoTextBox(int sock, PCSTR url)
{
    WebFormValues wfv(url);
    wfv.WriteUrl(sock, "TEXTBOX.GIF?");
}

/**
 *  DoRandomBox
 *
 *  Write out the URL for the GIF image that will correspond to the randomly generated boxes
 */
void DoRandomBox(int sock, PCSTR url)
{
    WebFormValues wfv(url);
    wfv.WriteUrl(sock, "RANDOMBOX.GIF?");
}


/**
 *  DoGifPost
 *
 *  Handles the post call that contains the data we want to build our GIF images with
 */

void DoGifPost( int sock,PostEvents event,const char * pName, const char * pValue)
{
static WebFormValues wfv;

switch(event)
{
case eStartingPost:
     break;
case eVariable:
    if(strcmp(pName,"canHt")==0) wfv.m_ht=atoi(pValue);
    else
    if(strcmp(pName,"canWd")==0) wfv.m_wid=atoi(pValue);
    else
    if(strcmp(pName,"FRed" )==0) wfv.m_fillRed=atoi(pValue);
    else
    if(strcmp(pName,"FGrn" )==0) wfv.m_fillGreen=atoi(pValue);
    else
    if(strcmp(pName,"FBlu" )==0) wfv.m_fillBlue=atoi(pValue);
    else
    if(strcmp(pName,"BRed" )==0) wfv.m_borderRed=atoi(pValue);
    else
    if(strcmp(pName,"BGrn" )==0) wfv.m_borderGreen=atoi(pValue);
    else
    if(strcmp(pName,"BBlu" )==0) wfv.m_borderBlue=atoi(pValue);
    else
    if(strcmp(pName,"TRed" )==0) wfv.m_textRed=atoi(pValue);
    else
    if(strcmp(pName,"TGrn" )==0) wfv.m_textGreen=atoi(pValue);
    else
    if(strcmp(pName,"TBlu" )==0) wfv.m_textBlue=atoi(pValue);
    break;
case eFile:    //Unused
break;


case eEndOfPost:
{
    char buffer[80];

        sniprintf(buffer, 80, "INDEX.HTML?%03X%03X%02X%02X%02X%02X%02X%02X%02X%02X%02X", wfv.m_ht, wfv.m_wid, wfv.m_fillRed, wfv.m_fillGreen,
              wfv.m_fillBlue, wfv.m_borderRed, wfv.m_borderGreen, wfv.m_borderBlue, wfv.m_textRed, wfv.m_textGreen, wfv.m_textBlue);

    RedirectResponse(sock, buffer);
}
break;
}//Switch

}


/**
 *  GenerateBox
 *
 *  Creates an outlined, filled box
 */
int GenerateBox(int sock, HTTP_Request & pr)
{
    WebFormValues wfv(pr.pURL);
    DrawImageObject dio(wfv.m_wid, wfv.m_ht, 3, false, 0);
    dio.SetColor(0, wfv.m_fillRed, wfv.m_fillGreen, wfv.m_fillBlue);
    dio.SetColor(1, wfv.m_borderRed, wfv.m_borderGreen, wfv.m_borderBlue);

    dio.FilledBox(0, 0, wfv.m_wid - 1, wfv.m_ht - 1, 0, 1);
    dio.WriteGIF(sock);
    return 1;
}

/**
 *  GenerateGradBox
 *
 *  Creates a box filled with a gradient between two colors
 */
int GenerateGradBox(int sock, HTTP_Request & pr)
{
    WebFormValues wfv(pr.pURL);
    // Determine how many colors we need to use for the gradient, up to the max of 256
    int numColors = (256 > wfv.m_wid) ? wfv.m_wid : 256;

    // Create an image object that will support up to 256 colors
    DrawImageObject dio(wfv.m_wid, wfv.m_ht, numColors, false, 0);

    // Determine the step values to increment our colors by
    float redStep = (float)(wfv.m_fillRed - wfv.m_borderRed) / ((float)numColors - 1);
    float greenStep = (float)(wfv.m_fillGreen - wfv.m_borderGreen) / ((float)numColors - 1);
    float blueStep = (float)(wfv.m_fillBlue - wfv.m_borderBlue) / ((float)numColors - 1);

    // Calculate how many columns of pixels we will use for each color step
    float linesPerStep = (numColors == wfv.m_wid) ? 1 : (float)wfv.m_wid / (float)numColors;

    // Set our colors in the color map
    for (int i = 0; i < numColors; i++)
    {
        dio.SetColor(i, uint8_t(wfv.m_fillRed - (redStep * i)), uint8_t(wfv.m_fillGreen - (greenStep * i)),
                     uint8_t(wfv.m_fillBlue - (blueStep * i)));
    }

    // Draw the lines
    for (int i = 0; i < wfv.m_wid; i++)
    {
        dio.Line(i, 0, i, wfv.m_ht - 1, int(i / linesPerStep));
    }

    // Write the GIF out
    dio.WriteGIF(sock);
    return 1;

}

/**
 *  GenerateTextBox
 *
 *  Creates a filled box with some text displayed
 */
int GenerateTextBox(int sock, HTTP_Request & pr)
{
    WebFormValues wfv(pr.pURL);
    // Create an image object that will support 3 colors
    DrawImageObject dio(wfv.m_wid, wfv.m_ht, 3, false, 0);

    // Set the colors we will be using
    dio.SetColor(0, wfv.m_textRed, wfv.m_textGreen, wfv.m_textBlue);
    dio.SetColor(1, wfv.m_borderRed, wfv.m_borderGreen, wfv.m_borderBlue);
    dio.SetColor(2, wfv.m_fillRed, wfv.m_fillGreen, wfv.m_fillBlue);

    // Draw a filled box
    dio.FilledBox(0, 0, wfv.m_wid - 1, wfv.m_ht - 1, 1, 2);

    // Draw our text
    dio.Text("Your Text Here!!!", wfv.m_ht / 6, wfv.m_ht / 2.5, GiantFont, 0);

    // Write the GIF out
    dio.WriteGIF(sock);
    return 1;
}

/**
 *  GenerateRandomBox
 *
 *  Generates a GIF filled with random box outlines
 */
int GenerateRandomBox(int sock,HTTP_Request & pr )
{
    WebFormValues wfv(pr.pURL);
    // Create an image object that supports transparency, and set the color map
    // index value for transparency to 0
    DrawImageObject dio(wfv.m_wid, wfv.m_ht, 256, true, 0);

    // Set our transparent color to white
    dio.SetColor(0, 255, 255, 255);

    // Randomly create the other colors we will be using
    for (int i = 1; i < 256; i++)
    {
        uint8_t red = rand() % 256;
        uint8_t green = rand() % 256;
        uint8_t blue = rand() % 256;

        dio.SetColor(i, red, green, blue);
    }

    // Make the entire canvas transparent
    dio.Box(0, 0, wfv.m_wid - 1, wfv.m_ht - 1, 0);

    // Determine how many squares we want to draw
    int squares = wfv.m_wid < 4 ? 4 : wfv.m_wid / 4;

    // Generate the coordinates for our squares, and draw them to the canvas
    for (int i = 0; i < squares; i++)
    {
        int x1 = rand() % wfv.m_wid;
        int y1 = rand() % wfv.m_ht;
        int x2 = rand() % wfv.m_wid;
        int y2 = rand() % wfv.m_ht;

        uint8_t colorInd = rand() % 256;

        dio.Box(x1, y1, x2, y2, colorInd);
    }

    // Write the GIF out
    dio.WriteGIF(sock);
    return 1;

}

// Callbacks to process specific form posts
HtmlPostVariableListCallback p1("MAKEGIF.HTM",DoGifPost);

// Callbacks to process specific GET requests
CallBackFunctionPageHandler h1("BOX.GIF?*",GenerateBox);
CallBackFunctionPageHandler h2("GRADBOX.GIF?*",GenerateGradBox);
CallBackFunctionPageHandler h3("TEXTBOX.GIF?*",GenerateTextBox);
CallBackFunctionPageHandler h4("RANDOMBOX.GIF?*",GenerateRandomBox);



