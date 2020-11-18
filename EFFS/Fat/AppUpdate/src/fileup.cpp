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

#include <hal.h>
#include <effs_fat/fat.h>
#include <malloc.h>

#include "fileup.h"

#if (defined(USE_MMC) && defined(MOD5441X))
#define MULTI_MMC true   // For modules with onboard flash sockets, even if you are using external flash cards
#include <effs_fat/multi_drive_mmc_mcf.h>
#elif (defined(USE_MMC))
#include <effs_fat/mmc_mcf.h>
#elif (defined(USE_CFC))
#include <effs_fat/cfc_mcf.h>
#endif

#ifdef NANO54415
int FlashCompare(const uint8_t *pWhere, const uint8_t *pWhat_in, uint32_t olen);
#endif

extern uint8_t user_flash_buffer[];
extern int ProcessS3(const char *cp, uint32_t base_Addr, puint8_t CopyTo, uint32_t &cur_pos, uint32_t maxlen);

extern const char PlatformName[];
extern uint32_t FlashAppBase;

char fat_read_buffer[256];
#define tmp_buffer_size (256)
char tmp_buffer[tmp_buffer_size];
int tmp_buffer_end;
int tmp_buffer_start;

/**
 *  my_f_read_line
 */
int my_f_read_line(char *buffer, int buf_siz, F_FILE *fp)
{
    int nr = 0;
    do
    {
        if (tmp_buffer_end <= tmp_buffer_start)
        {
            if (f_eof(fp)) { return 0; }

            int n = f_read(tmp_buffer, 1, tmp_buffer_size, fp);
            tmp_buffer_start = 0;
            tmp_buffer_end = n;

            if (n == 0)
            {
                buffer[nr + 1] = 0;
                return nr;
            }
        }

        *(buffer + nr) = tmp_buffer[tmp_buffer_start++];
        if ((buffer[nr] == '\r') || (buffer[nr] == '\n'))
        {
            if (nr != 0)
            {
                buffer[nr + 1] = 0;
                return nr;
            }
        }
        else
        {
            nr++;
        }
    } while (nr < buf_siz);

    buffer[nr + 1] = 0;
    return nr;
}

/**
 *  UpdateFromFat
 *
 *  This function can be used to update the application in the flash memory of a NetBurner
 *  device. The function has the ability to compare the checksum of the existing application
 *  and the flash card file to determine if an update should be performed.
 *  Parameters:
 *     fp = A pointer to a EFFS flash file type F_FILE. This file must have been opened
 *          by the code that called this function.
 *     bUpdateEvenIfCurrent = A value of 0 : do not update if checksums match,
 *                                       1 : ignore checksum result
 *
 *     Return Value: 0 = success, any other value is an error as defined in fileup.h
 */
int UpdateFromFat(F_FILE *fp, BOOL bUpdateEvenIfCurrent)
{
    int n = my_f_read_line(fat_read_buffer, 255, fp);

    if (n <= 0)
    {
        f_close(fp);
        return FAT_UPDATE_BAD_FORMAT;
    }

    if ((fat_read_buffer[0] == 'S') && (fat_read_buffer[1] == '0'))
    {
        char *cp = fat_read_buffer;
        cp += 2;
        const char *cpt = PlatformName;

        while (*cpt)
        {
            if (*cp != *cpt)
            {
                f_close(fp);
                return FAT_UPDATE_WRONG_PLATFORM;
            }
            cp++;
            cpt++;
        }
    }
    else
    {
        f_close(fp);
        return FAT_UPDATE_BAD_FORMAT;
    }

    int nlines = 0;
    uint32_t addr = 0;
    unsigned char *CopyTo = nullptr;
    uint32_t cur_pos = 0;
    uint32_t maxlen = 8192;
    CopyTo = user_flash_buffer;
    addr = (uint32_t)(&FlashAppBase);

    while (nlines < 5)
    {
        n = my_f_read_line(fat_read_buffer, 255, fp);
        if (n > 0)
        {
            if (ProcessS3(fat_read_buffer, addr, CopyTo, cur_pos, maxlen) != 0) { nlines++; }
        }
    }

    /* We have read enough lines to get the header for size */
    FlashStartUpStruct *ps = (FlashStartUpStruct *)user_flash_buffer;
    if ((ps->dwBlockRamStart + ps->dwExecutionAddr + ps->dwBlockSize + ps->dwSrcBlockSize + ps->dwBlockSum + ps->dwStructSum) != 0x4255524E)
    {
        f_close(fp);
        return FAT_UPDATE_BAD_FORMAT;
    }

    uint32_t siz = ps->dwSrcBlockSize + 28;
    puint8_t pb = (puint8_t)malloc(siz);
    if (pb == nullptr)
    {
        f_close(fp);
        return FAT_UPDATE_NO_MEM;
    }
    /* Now copy what we have */
    memcpy(pb, user_flash_buffer, cur_pos);
    CopyTo = pb;
    maxlen = siz;

    while (n > 0)
    {
        n = my_f_read_line(fat_read_buffer, 255, fp);
        if (n > 0)
        {
            if (ProcessS3(fat_read_buffer, addr, CopyTo, cur_pos, maxlen) != 0) { nlines++; }
        }
    }

    if (cur_pos == (maxlen - 4))
    {
        if (!bUpdateEvenIfCurrent)
        {
            puint8_t pbis = (uint8_t *)(&FlashAppBase);
            puint8_t psb = CopyTo;
#ifdef NANO54415
            if (FlashCompare(pbis, psb, cur_pos) == 0)
#else
            if (memcmp(pbis, psb, cur_pos) == 0)
#endif
            {
                free(CopyTo);
                f_close(fp);
                return FAT_UPDATE_SAMEVERSION;
            }
        }

        USER_ENTER_CRITICAL();
        FlashErase((void *)addr, cur_pos);
        FlashProgram((void *)addr, (void *)CopyTo, cur_pos);
        USER_EXIT_CRITICAL();

        free(CopyTo);
        f_close(fp);
        return FAT_UPDATE_OK;
    }

    free(CopyTo);
    f_close(fp);
    return FAT_UPDATE_BAD_FORMAT;
}
