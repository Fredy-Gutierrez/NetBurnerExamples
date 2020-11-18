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


/*-------------------------------------------------------------------
 * EFFS-STD configuration file for MicroChip SAME70Q21 Flash Chip.
 * This file is part of an example that allocates 512K of flash space
 * to the file system, and the rest to the application.
 *-----------------------------------------------------------------*/

#ifndef _ONCHIPFLASH_H_
#define _ONCHIPFLASH_H_

#include "file/fsf.h"
#include "basictypes.h"
#include "hal.h"


#define FLASH_NAME   "SAME70Q21"

/*functions implemented*/
extern int fs_phy_OnChipFlash( FS_FLASH *flash );


// Start of Flash memory base address
#define FS_FLASHBASE (0x00400000)

/*
 * BLOCKSIZE
 * This defines the size of the blocks to be used in the file storage area.
 * This must be an erasable unit of the flash chip. All blocks in the file
 * storage area must be the same size. This maybe different from the DESCSIZE
 * where the flash chip has different size erasable units available.
 *
 * SECTORSIZE
 * This defines the sector size. Each block is divided into a number of sectors.
 * This number is the smallest usable unit in the system and thus represents the
 * minimum file storage area. For best usage of the flash blocks the sector size
 * should always be a power of 2. For more information see sector section below.
 *
 * SECTORPERBLOCK
 * This defines the number of sectors in a block. It must always be true that:
 * SECTORPERBLOCK = BLOCKSIZE/SECTORSIZE
 *
 *
 * The memory map below is for a MODM7AE70 with a 2MB bottom boot block flash.
 * This example will allocate 1.5MB for the application space, and 512KB for
 * the file system.
 *
 * Microchip SAME70Q21:
 * The memory is organized in sectors. Each sector has a size of 128 Kbytes. The 
 * first sector is divided into 3 smaller sectors. The three smaller sectors are 
 * organized in 2 sectors of 8 Kbytes and 1 sector of 112 Kbytes. 
 * Total number of bytes for storage: 2,097,152. Applicatin space: 1.97MB max.
 *
 *                             Address
 *                            ----------
 *    ---------------------- 005F FFFF (End of flash space)
 *    |  File System Data  |
 *    |  256KB             |
 *    |  128K x 2 Blocks   |
 *    |--------------------| 005C 0000 (Start of File System Data)
 *    |  DESC BLOCK 0/1    |
 *    |  128K x 2 Blocks   |
 *    |--------------------| 0058 0000 (Start of File System)
 *    |                    |
 *    | Application        |
 *    | 1.375MB            |
 *    | 128K x 11 Blocks   |
 *    |                    |
 *    |--------------------| 0040 6004
 *    |   8K User Params   |
 *    |--------------------| 0040 4000
 *    |  10K Configuration |
 *    |--------------------| 0040 1800
 *    |   6K Monitor       |
 *    |--------------------| 0040 0000 (Start of Flash space)
 *
 *
 * CHANGES TO COMPCODE FLAGS
 * In NBEclipse, or your command line makefile, change the following line
 * so the application will only occupy the specified application space.
 * The first parameter is the start of application space, and the second
 * is the address just below the file system space.
 *
 *    COMPCODEFLAGS = 0x00406004 0x005A0000
 *
 * If using NBEclipse:
 * - Right-click on the project and select "Properties"
 * - Select "NetBurner" in the left side of the dialog box
 * - Verify the Platform is set to Mod5234, then check the "Use Custom Platform Settings" checkbox
 * - Modify the "Compcode Memory Range" to the above values
 *
 *
 * If using NBEclipse, you will also need to tell the linker to include the
 * /nburn/platform/<platform>/original/lib/libStdFFile.a library. To do this right-click on your
 * project, select properties, GNU Linker, then add the library.
 *
 */


/* WARNING: These settings are for MX29GL256F bottom boot block flash
 * components used on the Mod54415
 */
#define BLOCKSIZE  ( 16*512 )    // flash physical "sector" size
#define SECTORSIZE (    512 )    // file system sectors per BLOCK
#define SECTORPERBLOCK (BLOCKSIZE/SECTORSIZE)

/*
 * Specify the total amount of flash memory in the system, and the amount
 * allocated to be used by the file system (the rest is used by the
 * application.
 */
#define FLASH_SIZE ( 2*1024*1024 )        // size of total flash in the system, 32MB
#define FS_SIZE    ( 256*512 )            // amount allocated to file system: 2 Desc. plus 1MB for data (8 x 128k)
                                          // note that 1 block of file data will be reserved for the file system
#define FIRST_ADDR (FLASH_SIZE - FS_SIZE) // first file system address to use in the flash
#define BLOCKSTART 2                      // first block where file system data starts
                                          // (first 2 blocks are DESCRIPTORS)


/*
 * Descriptor Blocks:
 * These blocks contain critical information about the file system, block allocation,
 * wear information and file/directory information. At least two descriptor blocks
 * must be included in the system, which can be erased independently. An optional
 * descriptor write cache may be configured which improves the performance of the
 * file system. Please refer to the EFFS-STD implementation guide for additional
 * information.
 */
#define DESCSIZE ( 16*512 )        // size of one descriptor
#define DESCBLOCKSTART 0           // position of first descriptor
#define DESCBLOCKEND 1             // position of last descriptor
#define DESCCACHE 1024

#endif /* _ONCHIPFLASH_H_ */



