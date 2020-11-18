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

/*********************************************************************
**********************************************************************
**********************************************************************


Code from here on is extracted with some changes from the
GD distribution


**********************************************************************
**********************************************************************
*********************************************************************/

/*-----------------------------------------------------------------------
 *
 * miGIF Compression - mouse and ivo's GIF-compatible compression
 *
 *          -run length encoding compression routines-
 *
 * Copyright (C) 1998 Hutchison Avenue Software Corporation
 *               http://www.hasc.com
 *               info@hasc.com
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  This software is provided "AS IS." The Hutchison Avenue
 * Software Corporation disclaims all warranties, either express or implied,
 * including but not limited to implied warranties of merchantability and
 * fitness for a particular purpose, with respect to this code and accompanying
 * documentation.
 *
 * The miGIF compression routines do not, strictly speaking, generate files
 * conforming to the GIF spec, since the image data is not LZW-compressed
 * (this is the point: in order to avoid transgression of the Unisys patent
 * on the LZW algorithm.)  However, miGIF generates data streams that any
 * reasonably sane LZW decompresser will decompress to what we want.
 *
 * miGIF compression uses run length encoding. It compresses horizontal runs
 * of pixels of the same color. This type of compression gives good results
 * on images with many runs, for example images with lines, text and solid
 * shapes on a solid-colored background. It gives little or no compression
 * on images with few runs, for example digital or scanned photos.
 *
 *                               der Mouse
 *                      mouse@rodents.montreal.qc.ca
 *            7D C8 61 52 5D E7 2D 39  4E F1 31 3E E8 B3 27 4B
 *
 *                             ivo@hasc.com
 *
 * The Graphics Interchange Format(c) is the Copyright property of
 * CompuServe Incorporated.  GIF(sm) is a Service Mark property of
 * CompuServe Incorporated.
 */

#include <buffers.h>
#include <iosys.h>
#include <string.h>

#include "drawimage.h"
#include "gifCompress.h"

/* A static buffer to help queue the output */
static PoolPtr pDataPool = nullptr;

void GitCompress::ResetStatistics()
{
    rl_pixel = 0;
    rl_basecode = 0;
    rl_count = 0;
    rl_table_pixel = 0;
    rl_table_max = 0;
    just_cleared = 0;
    out_bits = 0;
    out_bits_init = 0;
    out_count = 0;
    out_bump = 0;
    out_bump_init = 0;
    out_clear = 0;
    out_clear_init = 0;
    max_ocodes = 0;
    code_clear = 0;
    code_eof = 0;
    obuf = 0;
    obits = 0;
    ofd = 0;
    for (int i = 0; i < 256; i++)
    {
        oblock[i] = 0;
    }
    oblen = 0;
}

// static const char * binformat( unsigned int v, int nbits )
//{
//   static char bufs[8][64];
//   static int bhand = 0;
//   unsigned int bit;
//   int bno;
//   char *bp;
//
//   bhand--;
//   if (bhand < 0) bhand = 7;
//   bp = &bufs[bhand][0];
//
//   for ( bno = nbits - 1,bit = 1U << bno; bno >= 0; bno--,bit >>= 1 )
//   {
//      *bp++ = ( v & bit ) ? '1' : '0';
//      if ( ( ( bno & 3 ) == 0 ) && ( bno != 0 ) )
//      {
//         *bp++ = '.';
//      }
//   }
//   *bp = '\0';
//   return( &bufs[bhand][0] );
//}

/**
 *  FlushData
 *
 *  Flush any unwritten data to the output
 */
void FlushData(int fd)
{
    if (pDataPool == nullptr) { return; }
    if (pDataPool->usedsize == 0) { return; }
    writeall(fd, (char *)(pDataPool->pData), pDataPool->usedsize);
    FreeBuffer(pDataPool);
    pDataPool = nullptr;
}

/**
 *  WriteData
 *
 *  This is the function that writes data to the output socket
 */
void WriteData(int fd, const char *c, int siz)
{
    if (pDataPool == nullptr)
    {
        pDataPool = GetBuffer();
        pDataPool->usedsize = 0;
    }
    if ((siz + pDataPool->usedsize) >= ETHER_BUFFER_SIZE)
    {
        int extra = (siz + pDataPool->usedsize) - ETHER_BUFFER_SIZE;

        memcpy(pDataPool->pData + pDataPool->usedsize, c, siz - extra);
        pDataPool->usedsize += siz - extra;
        FlushData(fd);
        if (extra) { WriteData(fd, c + (siz - extra), extra); }
    }
    else
    {
        memcpy(pDataPool->pData + pDataPool->usedsize, c, siz);
        pDataPool->usedsize += siz;
    }
}

/**
 *  OutputGifChar
 *
 *  Writes a GIF char to the given fd
 */
void OutputGifChar(const char c, int fd)
{
    WriteData(fd, &c, 1);
}

GitCompress::GitCompress() {}

void GitCompress::write_block(void)
{
    OutputGifChar(oblen, ofd);
    WriteData(ofd, (const char *)&oblock[0], oblen);
    oblen = 0;
}

void GitCompress::block_out(unsigned char c)
{
    oblock[oblen++] = c;
    if (oblen >= 255) { write_block(); }
}

void GitCompress::block_flush(void)
{
    if (oblen > 0) { write_block(); }
}

void GitCompress::output(int val)
{
    obuf |= val << obits;
    obits += out_bits;
    while (obits >= 8)
    {
        block_out(obuf & 0xff);
        obuf >>= 8;
        obits -= 8;
    }
}

void GitCompress::output_flush(void)
{
    if (obits > 0) { block_out(obuf); }
    block_flush();
}

void GitCompress::did_clear(void)
{
    out_bits = out_bits_init;
    out_bump = out_bump_init;
    out_clear = out_clear_init;
    out_count = 0;
    rl_table_max = 0;
    just_cleared = 1;
}

void GitCompress::output_plain(int c)
{
    just_cleared = 0;
    output(c);
    out_count++;
    if (out_count >= out_bump)
    {
        out_bits++;
        out_bump += 1 << (out_bits - 1);
    }
    if (out_count >= out_clear)
    {
        output(code_clear);
        did_clear();
    }
}

unsigned int GitCompress::isqrt(unsigned int x)
{
    unsigned int r;
    unsigned int v;

    if (x < 2) { return (x); }
    for (v = x, r = 1; v; v >>= 2, r <<= 1)
        ;
    while (1)
    {
        v = ((x / r) + r) / 2;
        if ((v == r) || (v == r + 1)) { return (r); }
        r = v;
    }
}

unsigned int GitCompress::compute_triangle_count(unsigned int count, unsigned int nrepcodes)
{
    unsigned int perrep;
    unsigned int cost;

    cost = 0;
    perrep = (nrepcodes * (nrepcodes + 1)) / 2;
    while (count >= perrep)
    {
        cost += nrepcodes;
        count -= perrep;
    }
    if (count > 0)
    {
        unsigned int n;
        n = isqrt(count);
        while ((n * (n + 1)) >= 2 * count)
        {
            n--;
        }
        while ((n * (n + 1)) < 2 * count)
        {
            n++;
        }
        cost += n;
    }
    return (cost);
}

void GitCompress::max_out_clear(void)
{
    out_clear = max_ocodes;
}

void GitCompress::reset_out_clear(void)
{
    out_clear = out_clear_init;
    if (out_count >= out_clear)
    {
        output(code_clear);
        did_clear();
    }
}

void GitCompress::rl_flush_fromclear(int count)
{
    int n;

    max_out_clear();
    rl_table_pixel = rl_pixel;
    n = 1;
    while (count > 0)
    {
        if (n == 1)
        {
            rl_table_max = 1;
            output_plain(rl_pixel);
            count--;
        }
        else if (count >= n)
        {
            rl_table_max = n;
            output_plain(rl_basecode + n - 2);
            count -= n;
        }
        else if (count == 1)
        {
            rl_table_max++;
            output_plain(rl_pixel);
            count = 0;
        }
        else
        {
            rl_table_max++;
            output_plain(rl_basecode + count - 2);
            count = 0;
        }
        if (out_count == 0) { n = 1; }
        else
        {
            n++;
        }
    }
    reset_out_clear();
}

void GitCompress::rl_flush_clearorrep(int count)
{
    int withclr;

    withclr = 1 + compute_triangle_count(count, max_ocodes);
    if (withclr < count)
    {
        output(code_clear);
        did_clear();
        rl_flush_fromclear(count);
    }
    else
    {
        for (; count > 0; count--)
        {
            output_plain(rl_pixel);
        }
    }
}

void GitCompress::rl_flush_withtable(int count)
{
    int repmax;
    int repleft;
    int leftover;

    repmax = count / rl_table_max;
    leftover = count % rl_table_max;
    repleft = (leftover ? 1 : 0);
    if (out_count + repmax + repleft > max_ocodes)
    {
        repmax = max_ocodes - out_count;
        leftover = count - (repmax * rl_table_max);
        repleft = 1 + compute_triangle_count(leftover, max_ocodes);
    }
    if (1 + compute_triangle_count(count, max_ocodes) < ((unsigned int)(repmax + repleft)))
    {
        output(code_clear);
        did_clear();
        rl_flush_fromclear(count);
        return;
    }
    max_out_clear();
    for (; repmax > 0; repmax--)
    {
        output_plain(rl_basecode + rl_table_max - 2);
    }
    if (leftover)
    {
        if (just_cleared) { rl_flush_fromclear(leftover); }
        else if (leftover == 1)
        {
            output_plain(rl_pixel);
        }
        else
        {
            output_plain(rl_basecode + leftover - 2);
        }
    }
    reset_out_clear();
}

void GitCompress::rl_flush(void)
{
    if (rl_count == 1)
    {
        output_plain(rl_pixel);
        rl_count = 0;
        return;
    }
    if (just_cleared) { rl_flush_fromclear(rl_count); }
    else if ((rl_table_max < 2) || (rl_table_pixel != rl_pixel))
    {
        rl_flush_clearorrep(rl_count);
    }
    else
    {
        rl_flush_withtable(rl_count);
    }
    rl_count = 0;
}

void GitCompress::compress(DrawImageObject &dio, int init_bits, int fd)
{
    int c = 0;

    ofd = fd;
    obuf = 0;
    obits = 0;
    oblen = 0;
    code_clear = 1 << (init_bits - 1);
    code_eof = code_clear + 1;
    rl_basecode = code_eof + 1;
    out_bump_init = (1 << (init_bits - 1)) - 1;
    /* for images with a lot of runs, making out_clear_init larger will
    give better compression. */
    out_clear_init = (init_bits <= 3) ? 9 : (out_bump_init - 1);

    out_bits_init = init_bits;
    max_ocodes = (1 << GIFBITS) - ((1 << (out_bits_init - 1)) + 3);
    did_clear();
    output(code_clear);
    rl_count = 0;
    while (1)
    {
        c = dio.GIFNextPixel();
        if ((rl_count > 0) && (c != rl_pixel)) { rl_flush(); }
        if (c == EOF) { break; }
        if (rl_pixel == c) { rl_count++; }
        else
        {
            rl_pixel = c;
            rl_count = 1;
        }
    }
    output(code_eof);
    output_flush();
}

/*-----------------------------------------------------------------------
 *
 * End of miGIF section  - See copyright notice at start of section.
 *
 *-----------------------------------------------------------------------*/
