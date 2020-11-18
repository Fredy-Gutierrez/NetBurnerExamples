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

/*
 *-------------------------------------------------------------------
 * Embedded Flash File System for on-chip flash memory (EFFS-STD)
 * configuration file for common parameters.
 * This file is part of an example that allocates flash space
 * to the file system, and the rest to the application.
 *
 * Note:
 *    COMPCODEFLAGS contain starting and ending addresses
 *    of application. To add file system you must modify the ending
 *    address to provide for the flash used. Not to do so will result
 *    in unpredictable results.
 *
 * See"
 *    "NetBurner Embedded Flash File System, Hardware and Software
 *    Guide" for detailed information.
 *
 *-------------------------------------------------------------------
 */

/****************************************************************************
 *
 *            Copyright (c) 2003 by HCC Embedded
 *
 * This software is copyrighted by and is the sole property of
 * HCC.  All rights, title, ownership, or other interests
 * in the software remain the property of HCC.  This
 * software may only be used in accordance with the corresponding
 * license agreement.  Any unauthorized use, duplication, transmission,
 * distribution, or disclosure of this software is expressly forbidden.
 *
 * This Copyright notice may not be removed or modified without prior
 * written consent of HCC.
 *
 * HCC reserves the right to modify this software without notice.
 *
 * HCC Embedded
 * Budapest 1132
 * Victor Hugo Utca 11-15
 * Hungary
 *
 * Tel:  +36 (1) 450 1302
 * Fax:  +36 (1) 450 1303
 * http: www.hcc-embedded.com
 * email: info@hcc-embedded.com
 *
 ***************************************************************************/

/* NB Library Definitions */
#include <predef.h>

/* C Standard Library */
#include <stdio.h>
#include <stdlib.h>

/* Portability & uCos Definitions */
#include <basictypes.h> /* startnet.h */

/* NB Runtime Libraries */
#include <constants.h> /* startnet.h */

/* NB EFFS-STD Library */
#include <file/flashdrv.h>

/* Ethernet to Serial Application EFFS-STD  */
#include "effs_std.h"
#include "fs_main.h"

extern int fs_phy_OnChipFlash(FS_FLASH *flash);

/*
 ******************************************************************************
 * Debug support
 ******************************************************************************
 */
#define debug_iprintf(...)                   \
    {                                        \
        if (bShowDebug == TRUE)              \
        {                                    \
            iprintf("%s : ", deviceNamePtr); \
            iprintf(__VA_ARGS__);            \
            iprintf("\r\n");                 \
        }                                    \
    }

/* Debugging flag */
extern BOOL bShowDebug;

/*-------------------------------------------------------------------------
 * Start up the on-chip flash file system
 *------------------------------------------------------------------------*/
void EffsStart(char *deviceNamePtr)
{
    long mem_size;
    char *mem_ptr;
    int rc;

    debug_iprintf("File system version: %s", fs_getversion());

    /* Initialize the filesystem */
    fs_init();

    /* Gets the needed memory for the filesystem */
    mem_size = fs_getmem_flashdrive(fs_phy_OnChipFlash);
    if (mem_size != 0)
    {
        mem_ptr = (char *)malloc(mem_size);
        if (mem_ptr != NULL)
        {
            /* Mount file system */
            rc = fd_mountstd(NOR_DRV_NUM, mem_ptr, mem_size, fs_mount_flashdrive, fs_phy_OnChipFlash); /* mounts filesystem */
            if (rc != FS_VOL_OK)
            {
                /* if mount was not successfull then format the drive */
                debug_iprintf("Formatting.(%d,%ld)", rc, mem_size);
                rc = fs_format(NOR_DRV_NUM);
            }

            if (rc != FS_NOERR) { debug_iprintf(" Failed->%d", rc); }
        }
        else
        {
            debug_iprintf("Memory allocation error");
        }
    }
    else
    {
        debug_iprintf("Flash drive error");
    }

    /* Setup default directory */
    (void)fd_chdrive(NOR_DRV_NUM);
    (void)fd_chdir("/");

    if (bShowDebug)
    {
        EffsDisplayStatistics((char *)"");
        EffsListCurrentDirectory((char *)"");
    }

    return;
}

/*-----------------------------------------------------------------------------------
 * Displays information in MTTTY of subdirectories and files in a current directory.
 * The information displayed is: the filename, modified time stamp,
 * modified date stamp, and filesize.
 *---------------------------------------------------------------------------------*/
void EffsDisplyFileInfo(FS_FIND *f)
{
    unsigned short t, d;   // t = time, d = date

    int nret = fs_gettimedate(f->filename, &t, &d);
    if (nret == FS_NOERR)
    {
        iprintf("%15s   |", f->filename);
        iprintf("%2.2d:%2.2d:%2.2d   |", ((t & 0xF800) >> 11), ((t & 0x07E0) >> 5), 2 * (t & 0x001F));
        iprintf("%2.2d/%2.2d/%4d   |", ((d & 0x01E0) >> 5), (d & 0x001F), (1980 + ((d & 0xFE00) >> 9)));
        iprintf("%9ld Bytes\r\n", f->len);
    }
    else
    {
        iprintf("Time stamp retrieval failed: %d\r\n", nret);
    }
}

void EffsListCurrentDirectory(char *deviceNamePtr)
{
    FS_FIND finder;   // location to store the information retrieved
    static int sEffsListCurrentDirectoryEntered = 0;

    /* Find first file or subdirectory in specified directory. First call the
       f_findfirst function, and if file was found get the next file with
       f_findnext function. Files with the system attribute set will be ignored.
       Note: If f_findfirst() is called with "*.*" and the current directory is
       not the root directory, the first entry found will be "." - the current
       directory.
    */
    sEffsListCurrentDirectoryEntered++;
    int rc = fs_findfirst("*", &finder);
    if (rc == F_NO_ERROR)   // found a file or directory
    {
        do
        {
            if ((finder.attr & FS_ATTR_DIR) == FS_ATTR_DIR)
            {
                debug_iprintf("Found Directory [%s]", finder.filename);

                if (finder.filename[0] != '.')
                {
                    fs_chdir(finder.filename);

                    /*** Recursion ***/
                    if (sEffsListCurrentDirectoryEntered > 8)
                    {
                        debug_iprintf("Directory too deep");
                        break;
                    }
                    EffsListCurrentDirectory(deviceNamePtr);
                    fs_chdir("..");
                }
            }
            else
            {
                EffsDisplyFileInfo(&finder);
            }
        } while (fs_findnext(&finder) == FS_NOERR);
    }
    else
    {
        debug_iprintf("fs_findfirest() found no files: %d", rc);
    }

    /* Ta dah */
    if (bShowDebug == TRUE) iprintf("\r\n");

    sEffsListCurrentDirectoryEntered--;

    return;
}

void EffsDisplayStatistics(char *deviceNamePtr)
{
    FS_SPACE space;
    int rv;
    rv = fs_getfreespace(NOR_DRV_NUM, &space);

    if (rv == F_NO_ERROR)
    {
        iprintf("Flash memory usage (bytes):");
        iprintf("%ld total, %ld free, %ld used, %ld bad", space.total, space.free, space.used, space.bad);
    }
    else
    {
        iprintf("*** Error in f_getfreepace(): ");
    }

    // if ( bShowDebug == TRUE )iprintf( "\r\n" );
    iprintf("\r\n");

    return;
}

/*-------------------------------------------------------------------
 * Format flash file system
 -------------------------------------------------------------------*/
uint8_t EffsFormat()
{
    int rv;
    iprintf("Formatting File System ... ");
    rv = fs_format(NOR_DRV_NUM);
    if (rv != FS_NOERR) { iprintf("*** Error in fs_format(): %d", rv); }
    else
        iprintf("complete\r\n");
    return rv;
}
