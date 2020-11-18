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

#ifndef _GIT_COMPRESS_H_
#define _GIT_COMPRESS_H_
#pragma once

#define GIFBITS 12

//#include "drawimage.h"

class DrawImageObject;

class GitCompress
{
   public:
    GitCompress();
    void ResetStatistics();
    void compress(DrawImageObject &dio, int init_bits, int fd);

   private:
    void write_block(void);
    void block_out(unsigned char c);
    void block_flush(void);
    void output(int val);
    void output_flush(void);
    void did_clear(void);
    void output_plain(int c);
    unsigned int isqrt(unsigned int x);
    unsigned int compute_triangle_count(unsigned int count, unsigned int nrepcodes);
    void max_out_clear(void);
    void reset_out_clear(void);
    void rl_flush_fromclear(int count);
    void rl_flush_clearorrep(int count);
    void rl_flush_withtable(int count);
    void rl_flush(void);

    int rl_pixel = 0;
    int rl_basecode = 0;
    int rl_count = 0;
    int rl_table_pixel = 0;
    int rl_table_max = 0;
    int just_cleared = 0;
    int out_bits = 0;
    int out_bits_init = 0;
    int out_count = 0;
    int out_bump = 0;
    int out_bump_init = 0;
    int out_clear = 0;
    int out_clear_init = 0;
    int max_ocodes = 0;
    int code_clear = 0;
    int code_eof = 0;
    unsigned int obuf = 0;
    int obits = 0;
    int ofd = 0;
    int oblen = 0;
    unsigned char oblock[256];
    bool m_transparent = false;
};

#endif /* _GIT_COMPRESS_H_ */

/*-----------------------------------------------------------------------
 *
 * End of miGIF section  - See copyright notice at start of section.
 *
 *-----------------------------------------------------------------------*/
