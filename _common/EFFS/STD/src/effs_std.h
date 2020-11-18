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
 *  Embedded Flash File System for on-chip flash memory (EFFS-STD)
 *  configuration file for common parameters.
 *  This file is part of an example that allocates flash space
 *  to the file system, and the rest to the application.
 *
 *  Note:
 *     COMPCODEFLAGS contain starting and ending addresses
 *     of application. To add file system you must modify the ending
 *     address to provide for the flash used. Not to do so will result
 *     in unpredictable results.
 *
 *  See"
 *     "NetBurner Embedded Flash File System, Hardware and Software
 *     Guide" for detailed information.
 */

#ifndef _EFFSSTD_H_
#define _EFFSSTD_H_

/*******************************************************************
 * Definitions
 *******************************************************************/
/* On-chip Flash NOR */
#define USE_NOR

/* Drive numbers */
#define NOR_DRV_NUM 0
#define STDRAM_DRV_NUM 1
#define MMC_DRV_NUM 2
#define CFC_DRV_NUM 3
#define HDD_DRV_NUM 3
#define FATRAM_DRV_NUM 4

#define FS_NO_ERROR FS_NOERR

/* Routine definitions */
#include <file/fsf.h>
#include <file/fwerr.h>

/*
******************************************************************************
*
* "C" Routines
*
******************************************************************************
*/
#ifdef __cplusplus
extern "C"
{
#endif

    /*
    ******************************************************************************

    Start EFFS

    Parameters:
    deviceNamePtr        - Device name

    Return:
    None

    Notes:
    Starts EFFS-STD file system, formats if necessary.

    ******************************************************************************
    */
    void EffsStart(char *deviceNamePtr);

    /*
    ******************************************************************************

    Lists files and directories starting with the current directory

    Parameters:
    deviceNamePtr        - Device name

    Return:
    None

    Notes:
    None

    ******************************************************************************
    */
    void EffsListCurrentDirectory(char *deviceNamePtr);

    /*
    ******************************************************************************

    Display space used, total and bad

    Parameters:
    deviceNamePtr        - Device name

    Return:
    None

    Notes:
    None

    ******************************************************************************
    */
    void EffsDisplayStatistics(char *deviceNamePtr);

    /* Format the EFFS Flash file system */
    uint8_t EffsFormat();

#ifdef __cplusplus
};
#endif

#endif /* #ifndef _EFFSSTD_H_ */
