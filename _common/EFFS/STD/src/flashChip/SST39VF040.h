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
 * EFFS-STD configuration file for SST39V040 flash memory chip.  This file is part
 * of an example that allocates 64KB of flash space to the file system, and the
 * rest to the application.
 *
 * To modify the amount of space allocated to the file system:
 *
 * 1. Change the definition in this file:  #define FS_SIZE ( 64 * 1024 )
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

#include "basictypes.h"
#include "hal.h"
#include "file/fsf.h"


/**
 * Function implemented.
 */
extern int fs_phy_OnChipFlash( FS_FLASH *flash );


#define FLASH_NAME   "SST39VF040"


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
 * The memory map below is for a module with a 512kB bottom boot block flash.
 * This example will allocate 448kB for the application and monitor space, and
 * 64kB for the file system.  There are a total of 128 4kB blocks on the flash
 * chip.
 *
 *                            Address
 *                            -------
 *    ----------------------  FFC8 0000 (End of flash space)
 *    |  File System Data  |
 *    |  56k               |
 *    |  4k x 14 Blocks    |
 *    |--------------------|  FFC7 2000 (Start of file system data)
 *    |  DESC BLOCKS 0/1   |
 *    |  8k                |
 *    |  4k x 2 Blocks     |
 *    |--------------------|  FFC7 0000 (Start of file system)
 *    |                    |
 *    |  Application       |
 *    |  416k Bytes        |
 *    |  4k x 104 Blocks   |
 *    |                    |
 *    |--------------------|  FFC0 8000 (Start of application space)
 *    |  8k User Params    |
 *    |  4k x 2 Blocks     |
 *    |--------------------|
 *    |  8k System Params  |
 *    |  4k x 2 Blocks     |
 *    |--------------------|
 *    |  16k Monitor       |
 *    |  4k x 4 Blocks     |
 *    |--------------------|  FFC0 0000 (Start of flash space)
 */

/**
 * Warning:  These settings are for SST 39VF040 bottom boot block flash
 * components.
 */
#define BLOCKSIZE        (               4 * 1024 )  // Use only the 64k sectors
#define SECTORSIZE       (                    512 )  // 8 sectors per block
#define SECTORPERBLOCK   ( BLOCKSIZE / SECTORSIZE )


/*
 * Specify the total amount of flash memory in the system, and the amount
 * allocated to be used by the file system (the rest is used by the
 * application).
 */
#define FLASH_SIZE   (           512 * 1024 )   // Size of total flash in the
                                                //    system, 512kB
#define FS_SIZE      (            64 * 1024 )   // Amount allocated to file
                                                //    system, 64kB
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
#define DESCSIZE         ( 4 * 1024 )   // Size of one descriptor
#define DESCBLOCKSTART   (        0 )   // Position of first descriptor
#define DESCBLOCKEND     (        1 )   // Position of last descriptor
#define DESCCACHE        (     1024 )


#endif /* _ONCHIPFLASH_H_ */
