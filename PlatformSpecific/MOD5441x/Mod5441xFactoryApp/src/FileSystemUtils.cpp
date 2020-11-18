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


#include <nbrtos.h>
#include <stdio.h>
#include <ctype.h>
#include <utils.h>
#include <effs_fat/fat.h>
#include <effs_fat/multi_drive_mmc_mcf.h>
#include <effs_fat/effs_utils.h>
#include "FileSystemUtils.h"

#define MMC_OFF_BOARD   (0)
#define MMC_ON_BOARD    (1)


extern "C"
{
    int get_cd(int drv);    // Card detection check for MMC cards
    int get_wp(int drv);    // Write protection check for MMC cards
}

char driveType[20];

char EffsErrorCode[][80] = {
  "F_NO_ERROR",                // 0
  "F_ERR_INVALIDDRIVE",        // 1
  "F_ERR_NOTFORMATTED",        // 2
  "F_ERR_INVALIDDIR",          // 3
  "F_ERR_INVALIDNAME",         // 4
  "F_ERR_NOTFOUND",            // 5
  "F_ERR_DUPLICATED",          // 6
  "F_ERR_NOMOREENTRY",         // 7
  "F_ERR_NOTOPEN",             // 8
  "F_ERR_EOF",                 // 9
  "F_ERR_RESERVED",            // 10
  "F_ERR_NOTUSEABLE",          // 11
  "F_ERR_LOCKED",              // 12
  "F_ERR_ACCESSDENIED",        // 13
  "F_ERR_NOTEMPTY",            // 14
  "F_ERR_INITFUNC",            // 15
  "F_ERR_CARDREMOVED",         // 16
  "F_ERR_ONDRIVE",             // 17
  "F_ERR_INVALIDSECTOR",       // 18
  "F_ERR_READ",                // 19
  "F_ERR_WRITE",               // 20
  "F_ERR_INVALIDMEDIA",        // 21
  "F_ERR_BUSY",                // 22
  "F_ERR_WRITEPROTECT",        // 23
  "F_ERR_INVFATTYPE",          // 24
  "F_ERR_MEDIATOOSMALL",       // 25
  "F_ERR_MEDIATOOLARGE",       // 26
  "F_ERR_NOTSUPPSECTORSIZE",   // 27
  "F_ERR_DELFUNC",             // 28
  "F_ERR_MOUNTED",             // 29
  "F_ERR_TOOLONGNAME",         // 30
  "F_ERR_NOTFORREAD",          // 31
  "F_ERR_DELFUNC",             // 32
  "F_ERR_ALLOCATION",          // 33
  "F_ERR_INVALIDPOS",          // 34
  "F_ERR_NOMORETASK",          // 35
  "F_ERR_NOTAVAILABLE",        // 36
  "F_ERR_TASKNOTFOUND",        // 37
  "F_ERR_UNUSABLE"             // 38
};


void f_open_PrintError(char *pFileName);
void f_close_PrintError(char *pFileName);


/**
 * DisplayEffsErrorCode
 */
void DisplayEffsErrorCode(int code)
{
   if (code <= MAX_EFFS_ERRORCODE) iprintf("%s\r\n", EffsErrorCode[code]);
   else iprintf("Unknown EFFS error code [%d]\r\n", code);
}


/**
 * OpenMMCFlash
 */
int OpenMMCFlash(int drv)
{
    // Card detection check
    if (get_cd(drv) == 0)
    {
        while (get_cd(drv) == 0)
        {
            if (drv == MMC_OFF_BOARD)
                iprintf("No off-board MMC/SD card detected. Insert a card and then press ");
            else
                iprintf("No on-board MMC/SD card detected. Insert a card and then press ");

            iprintf("<Enter>\r\n");

            if (getchar() == 13)
                break;
        }
    }

    // Write protection check
    if (get_wp(drv) == 1)
    {
        while (get_wp(drv) == 1)
        {
            iprintf("SD/MMC Card is write-protected. Disable write protection, then ");
            iprintf("press <Enter>\r\n");
            if (getchar() == 13)
                break;
        }
    }

    int rv;

    if (drv == MMC_OFF_BOARD)
        rv = f_mountfat(drv, mmc_initfunc, F_MMC_DRIVE0);
    else if (drv == MMC_ON_BOARD)
        rv = f_mountfat(drv, mmc_initfunc, F_MMC_DRIVE1);
    else
        rv = F_ERR_INVALIDDRIVE;

    if (rv == F_NO_ERROR)
        iprintf("FAT mount to %s successful\r\n", driveType);
    else
    {
        iprintf("FAT mount to %s failed: ", driveType);
        DisplayEffsErrorCode(rv);
        return -1;
    }

    return drv;
}

/**
 * OpenOnBoardFlash
 */
int OpenOnBoardFlash()
{
    return OpenMMCFlash(MMC_ON_BOARD);
}

/**
 * OpenOffBoardFlash
 */
int OpenOffBoardFlash()
{
    return OpenMMCFlash(MMC_OFF_BOARD);
}

/**
 * UnmountFlash
 */
int UnmountFlash(int drv)
{
    int rv;
    iprintf("Unmounting card\r\n\r\n");
    rv = f_delvolume(drv);

    if (rv != F_NO_ERROR)
    {
        iprintf("*** Error in f_delvolume(): ");
        DisplayEffsErrorCode(rv);
    }

    return rv;
}

/**
 * FormatExtFlash
 */
uint8_t FormatExtFlash(int drv, long FATtype)
{
    int rv;
    iprintf("Formatting %s card\r\n\r\n", driveType);
    rv = f_format(drv, FATtype);

    if (rv != F_NO_ERROR)
    {
        iprintf("*** Error in f_format(): ");
        DisplayEffsErrorCode(rv);
    }

    return rv;
}

/*-------------------------------------------------------------------
 DisplayEffsSpaceStats() - Display space used, total and bad
 -------------------------------------------------------------------*/
uint8_t DisplayEffsSpaceStats(int drv)
{
    F_SPACE space;
    volatile int rv;
    iprintf("Retrieving external flash usage...\r\n");
    rv = f_getfreespace(drv, &space);

    if (rv == F_NO_ERROR)
    {
        iprintf("Flash card memory usage (uint8_ts):\r\n");
        long long totalSize = space.total_high;
        totalSize = ((totalSize << 32) + space.total);

        long long freeSize = space.free_high;
        freeSize = ((freeSize << 32) + space.free);

        long long usedSize = space.used_high;
        usedSize = ((usedSize << 32) + space.used);

        long long badSize = space.bad_high;
        badSize = ((badSize << 32) + space.bad);

        iprintf("%llu total, %llu free, %llu used, %llu bad\r\n", totalSize, freeSize, usedSize, badSize);
        //iprintf( "%lu total, %lu free, %lu used, %lu bad\r\n", space.total, space.free, space.used, space.bad );
    }
    else
    {
        iprintf("\r\n*** Error in f_getfreepace(): ");
        DisplayEffsErrorCode(rv);
    }

    return rv;
}

/*-------------------------------------------------------------------
 DumpDir() - Displays a list of all directories and files in the
 file system.
 -------------------------------------------------------------------*/
uint8_t DumpDir()
{
    F_FIND finder;    // location to store the information retreived

    /* Find first file or subdirectory in specified directory. First call the
     f_findfirst function, and if file was found get the next file with
     f_findnext function. Files with the system attribute set will be ignored.
     Note: If f_findfirst() is called with "*.*" and the current directory is
     not the root directory, the first entry found will be "." - the current
     directory.
     */
    volatile int rc = f_findfirst("*.*", &finder);
    if (rc == F_NO_ERROR)  // found a file or directory
    {
        do
        {
            if ((finder.attr & F_ATTR_DIR))
            {
                iprintf("Found Directory [%s]\r\n", finder.filename);

                if (finder.filename[0] != '.')
                {
                    f_chdir(finder.filename);
                    DumpDir();
                    f_chdir("..");
                }
            }
            else
            {
                iprintf("Found File [%s] : %ld uint8_ts\r\n", finder.filename, finder.filesize);
            }
        } while (!f_findnext(&finder));
    }

    return rc;
}

/*
 typedef struct
 {
 char filename[FN_MAXPATH];  // file name+ext
 char name[F_MAXNAME];       // file name
 char ext[F_MAXEXT];         // file extension
 char attr;                     // attribute of the file

 unsigned short ctime;       // creation time
 unsigned short cdate;       // creation date
 unsigned long filesize;     // length of file

 unsigned long cluster;      // current file starting position
 F_NAME findfsname;         // find properties
 F_POS pos;                     // position of the current list
 } FN_FIND;
 */

/*-------------------------------------------------------------------
 uint8_t DeleteFile( char* );  - Deletes File and prints status, returns 0 for success
 -------------------------------------------------------------------*/
uint8_t DeleteFile(char* pFileName)
{
    volatile int rv;
    rv = f_delete(pFileName);
    if (rv != F_NO_ERROR)
    {
        if (OSTaskName() != NULL)
            iprintf("\r\n*** Error in f_delete( %s ) during task(%s)\r\n", pFileName, OSTaskName());
        else
            iprintf("\r\n*** Error in f_delete( %s ) during task(%d)\r\n", pFileName, OSTaskID());
        DisplayEffsErrorCode(rv);
    }
    return rv;
}

/*-------------------------------------------------------------------
 uint32_t WriteFile( uint8_t* pDataToWrite, char* pFileName, uint32_t Numuint8_ts );
 - Open, Writes, Closes File and prints status, returns 0 for success

 -------------------------------------------------------------------*/
uint32_t WriteFile(uint8_t* pDataToWrite, char* pFileName, uint32_t Numuint8_ts)
{
    uint32_t rvW, rvC;
    F_FILE* fp = f_open(pFileName, "w+");
    if (fp)
    {
        rvW = f_write(pDataToWrite, 1, Numuint8_ts, fp);
        if (rvW != Numuint8_ts)
            iprintf("\r\n*** Error in f_write(%s): %ld out of %ld uint8_ts writte\r\n", pFileName, rvW, Numuint8_ts);

        rvC = f_close(fp);  // Close a previously opened file of type F_FILE
        if (rvC != F_NO_ERROR)
        {
            f_close_PrintError(pFileName);
            DisplayEffsErrorCode(rvC);
            return 0;
        }
    }
    else
    {
        f_open_PrintError(pFileName);
        return 0;
    }

    return rvW;
}

/*-------------------------------------------------------------------
 uint32_t AppendFile( uint8_t* pDataToWrite, char* pFileName, uint32_t Numuint8_ts );
 - Open, Writes stating at end of data in file, Closes File and prints status, returns 0 for success

 -------------------------------------------------------------------*/
uint32_t AppendFile(uint8_t* pDataToWrite, char* pFileName, uint32_t Numuint8_ts)
{
    uint32_t rvA, rvC;
    F_FILE* fp = f_open(pFileName, "a+");
    if (fp)
    {
        rvA = f_write(pDataToWrite, 1, Numuint8_ts, fp);
        if (rvA != Numuint8_ts)
            iprintf("\r\n*** Error in f_write(%s): %ld out of %ld uint8_ts written\r\n", pFileName, rvA, Numuint8_ts);

        rvC = f_close(fp);  // Close a previously opened file of type F_FILE
        if (rvC != F_NO_ERROR)
        {
            f_close_PrintError(pFileName);
            DisplayEffsErrorCode(rvC);
            return 0;
        }
    }
    else
    {
        f_open_PrintError(pFileName);
        return 0;
    }

    return rvA;
}

/*-------------------------------------------------------------------
 uint32_t ReadFile( uint8_t* pReadBuffer, char* pFileName, uint32_t Numuint8_ts );
 - Open, Writes, Closes File and prints status, returns 0 for success

 -------------------------------------------------------------------*/
uint32_t ReadFile(uint8_t* pReadBuffer, char* pFileName, uint32_t Numuint8_ts)
{
    uint32_t rvR, rvC;
    F_FILE* fp = f_open(pFileName, "r");
    if (fp)
    {
        rvR = (uint32_t) f_read(pReadBuffer, 1, Numuint8_ts, fp);
        //if( Numuint8_ts != rvR )
        //    iprintf( "*** Warning in ReadFile(%s): %lu out of %lu uint8_ts read\r\n", pFileName, rvR, Numuint8_ts );

        rvC = f_close(fp);  // Close a previously opened file of type F_FILE
        if (rvC != F_NO_ERROR)
        {
            f_close_PrintError(pFileName);
            DisplayEffsErrorCode(rvC);
            return 0;
        }
    }
    else
    {
        f_open_PrintError(pFileName);
        return 0;
    }
    return rvR;
}

/*-------------------------------------------------------------------
 ReadWriteTest() - This function will read and write files/data to
 demonstrate basic file system operation.
 -------------------------------------------------------------------*/
void ReadWriteTest(char *FileName)
{
    /* Create a test file
     The f_open() function opens a file for reading or writing. The following
     modes are allowed to open:
     "r"   Open existing file for reading. The stream is positioned at the
     beginning of the file.
     "r+"  Open existing file for reading and writing. The stream is
     positioned at the beginning of the file.
     "w"   Truncate file to zero length or create file for writing. The
     stream is positioned at the beginning of the file.
     "w+"  Open a file for reading and writing. The file is created if it
     does not exist, otherwise it is truncated. The stream is
     positioned at the beginning of the file.
     "a"   Open for appending (writing to end of file). The file is created
     if it does not exist. The stream is positioned at the end of the
     file.
     "a+"  Open for reading and appending (writing to end of file). The file
     is created if it does not exist. The stream is positioned at the
     end of the file.
     Note: There is no text mode. The system assumes all files to be accessed in
     binary mode only.
     */
    iprintf("\r\nCreating test file: %s\r\n", FileName);
    F_FILE* fp = f_open(FileName, "w+");
    if (fp)
    {
        for (int i = 0; i < 5; i++)
        {
#define WRITE_BUFSIZE 80
            char write_buf[WRITE_BUFSIZE];
            sniprintf(write_buf, WRITE_BUFSIZE, "Hello World %d\r\n", i);
            /* f_write( const void *buffer,  // pointer to data to be written
             long size,           // size of items to be written
             long size size_st,   // number of items to be written
             F_FILE )             // handle of target file

             Returns the number of items written.
             */
            int n = f_write(write_buf, 1, strlen(write_buf), fp);
            iprintf("Wrote %d uint8_ts: %s", n, write_buf);
        }

        // Read the data in the test file
        iprintf("\r\nRewinding file\r\n");
        int rv = f_rewind(fp);   // set current file pointer to start of file
        if (rv != F_NO_ERROR)
        {
            f_close_PrintError(FileName);
            DisplayEffsErrorCode(rv);
        }
        else
        {
            while (!f_eof(fp))
            {
                /* Read uint8_ts from the current position in the target file. File has
                 to be opened with “r”, "r+", "w+" or "a+".
                 f_read ( const void *buffer,  // pointer to buffer to store data
                 long size,           // size of items to be read
                 long size size_st,   // number of items to be read
                 F_FILE )             // handle of target file
                 Returns the number of items read.
                 */
#define READ_BUFSIZE 80
                char read_buf[READ_BUFSIZE];
                int n = f_read(read_buf, 1, READ_BUFSIZE - 1, fp);
                read_buf[n] = '\0';  // terminate string
                iprintf("Read %d uint8_ts:\r\n%s\r\n", n, read_buf);
            }

            iprintf("Closing file %s\r\n\r\n", FileName);
            rv = f_close(fp);  // Close a previously opened file of type F_FILE
            if (rv != F_NO_ERROR)
            {
                f_close_PrintError(FileName);
                DisplayEffsErrorCode(rv);
            }
        }
    }
    else
    {
        f_open_PrintError(FileName);
    }
}

/*-------------------------------------------------------------------
 fgets_test
 ------------------------------------------------------------------*/
void fgets_test(char *FileName)
{
    iprintf("\r\nOpening test file for reading: %s\r\n", FileName);
    F_FILE* fp = f_open(FileName, "r");
    if (fp)
    {
        iprintf("Calling fgets() until end of file\r\n");
        char buf[128];
        while (!f_eof(fp))
        {
            char *buf_rtn = f_fgets(buf, 128, fp);
            if (buf_rtn != NULL)
            {
                iprintf("fgets() returned: \"");
                for (int i = 0; i < (int) strlen(buf); i++)
                {
                    if (isprint(buf[i]))
                        iprintf("%c", buf[i]);
                    else
                        iprintf("<0x%X>", buf[i]);
                }
                iprintf("\"\r\n");
            }
            else
                iprintf("End of file\r\n");
        }
    }
    else
    {
        f_open_PrintError(FileName);
    }

    iprintf("Closing file %s\r\n\r\n", FileName);
    int rv = f_close(fp);  // Close a previously opened file of type F_FILE
    if (rv != F_NO_ERROR)
    {
        f_close_PrintError(FileName);
        DisplayEffsErrorCode(rv);
    }
}

/*-------------------------------------------------------------------
 Display a text file
 ------------------------------------------------------------------*/
void DisplayTextFile(char *FileName)
{
    iprintf("\r\nOpening test file for reading: %s\r\n", FileName);
    F_FILE* fp = f_open(FileName, "r");
    if (fp)
    {
        while (!f_eof(fp))
        {
            /* Read uint8_ts from the current position in the target file. File has
             to be opened with “r”, "r+", "w+" or "a+".
             f_read ( const void *buffer,  // pointer to buffer to store data
             long size,           // size of items to be read
             long size size_st,   // number of items to be read
             F_FILE )             // handle of target file
             Returns the number of items read.
             */
#define DISP_READ_BUFSIZE 255
            char read_buf[DISP_READ_BUFSIZE];
            int n = f_read(read_buf, 1, DISP_READ_BUFSIZE - 1, fp);
            read_buf[n] = '\0';  // terminate string
            iprintf("Read %d uint8_ts:\r\n%s\r\n", n, read_buf);
        }

        iprintf("Closing file %s\r\n\r\n", FileName);
        int rv = f_close(fp);  // Close a previously opened file of type F_FILE
        if (rv != F_NO_ERROR)
        {
            f_close_PrintError(FileName);
            DisplayEffsErrorCode(rv);
        }
    }
    else
    {
        f_open_PrintError(FileName);
    }
}

/*-------------------------------------------------------------------
 fprintf_test
 -------------------------------------------------------------------*/
void fprintf_test()
{
    /* Open the test file
     The f_open() function opens a file for reading or writing. The following
     modes are allowed to open:
     "r"   Open existing file for reading. The stream is positioned at the
     beginning of the file.
     "r+"  Open existing file for reading and writing. The stream is
     positioned at the beginning of the file.
     "w"   Truncate file to zero length or create file for writing. The
     stream is positioned at the beginning of the file.
     "w+"  Open a file for reading and writing. The file is created if it
     does not exist, otherwise it is truncated. The stream is
     positioned at the beginning of the file.
     "a"   Open for appending (writing to end of file). The file is created
     if it does not exist. The stream is positioned at the end of the
     file.
     "a+"  Open for reading and appending (writing to end of file). The file
     is created if it does not exist. The stream is positioned at the
     end of the file.
     Note: There is no text mode. The system assumes all files to be accessed in
     binary mode only.
     */

    char FileName[] = "TestFile.txt";  // 8.3 file names supported by default
    static uint16_t WriteCount;

    iprintf("\r\nOpening test file for appending: %s\r\n", FileName);
    F_FILE* fp = f_open(FileName, "a");
    if (fp)
    {
        f_fprintf(fp, "Write #%u, Secs = %lu, Secs = 0x%lX\r\n", WriteCount, Secs, Secs);
        int rv = f_close(fp);
        if (rv != F_NO_ERROR)
        {
            f_close_PrintError(FileName);
            DisplayEffsErrorCode(rv);
        }

        iprintf("Wrote to file: \"Write #%u, Secs = %lu, Secs = 0x%lX\"\r\n", WriteCount, Secs, Secs);
        WriteCount++;
    }
    else
    {
        f_open_PrintError(FileName);
    }

    DisplayTextFile(FileName);
}

/*-------------------------------------------------------------------
 fputs_test
 -------------------------------------------------------------------*/
void fputs_test(char *FileName)
{
    /* Open the test file
     The f_open() function opens a file for reading or writing. The following
     modes are allowed to open:
     "r"   Open existing file for reading. The stream is positioned at the
     beginning of the file.
     "r+"  Open existing file for reading and writing. The stream is
     positioned at the beginning of the file.
     "w"   Truncate file to zero length or create file for writing. The
     stream is positioned at the beginning of the file.
     "w+"  Open a file for reading and writing. The file is created if it
     does not exist, otherwise it is truncated. The stream is
     positioned at the beginning of the file.
     "a"   Open for appending (writing to end of file). The file is created
     if it does not exist. The stream is positioned at the end of the
     file.
     "a+"  Open for reading and appending (writing to end of file). The file
     is created if it does not exist. The stream is positioned at the
     end of the file.
     Note: There is no text mode. The system assumes all files to be accessed in
     binary mode only.
     */

    iprintf("\r\nOpening test file for appending: %s\r\n", FileName);
    F_FILE* fp = f_open(FileName, "a");
    if (fp)
    {
        char s[128];
        sniprintf(s, 128, "f_fputs() executed at %ld seconds\r\n", Secs);
        int n = f_fputs(s, fp);

        int rv = f_close(fp);
        if (rv != F_NO_ERROR)
        {
            f_close_PrintError(FileName);
            DisplayEffsErrorCode(rv);
        }

        iprintf("Wrote %d bytes to file: \"%s\"\r\n", n, s);
    }
    else
    {
        f_open_PrintError(FileName);
    }

    DisplayTextFile(FileName);
}

void f_open_PrintError(char* pFileName)
{
    if (OSTaskName() != NULL)
        iprintf("*** Error in f_open(%s) during task(%s)\r\n", pFileName, OSTaskName());
    else
        iprintf("*** Error in f_open(%s) during task(%d)\r\n", pFileName, OSTaskID());

    int rv = f_getlasterror();
    DisplayEffsErrorCode(rv);
}

void f_close_PrintError(char* pFileName)
{
    if (OSTaskName() != NULL)
        iprintf("*** Error in f_close(%s) during task(%s)\r\n", pFileName, OSTaskName());
    else
        iprintf("*** Error in f_close(%s) during task(%d)\r\n", pFileName, OSTaskID());
}

