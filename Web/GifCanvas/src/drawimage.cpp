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
 *  @file   drawimage.cpp
 *  @brief  Function definitions for the DrawImageObject class
 *
 *  This class hold the data associated with and draws GIF images
 *  with text to a given file descriptor
 */

#include <iosys.h>
#include <malloc.h>
#include <string.h>

#include "drawimage.h"
#include "gifCompress.h"

static GitCompress gifCompress;

/* Forward declarations */
int writeall(int fd, const char *c, int siz);
void FlushData(int fd);
void WriteData(int fd, const char *c, int siz);
void OutputGifChar(const char c, int fd);

/**
 *  DrawImageObject
 *
 *  The object constructor
 */
DrawImageObject::DrawImageObject(int x, int y, int ncolors, bool transparent, uint8_t transparentIndex)
    : m_xSize(x), m_ySize(y), m_nColors(ncolors), m_trans(transparent), m_transIndex(transparent)
{
    m_curx = 0;
    m_cury = 0;

    if (ncolors > 256) { ncolors = 256; } /* Max 512 colors */

    m_pImageBuffer = (uint8_t *)malloc((x * y) + (ncolors * 3));
    if (m_pImageBuffer == nullptr) { return; }
    memset(m_pImageBuffer, 0, (x * y));

    m_pColorArray = m_pImageBuffer + (x * y);
    m_nColors = ncolors;
    for (int i = 0; i < m_nColors; i++)
    {
        SetColor(i, 0, 0, 0);
    }
}

/**
 *  ~DrawImageObject
 *
 *  The object deconstructors
 */
DrawImageObject::~DrawImageObject()
{
    free(m_pImageBuffer);
}

/************************* Drawing functions ***************************/

/**
 *  PutPixel
 *
 *  Assigns the pixel at x, y the color passed in
 */
void DrawImageObject::PutPixel(int x, int y, uint8_t color)
{
    if ((x >= m_xSize) || (y >= m_ySize) || (color >= m_nColors)) { return; }
    m_pImageBuffer[x + (y * m_xSize)] = color;
}

/**
 *  GetPixel
 *
 *  Returns the color of the pixel at the x, y coordinate
 */
uint8_t DrawImageObject::GetPixel(int x, int y)
{
    if ((x >= m_xSize) || (y >= m_ySize)) { return 0; }
    return m_pImageBuffer[x + (y * m_xSize)];
}

/**
 *  SetColor
 *
 *  Sets the color of the colorArray at the index provided
 */
void DrawImageObject::SetColor(uint8_t index, uint8_t red, uint8_t green, uint8_t blue)
{
    if (index >= m_nColors) { return; }
    m_pColorArray[index * 3] = red;
    m_pColorArray[index * 3 + 1] = green;
    m_pColorArray[index * 3 + 2] = blue;
}

/**
 *  Box
 *
 *  Draws a box
 */
void DrawImageObject::Box(int x1, int y1, int x2, int y2, uint8_t color)
{
    Line(x1, y1, x2, y1, color);
    Line(x2, y1, x2, y2, color);
    Line(x2, y2, x1, y2, color);
    Line(x1, y2, x1, y1, color);
}

/**
 *  FilledBox
 *
 *  Draws a box and fills it in
 */
void DrawImageObject::FilledBox(int x1, int y1, int x2, int y2, uint8_t fillcolor, uint8_t outlinecolor)
{
    if (y1 < y2)
    {
        for (int yp = y1; yp <= y2; yp++)
        {
            Line(x1, yp, x2, yp, fillcolor);
        }
    }
    else if (y1 > y2)
    {
        for (int yp = y2; yp <= y1; yp++)
        {
            Line(x1, yp, x2, yp, fillcolor);
        }
    }

    Box(x1, y1, x2, y2, outlinecolor);
}

/**
 *  Line
 *
 *  Draws a line
 */
void DrawImageObject::Line(int x1, int y1, int x2, int y2, uint8_t color)
{
    int runcount = 0;
    int dx, dy = 0;
    int xinc, yinc = 0;
    int xplot, yplot = 0;

    if (x2 > x1)
    {
        dx = x2 - x1;
        xinc = 1;
    }
    if (x2 == x1)
    {
        dx = 0;
        xinc = 0;
    }
    if (x2 < x1)
    {
        dx = x1 - x2;
        xinc = -1;
    }

    if (y2 > y1)
    {
        dy = y2 - y1;
        yinc = 1;
    }
    if (y2 == y1)
    {
        dy = 0;
        yinc = 0;
    }
    if (y2 < y1)
    {
        dy = y1 - y2;
        yinc = -1;
    }

    xplot = x1;
    yplot = y1;
    PutPixel(xplot, yplot, color);
    if (dx > dy)
    {
        /* iterate x */
        while (xplot != x2)
        {
            xplot += xinc;
            runcount += dy;
            if (runcount >= (dx - runcount))
            {
                yplot += yinc;
                runcount -= dx;
            }
            PutPixel(xplot, yplot, color);
        }
    }
    else
    {
        /* iterate y */
        while (yplot != y2)
        {
            yplot += yinc;
            runcount += dx;
            if (runcount >= (dy - runcount))
            {
                xplot += xinc;
                runcount -= dy;
            }
            PutPixel(xplot, yplot, color);
        }
    }
}   // Line

/**************************Text drawing functions *************************8*/
struct FontData
{
    uint8_t nchars = 0;
    uint8_t offset = 0;
    uint8_t xsiz = 0;
    uint8_t ysiz = 0;
    uint8_t data[2] = {0};
};

/**
 *  GetBit
 *
 *  Returns the bit at a given position
 */
static int GetBit(const uint8_t *cp, int bitnum)
{
    int offset = bitnum / 8;
    int bitpos = (bitnum % 8);
    if (cp[offset] & (1 << (7 - bitpos))) { return 1; }
    return 0;
}

/**
 *  WriteOneChar
 *
 *  Writes a single character
 */
void WriteOneChar(const FontData *pf, char c, int xp, int yp, uint8_t color, DrawImageObject &doi)
{
    if ((c < pf->offset) || (c >= (pf->nchars + pf->offset)))
    {
        /*Skip the char */
        return;
    }
    else
    {
        int bitoffset;
        bitoffset = (int)(c - pf->offset) * ((int)pf->xsiz * (int)pf->ysiz);
        for (int yi = 0; yi < pf->ysiz; yi++)
        {
            for (int xi = 0; xi < pf->xsiz; xi++)
            {
                if (GetBit(pf->data, xi + (int)(yi * pf->xsiz) + bitoffset)) { doi.PutPixel(xp + xi, yp + yi, color); }
            }
        }
    }
}

/**
 *  Text
 *
 *  Draws text
 */
void DrawImageObject::Text(const char *pText, int x1, int y1, const char *fontrecord, uint8_t color)
{
    const FontData *pf = (FontData *)fontrecord;
    int xp = x1;
    int yp = y1;
    const char *cp = pText;
    while (*cp)
    {
        WriteOneChar(pf, *cp, xp, yp, color, *this);
        xp += pf->xsiz;
        cp++;
    };
}

/**
 *  TextXsize
 *
 *  Returns the length of the text
 */
int DrawImageObject::TextXsize(const char *pText, const char *fontrecord)
{
    const FontData *pf = (FontData *)fontrecord;
    return strlen(pText) * pf->xsiz;
}

/**
 *  TextYsize
 *
 *  Returns the height of the text
 */
int DrawImageObject::TextYsize(const char *pText, const char *fontrecord)
{
    const FontData *pf = (FontData *)fontrecord;
    return pf->ysiz;
}

/* Code drawn from ppmtogif.c, from the pbmplus package
 *
 * Based on GIFENCOD by David Rowley <mgardi@watdscu.waterloo.edu>. A
 * Lempel-Zim compression based on "compress".
 *
 * Modified by Marcel Wijkstra <wijkstra@fwi.uva.nl>
 *
 * Copyright (C) 1989 by Jef Poskanzer.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  This software is provided "as is" without express or
 * implied warranty.
 *
 * The Graphics Interchange Format(c) is the Copyright property of
 * CompuServe Incorporated.  GIF(sm) is a Service Mark property of
 * CompuServe Incorporated.
 *
 *  Heavily modified by Mouse, 1998-02-12.
 *  Remove LZW compression.
 *  Added miGIF run length compression.
 */

/*
 * a code_int must be able to hold 2**GIFBITS values of type int, and also -1
 */
typedef int code_int;

static int colorstobpp(int colors);

static void Putword(int w, int fd);
static void output(code_int code);

static int colorstobpp(int colors)
{
    int bpp = 0;

    if (colors <= 2) { bpp = 1; }
    else if (colors <= 4)
    {
        bpp = 2;
    }
    else if (colors <= 8)
    {
        bpp = 3;
    }
    else if (colors <= 16)
    {
        bpp = 4;
    }
    else if (colors <= 32)
    {
        bpp = 5;
    }
    else if (colors <= 64)
    {
        bpp = 6;
    }
    else if (colors <= 128)
    {
        bpp = 7;
    }
    else if (colors <= 256)
    {
        bpp = 8;
    }
    return bpp;
}

/**
 *  GIFNextPixel
 *
 *  Return the next pixel from the image
 */
int DrawImageObject::GIFNextPixel()
{
    int r;
    if ((m_curx == 0) && (m_cury == m_ySize)) { return EOF; }
    r = GetPixel(m_curx, m_cury);
    m_curx++;
    if (m_curx == m_xSize)
    {
        m_curx = 0;
        m_cury++;
    }
    return r;
}

/**
 *  WriteGIF
 *
 *  Write the GIF image out to the given fd
 */
void DrawImageObject::WriteGIF(int fd)
{
    int B;
    int LeftOfs, TopOfs;
    int Resolution;
    int ColorMapSize;
    int InitCodeSize;
    int i;
    int BitsPerPixel;

    m_gitCompress.ResetStatistics();
    BitsPerPixel = colorstobpp(m_nColors);

    ColorMapSize = 1 << BitsPerPixel;
    LeftOfs = TopOfs = 0;
    Resolution = BitsPerPixel;

    /*
     * The initial code size
     */
    if (BitsPerPixel <= 1) { InitCodeSize = 2; }
    else
    {
        InitCodeSize = BitsPerPixel;
    }

    /*
     * Set up the current x and y position
     */
    m_curx = m_cury = 0;

    /*
     * Write the Magic header
     */
    WriteData(fd, m_trans ? "GIF89a" : "GIF87a", 6);

    /*
     * Write out the screen width and height
     */
    Putword(m_xSize, fd);
    Putword(m_ySize, fd);

    /*
     * Indicate that there is a global colour map
     */
    B = 0x80; /* Yes, there is a color map */

    /*
     * OR in the resolution
     */
    B |= (Resolution - 1) << 4;

    /*
     * OR in the Bits per Pixel
     */
    B |= (BitsPerPixel - 1);

    /*
     * Write it out
     */
    OutputGifChar(B, fd);

    /*
     * Write out the Background colour
     */
    OutputGifChar(0, fd);

    /*
     * Byte of 0's (future expansion)
     */
    OutputGifChar(0, fd);

    /*
     * Write out the Global Colour Map
     */
    for (i = 0; i < ColorMapSize; ++i)
    {
        if (i > m_nColors)
        {
            OutputGifChar(0, fd);
            OutputGifChar(0, fd);
            OutputGifChar(0, fd);
        }
        else
        {
            OutputGifChar(m_pColorArray[i * 3], fd);
            OutputGifChar(m_pColorArray[i * 3 + 1], fd);
            OutputGifChar(m_pColorArray[i * 3 + 2], fd);
        }
    } /* For */

    /*
     * Write out extension for transparent colour index, if necessary.
     */
    if (m_trans)
    {
        OutputGifChar('!', fd);
        OutputGifChar(0xf9, fd);
        OutputGifChar(4, fd);
        OutputGifChar(1, fd);
        OutputGifChar(0, fd);
        OutputGifChar(0, fd);
        OutputGifChar((unsigned char)m_transIndex, fd);
        OutputGifChar(0, fd);
    }

    /*
     * Write an Image separator
     */
    OutputGifChar(',', fd);

    /*
     * Write the Image header
     */

    Putword(LeftOfs, fd);
    Putword(TopOfs, fd);
    Putword(m_xSize, fd);
    Putword(m_ySize, fd);

    /*
     * Write out whether or not the image is interlaced
     */
    OutputGifChar(0x00, fd);

    /*
     * Write out the initial code size
     */
    OutputGifChar(InitCodeSize, fd);

    /*
     * Go and actually compress the data
     */
    compress(InitCodeSize + 1, fd);

    /*
     * Write out a Zero-length packet (to end the series)
     */
    OutputGifChar(0, fd);

    /*
     * Write the GIF file terminator
     */
    OutputGifChar(';', fd);
    FlushData(fd);
}

/**
 *  Putword
 *
 *  Write out a word to the GIF file
 */
static void Putword(int w, int fd)
{
    OutputGifChar(w & 0xff, fd);
    OutputGifChar((w / 256) & 0xff, fd);
}

#define GIFBITS 12

/**
 *  compress
 *
 *  Compress the image
 */
void DrawImageObject::compress(int init_bits, int fd)
{
    m_gitCompress.compress(*this, init_bits, fd);
}
