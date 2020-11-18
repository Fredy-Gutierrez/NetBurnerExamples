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

#if ( defined MOD5441X )
#include "flashChip/MX29GL256F.h"

#elif ( defined NANO54415 )
#error "*** NANO54415 not supported in generic example. Can be found in the NANO54415 examples ***"

#elif ( defined MODM7AE70 )
#include "flashChip/SAME70Q21.h"

#else
#error "*** EFFS STD FLASH PLATFORM NOT DEFINED ***"
#endif

#ifdef MODM7AE70
extern "C" uint32_t _FlashGetSize_Bytes();
uint32_t FlashGetSize_Bytes()
{
    return _FlashGetSize_Bytes() - FS_SIZE;
}
#endif

/**
 * OnChipFlash_GetFlashName
 */
void OnChipFlash_GetFlashName( char *pName )
{
    strcpy( pName, FLASH_NAME );
    return;
}


/**
 *  GetBlockAddr
 *
 *  Get a logical block physical relative address in flash
 *  relsector - relative sector in the block (<sectorperblock)
 *
 *  INPUTS
 *
 *  block - block number
 *  relsector - rel
 *
 *  RETURNS
 *
 *  relative physical address of block
 */
static long GetBlockAddr( long block, long relsector )
{
    long rc = FIRST_ADDR + block * BLOCKSIZE + relsector * SECTORSIZE;
    if( rc < FIRST_ADDR || rc > FLASH_SIZE )
    {
        rc = FIRST_ADDR;
    }
    return( rc );
}


/**
 *  EraseFlash
 *
 *  INPUTS
 *
 *  block - which block needs to be erased
 *
 *  RETURNS
 *  0 - always
 */
int EraseFlash( long block )
{
    FlashErase( ( void * )( FS_FLASHBASE + GetBlockAddr( block, 0 ) ), BLOCKSIZE );
    return 0;
}


/**
 *  WriteFlash
 *
 *  Writing (programming) Flash device
 *
 *  INPUTS
 *
 *  data - where data is
 *  block - which block is programmed
 *  relsector - relative sector in the block (<sectorperblock)
 *  size - length of data
 *  relpos - relative position of data in block
 *
 *  RETURNS
 *
 *  0 - if ok
 *  1 - if there was any error
 */
int WriteFlash( void *data, long block, long relsector, long size, long relpos )
{
    FlashProgram( ( void * )( FS_FLASHBASE + GetBlockAddr( block, relsector ) + relpos ),
        data,
        size );
    return 0;
}


/**
 *  VerifyFlash
 *
 *  Compares data with what flash contains
 *
 *  INPUTS
 *
 *  data - where data is
 *  block - which block is programmed
 *  relsector - relative sector in the block (<sectorperblock)
 *  size - length of data
 *  relpos - relative position of data in block
 *
 *  RETURNS
 *
 *  0 - if ok
 *  1 - if there was any error
 */
int VerifyFlash( void *data, long block, long relsector, long size, long relpos )
{
    unsigned short * d = ( unsigned short * )
        ( FS_FLASHBASE + GetBlockAddr( block, relsector ) + relpos );
    unsigned short * s = ( unsigned short * )( data );
    long a = 0;

    size++;

    size >>= 1;
    size <<= 1; //word align

    for( a = 0; a < size; a += 2 )  // verify
    {
        if( *d++ != *s++ )
        {
            return 1; // failed
        }
    }
    return 0; //ok
}


/**
 *  ReadFlash
 *
 *  read data from flash
 *
 *  INPUTS
 *
 *  data - where to store data
 *  block - block number which block to be read
 *  blockrel - relative start address in the block
 *  datalen - length of data in bytes
 *
 *  RETURNS
 *  0 - if successfully read
 */
int ReadFlash( void *data, long block, long blockrel, long datalen )
{
    long src = GetBlockAddr( block, 0 ) + blockrel;
    fsm_memcpy( data, ( char * )( FS_FLASHBASE + src ), datalen );
    return 0;
}


/**
 *  fs_phy_OnChipFlash
 *
 *  Identify a flash and fills FS_FLASH structure with data
 *
 *  INPUTS
 *
 *  flash - structure which is filled with data of flash properties
 *
 *  RETURNS
 *
 *  0 - if successfully
 *  1 - flash not valid
 */
int fs_phy_OnChipFlash( FS_FLASH *flash )
{
    flash->ReadFlash = ReadFlash;       //read content
    flash->EraseFlash = EraseFlash;     //erase a block
    flash->WriteFlash = WriteFlash;     //write content
    flash->VerifyFlash = VerifyFlash;   //verify content


    flash->maxblock = ( FS_SIZE / BLOCKSIZE ) - BLOCKSTART;
    flash->blocksize = BLOCKSIZE;
    flash->sectorsize = SECTORSIZE;
    flash->sectorperblock = SECTORPERBLOCK;
    flash->blockstart = BLOCKSTART;
    flash->descsize = DESCSIZE;
    flash->descblockstart = DESCBLOCKSTART;
    flash->descblockend = DESCBLOCKEND;
    flash->cacheddescsize = DESCCACHE; // write cache size in descriptor

    return 0;
}
