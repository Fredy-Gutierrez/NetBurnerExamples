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


/*------------------------------------------------------------------------------
 * EFFS-STD configuration file for Spansion AM29LV160B flash chip.  This file is
 * part of an example that allocates 512kB of flash space to the file system,
 * and the rest to the application.
 *
 * Note:  The AM29LV160B is identical to the Spansion S29AL016D.
 *
 * To modify the amount of space allocated to the file system:
 *
 * 1. Change the definition in this file:  #define FS_SIZE ( 1024 * 1024 )
 * 2. Change the compcode memory address range for the application in your
 *    NBEclipse project settings so that the end of the application space does
 *    not exceed the start of the file system space.  See the EFFS Programmer's
 *    Guide for details, and the header comments in main.cpp of this example
 *    on how to make the changes in the NBEclipse project.
 * 3. Be sure to add the /nburn/platform/<platform>/original/lib/libStdFFile.a library
 *    to your NBEclipse project C/C++ build linker library options.  See the header
 *    comments in main.cpp for this example on how to add the library in the
 *    NBEclipse project.
 *----------------------------------------------------------------------------*/

#ifndef _ONCHIPFLASH_H_
#define _ONCHIPFLASH_H_

#include "file/fsf.h"
#include "basictypes.h"
#include "hal.h"


/**
 * Function implemented.
 */
extern int fs_phy_OnChipFlash( FS_FLASH *flash );


#define FLASH_NAME   "S29AL016/AMDLV160"


/**
 * Start of flash memory base address.
 */
#define FS_FLASHBASE   ( 0xFFC00000 )


/**
 * BLOCKSIZE
 * This defines the size of the blocks to be used in the file storage area.
 * This must be an erasable unit of the flash chip.  All blocks in the file
 * storage area must be the same size.  This may be different from the DESCSIZE
 * where the flash chip has different size erasable units available.
 *
 * SECTORSIZE
 * This defines the sector size.  Each block is divided into a number of
 * sectors.  This number is the smallest usable unit in the system and thus
 * represents the minimum file storage area.  For best usage of the flash
 * blocks, the sector size should always be a power of two.  See sector section
 * below for more information.
 *
 * SECTORPERBLOCK
 * This defines the number of sectors in a block.  It must always be true that:
 *
 *    SECTORPERBLOCK = BLOCKSIZE / SECTORSIZE
 *
 * The memory map below is for a module with a 2MB bottom boot block flash.
 * This example will allocate 1.5MB for the application space, and 512kB for the
 * file system.  There are a total of 31 64kB blocks and 8 8kB blocks on the
 * flash chip.
 *
 *                            Address
 *                            -------
 *    ----------------------  FFE0 0000 (End of flash space)
 *    |  File System Data  |
 *    |  384k              |
 *    |  64k x 6 Blocks    |
 *    |--------------------|  FFDA 0000 (Start of file system data)
 *    |  DESC BLOCK 0/1    |
 *    |  128k              |
 *    |  64k x 2 Blocks    |
 *    |--------------------|  FFD8 0000 (Start of file system)
 *    |                    |
 *    |  1.5M Application  |
 *    |  64k x 23 Blocks   |
 *    |                    |
 *    |--------------------|
 *    |  32k Application   |
 *    |  8k x 4 Blocks     |
 *    |--------------------|  FFC0 8000 (Start of application space)
 *    |  8k User Params    |
 *    |  8k x 1 Block      |
 *    |--------------------|
 *    |  8k System Params  |
 *    |  8k x 1 Block      |
 *    |--------------------|
 *    |  16k Monitor       |
 *    |  8k x 2 Blocks     |
 *    |--------------------|  FFC0 0000 (Start of flash space)
 */


/**
 * Warning:  These settings are for AM29LV160DB/S29AL016D bottom boot block
 * flash components.
 */
#define BLOCKSIZE        (              64 * 1024 )  // Use only the 64k sectors
#define SECTORSIZE       (              16 * 1024 )  // 4 sectors per block
#define SECTORPERBLOCK   ( BLOCKSIZE / SECTORSIZE )


/*
 * Specify the total amount of flash memory in the system, and the amount
 * allocated to be used by the file system (the rest is used by the
 * application).
 */
#define FLASH_SIZE   (      2 * 1024 * 1024 )   // Size of total flash in the
                                                //    system, 2MB
#define FS_SIZE    (           512 * 1024 )   // Amount allocated to file
                                                //    system, 512kB
//#define FS_SIZE      (          1024 * 1024 )   // Amount allocated to file
                                                //    system, 1MB
#define FIRST_ADDR   ( FLASH_SIZE - FS_SIZE )   // First file system address to
                                                //    use in the flash
#define BLOCKSTART   (                    2 )   // First block where file system
                                                //    data starts (first 2
                                                //    blocks are DESCRIPTORS)


/*
 * Descriptor Blocks:
 * These blocks contain critical information about the file system, block
 * allocation, wear information, and file/directory information.  At least two
 * descriptor blocks must be included in the system, which can be erased
 * independently.  An optional descriptor write cache may be configured which
 * improves the performance of the file system.  Please refer to the EFFS-STD
 * implementation guide for additional information.
 */
#define DESCSIZE         ( 8 * 1024 )   // Size of one descriptor
#define DESCBLOCKSTART   (        0 )   // Position of first descriptor
#define DESCBLOCKEND     (        1 )   // Position of last descriptor
#define DESCCACHE        (     2048 )


#endif /* _ONCHIPFLASH_H_ */
