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

#include <stdio.h>
#include <stdlib.h>
#include <constants.h>

#include "file/flashdrv.h"
#include "fs_main.h"

int fs_phy_OnChipFlash( FS_FLASH *flash );

/**
 *  fs_main
 *
 *  Initializes the flash file system.
 */
void fs_main( void )
{
    iprintf( "File system version:  %s\r\n", fs_getversion() );

    fs_init(); /* Initialize the file system */

    {
        long mem_size = 0;
        char *mem_ptr = nullptr;
        int rc = 0;

        iprintf( "Init NOR.\r\n" );
        
        // Gets the needed memory for the file system.
        mem_size = fs_getmem_flashdrive( fs_phy_OnChipFlash );
        if( mem_size )
        {
            mem_ptr = ( char * )malloc( mem_size );
            if( mem_ptr )
            {
                // Mounts the file system.
                rc = fd_mountstd( NOR_DRV_NUM, mem_ptr, mem_size,
                    fs_mount_flashdrive, fs_phy_OnChipFlash );

                if( rc )
                {
                    iprintf( "Formatting (%d)...", rc );

                    // If mount was not successful, then format the drive.
                    rc = fd_format( NOR_DRV_NUM, 0 );
                }

                if( rc )
                {
                    iprintf( " Failed.\r\n" );
                }
            }
            else
            {
                iprintf( " Allocation error.\r\n" );
            }
        }
        else
        {
            iprintf( " Flash drive error.\r\n" );
        }
    }

    fd_chdrive( NOR_DRV_NUM );
}
