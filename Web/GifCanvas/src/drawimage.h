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

/* This file contains a simple class for drawing Graphics into a GIF image */

#ifndef _DRAWIMAGE_H_
#define _DRAWIMAGE_H_

#include <basictypes.h>

#include "gifCompress.h"

class GitCompress;

class DrawImageObject
{
    uint8_t *m_pImageBuffer = nullptr;
    uint8_t *m_pColorArray = nullptr;

    int m_xSize = 0;
    int m_ySize = 0;
    int m_nColors = 0;

    int m_curx = 0;
    int m_cury = 0;

    bool m_trans = false;
    uint8_t m_transIndex = 0;

    GitCompress m_gitCompress;

    friend class GitCompress;

   private:
    int GIFNextPixel();
    void compress(int init_bits, int fd);

   public:
    /* You must specify the size and color depth of the GIF object in the constructor */
    DrawImageObject(int x, int y, int ncolors, bool transparent, uint8_t transIndex);
    ~DrawImageObject();

    /* Set a specific pixel to a specific color */
    void PutPixel(int x, int y, uint8_t color);

    /* Get the color of a specific pixel */
    uint8_t GetPixel(int x, int y);

    /* All colors are index based. You must define the color for each index */
    void SetColor(uint8_t index, uint8_t red, uint8_t green, uint8_t blue);

    /* Draw a line */
    void Line(int x1, int y1, int x2, int y2, uint8_t colorindex);

    /* Draw a box */
    void Box(int x1, int y1, int x2, int y2, uint8_t colorindex);

    /* Draw a filled box */
    void FilledBox(int x1, int y1, int x2, int y2, uint8_t fillc, uint8_t outlinec);

    /* Draw text */
    void Text(const char *pText, int x1, int x2, const char *fontrecord, uint8_t color);
    int TextXsize(const char *pText, const char *fontrecord);
    int TextYsize(const char *pText, const char *fontrecord);

    /* After you have  done all of your drawing you must call this function to send the GIF */
    void WriteGIF(int fd);
};

extern const char GiantFont[];
extern const char LargeFont[];
extern const char MediumFont[];
extern const char SmallFont[];
extern const char TinyFont[];

#endif /* _DRAWIMAGE_H_ */
