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

#ifndef _NB_FACTORY_H_
#define _NB_FACTORY_H_

/*
 ******************************************************************************
 *
 * Factory defaults for this factory application
 *
 ******************************************************************************
 */
/* Configuration verify key (increment if data changed, added, reorganized) */
#define NB_FACTORY_VERIFY_KEY (0x7E545085)
/* STD-EFFS verify key (increment to format file system, required for 1.6-1.9 STD EFFS Upgrade) */
#define STD_EFFS_VERIFY_KEY (0x15A58101)

/* Version  VV.NN.vvvv ( 0xVVNNvvvv ), string must match constant  */
#define NB_FACTORY_DEFAULTS_VERSION (DWORD)(0x01000000)
#define NB_FACTORY_DEFAULTS_VERSION_STRING "01.00.0000"

/* Module Base Name */
#if defined MOD5441X
#define NB_FACTORY_MODULE_BASE_NAME "MOD5441X"
#elif defined NANO54415
#define NB_FACTORY_MODULE_BASE_NAME "NANO54415"
#elif defined MODM7AE70
#define NB_FACTORY_MODULE_BASE_NAME "MODM7AE70"
#else
#error Module not supported
#endif

/* Module Description */
#define NB_FACTORY_BASE_DESC "SSL/TLS"

/* Feature Name and Description */
#define NB_FACTORY_FEATURE_NAME "SX"
#define NB_FACTORY_FEATURE_DESC "Verify Peer Effs Example"

/* SSL */
#define NB_FACTORY_INCLUDE_SSL (1)

/* SSL */
#define NB_FACTORY_SSL_PERMANENT_DESC_DEFAULT "NetBurner Library Default "
#define NB_FACTORY_SSL_INCLUDED_DESC_DEFAULT "Default "
#define NB_FACTORY_SSL_INSTALLED_DESC_DEFAULT "User Installed "

#define NB_FACTORY_SSL_FILE_NAME_CERT "cert.crt"
#define NB_FACTORY_SSL_FILE_NAME_KEY "cert.key"

/* DHCP timeout */
#define NB_FACTORY_DHCP_TIMEOUT_IN_TICKS (10 * TICKS_PER_SECOND)

/*
 * Maximum size of certificate or key files in bytes
 *    Must the maximum of
 *       SERIAL_BURNER_CERTIFICATE_SIZE_MAX
 *       SERIAL_BURNER_RSA_KEY_SIZE_MAX
 *       SERIAL_BURNER_DSA_KEY_SIZE_MAX
 */
#define NB_FACTORY_FILE_SIZE_MAXIMUM (5 * 1024)

/*
 * On-chip file system EFFS-STD
 *    COMPCODEFLAGS end address must be set to file system start
 *    (FIRST_ADDR)
 */

/* Module unique flash parameters */

#if defined MOD5441X
/* Flash */
#define NB_FACTORY_FLASH_32MB_128KB_SECTORS (1)
/* Base address */
#define NB_FACTORY_FS_FLASHBASE (0xC0000000)
/* COMPCODEFLAGS = 0xC0040000 0xC1EC0000 */

#elif defined NANO54415
/* Flash */
#define NB_FACTORY_FLASH_SPI_8MB_4KB_SECTORS (1)
/* Base address */
//#define NB_FACTORY_FS_FLASHBASE              ( 0x040000 )  // not used for spi flash
/* COMPCODEFLAGS = 0x7F0000 0x040000 */

#elif defined MODM7AE70
/* Flash */
#define NB_FACTORY_FLASH_2MB_8KB_SECTORS (1)
/* Base address */
#define NB_FACTORY_FS_FLASHBASE (0x00400000)
/* COMPCODEFLAGS = 0x00406004 0x005A0000 */

#else
#error Module not supported
#endif

#endif /* #ifdef _NB_FACTORY_H_ */
