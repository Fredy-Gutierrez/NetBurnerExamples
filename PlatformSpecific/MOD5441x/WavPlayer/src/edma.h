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

#ifndef  ___EDMA_H
#define  ___EDMA_H

#define TCD_ATTR_8BIT_TRANS     0x0000
#define TCD_ATTR_16BIT_TRANS    0x0101

#define TCD_XOFF_0BYTE          0x0000
#define TCD_XOFF_1BYTE          0x0001
#define TCD_XOFF_2BYTE          0x0002

#define TCD_XITER_CNT_MASK      0x7FFF

#define TCD_CSR_DONE            0x0080
#define TCD_CSR_ACTIVE          0x0040
#define TCD_CSR_MAJ_LINK        0x0020
#define TCD_CSR_EN_SG           0x0010
#define TCD_CSR_DREQ            0x0008
#define TCD_CSR_INT_HALF        0x0004
#define TCD_CSR_INT_MAJOR       0x0002
#define TCD_CSR_START           0x0001

#define TCD_CSR_DISABLE_REQ     0xC008
#define TCD_CSR_DREQ_INT_MAJOR  0xC00A

#define TCD_CITER_MAX_COUNT     0x7FFF

#endif   /* ----- #ifndef ___EDMA_H  ----- */

