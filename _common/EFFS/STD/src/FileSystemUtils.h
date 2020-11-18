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

#ifndef _FILESYSUTIL_H
#define _FILESYSUTIL_H

#include <effs_fat/fat.h>

#define MAX_EFFS_ERRORCODE (38)
extern char EffsErrorCode[][80];

#ifdef __cplusplus
extern "C"
{
#endif

    void DisplayEffsErrorCode(int code);
    uint8_t FormatEffsStdFlash();
    uint8_t DisplayEffsSpaceStats();
    uint8_t DumpDir();
    uint32_t WriteFile(uint8_t *pDataToWrite, char *pFileName, uint32_t NumBytes);
    uint32_t AppendFile(uint8_t *pDataToWrite, char *pFileName, uint32_t NumBytes);
    uint32_t ReadFile(uint8_t *pReadBuffer, char *pFileName, uint32_t NumBytes);
    uint8_t DeleteFile(char *pFileName);
    void ReadWriteTest();
    void DisplayTextFile(char *FileName);
    void fgets_test(char *FileName);
    void fprintf_test();
    void fputs_test(char *FileName);

#ifdef __cplusplus
}
#endif

#endif
