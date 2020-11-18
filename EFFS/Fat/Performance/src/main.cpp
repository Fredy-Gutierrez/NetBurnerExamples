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

#include <effs_fat/fat.h>
#include <init.h>
#include <stopwatch.h>
#include <system.h>

#include "FileSystemUtils.h"

#if (defined(USE_MMC) && defined(MOD5441X))
#define MULTI_MMC true   // For modules with onboard flash sockets, even if you are using external flash cards
#include <effs_fat/multi_drive_mmc_mcf.h>
#elif (defined(USE_MMC))
#include <effs_fat/mmc_mcf.h>
#elif (defined(USE_CFC))
#include <effs_fat/cfc_mcf.h>
#endif

#define CREATE_FILES_NUM 1000

const char *AppName = "EFFS FAT Tests";

/**
 *  testCreateSingleFile
 *
 *  Create a single file
 */
double testCreateSingleFile(const char *fileNameBase, const char *textFileData)
{
    StopWatch stopWatch;
    double totalTime = 0.0;
    char fileName[80];
    static int fileCounter;

    long dataLen = strlen(textFileData);
    sniprintf(fileName, 80, "%s%d", fileNameBase, fileCounter++);

    stopWatch.Start();
    WriteFile((uint8_t *)textFileData, fileName, dataLen);
    stopWatch.Stop();

    totalTime = stopWatch.Convert(stopWatch.GetTime());
    printf("Time: %g seconds\r\n", totalTime);
    stopWatch.Clear();
    return totalTime;
}

/**
 *  testCreateFiles
 *
 *  Create as many files as possible in the specified time period.
 */
unsigned int testCreateFiles(uint16_t numFiles, const char *fileNameBase, const char *textFileData)
{
    double TotalTime = 0;
    uint16_t filesCreated = 0;
    char fileName[80];

    if (numFiles > 65534)
    {
        iprintf("Error: Max number of files is 65534\r\n");
        return -1;
    }

    for (unsigned int i = 0; i < numFiles; i++)
    {
        sniprintf(fileName, 80, "%s%d ", fileNameBase, i);
        iprintf("%s", fileName);
        TotalTime += testCreateSingleFile(fileNameBase, textFileData);
    }

    printf("The total time elapsed was %g seconds\r\n", TotalTime);
    iprintf("Files created: %d\r\n", numFiles);
    printf("Average time to create a file: %g\r\n", TotalTime / (double)numFiles);

    return filesCreated;
}

/**
 *  testWriteSpeed
 *
 *  Write speed test
 */
void testWriteSpeed(uint8_t *pDataToWrite, const char *pFileName, long numBytes)
{
    StopWatch stopWatch;
    double totalTime = 0.0;
    long bytesWritten = 0;

    iprintf("Starting write speed test\r\n");

    if (numBytes < 0)
    {
        iprintf("Error: number of bytes less than 0\r\n");
        return;
    }

    uint32_t rvC;

    F_FILE *fp = f_open(pFileName, "w+");
    if (fp)
    {
        stopWatch.Start();
        bytesWritten = f_write(pDataToWrite, 1, numBytes, fp);
        stopWatch.Stop();

        if (bytesWritten != numBytes)
        { iprintf("\r\n*** Error in f_write(%s): %ld out of %ld bytes written\r\n", pFileName, bytesWritten, numBytes); }

        rvC = f_close(fp);   // Close a previously opened file of type F_FILE
        if (rvC != F_NO_ERROR)
        {
            DisplayEffsErrorCode(rvC);
            return;
        }
    }
    else
    {
        iprintf("Could not open file\r\n");
        return;
    }

    totalTime = stopWatch.Convert(stopWatch.GetTime());
    printf("The total time elapsed to write %ld bytes was %g seconds\r\n", numBytes, totalTime);
    printf("Write speed: %g MBps\r\n", (double)numBytes / totalTime / 1000000.0);
    stopWatch.Clear();
}

/**
 *  testWriteSpeedMultiple
 *
 *  Write speed test with multiple writes
 */
void testWriteSpeedMultiple(uint8_t *pDataToWrite, const char *pFileName, long totalBytes)
{
    StopWatch stopWatch;
    double totalTime = 0.0;
    long bytesWritten = 0;

    iprintf("Starting write speed test with multiple writes\r\n");

    if (totalBytes < 0)
    {
        iprintf("Error: number of bytes less than 0\r\n");
        return;
    }

    F_FILE *fp = f_open(pFileName, "w+");
    if (fp)
    {
        long bytesPerWrite = totalBytes / 10;

        stopWatch.Start();
        for (int i = 0; i < 10; i++)
        {
            bytesWritten += f_write(pDataToWrite, 1, bytesPerWrite, fp);
        }
        stopWatch.Stop();

        if (bytesWritten != totalBytes)
        { iprintf("\r\n*** Error in f_write(%s): %ld out of %ld bytes written\r\n", pFileName, bytesWritten, totalBytes); }

        uint32_t rvC = f_close(fp);   // Close a previously opened file of type F_FILE
        if (rvC != F_NO_ERROR)
        {
            DisplayEffsErrorCode(rvC);
            return;
        }
    }
    else
    {
        iprintf("Cound not open file\r\n");
        return;
    }

    totalTime = stopWatch.Convert(stopWatch.GetTime());
    printf("The total time elapsed for 10 writes, %ld bytes total, was %g seconds\r\n", totalBytes, totalTime);
    printf("Write speed: %g MBps\r\n", (double)totalBytes / totalTime / 1000000.0);
    stopWatch.Clear();
}

/**
 *  testReadSpeed
 *
 *  Read speed test
 */
void testReadSpeed(uint8_t *pDataStore, const char *pFileName, long numBytes)
{
    StopWatch stopWatch;
    double totalTime = 0.0;
    long bytesRead = 0;

    iprintf("Starting read speed test (write test must be run once first)\r\n");

    if (numBytes < 0)
    {
        iprintf("Error: number of bytes less than 0\r\n");
        return;
    }

    F_FILE *fp = f_open(pFileName, "r");
    if (fp)
    {
        stopWatch.Start();
        bytesRead = f_read(pDataStore, 1, numBytes, fp);
        stopWatch.Stop();

        if (bytesRead != numBytes)
        { iprintf("\r\n*** Error in f_read(%s): %ld out of %ld bytes read\r\n", pFileName, bytesRead, numBytes); }

        uint32_t rvC = f_close(fp);   // Close a previously opened file of type F_FILE
        if (rvC != F_NO_ERROR)
        {
            DisplayEffsErrorCode(rvC);
            return;
        }
    }
    else
    {
        iprintf("Could not open file\r\n");
        return;
    }

    totalTime = stopWatch.Convert(stopWatch.GetTime());
    printf("The total time elapsed was %g seconds\r\n", totalTime);
    printf("Read speed: %g MBps\r\n", (double)numBytes / totalTime / 1000000.0);
    stopWatch.Clear();
}

/**
 *  UserMain
 *
 *  Main entry point for the example
 */
void UserMain(void *pd)
{
    const int BUFFER_SIZE = 1000000;
    static uint8_t buffer[BUFFER_SIZE];

    init();                                       // Initialize network stack
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

    StopWatch stopWatch;
    stopWatch.Start();
    iprintf("Application: %s\r\nNNDK Revision: %s\r\n", AppName, GetReleaseTag());
    stopWatch.Stop();
    double totalTime = stopWatch.Convert(stopWatch.GetTime());
    printf("The total time elapsed was %g seconds\r\n", totalTime);

    /* The following call to f_enterFS() must be called in every task that accesses
    the file system.  This must only be called once in each task and must be done before
    any other file system functions are used.  Up to 10 tasks can be assigned to use
    the file system. Any task may also be removed from accessing the file system with a
    call to the function f_releaseFS(). */
    f_enterFS();
    InitExtFlash();            // Initialize the CFC or SD/MMC external flash drive
    DisplayEffsSpaceStats();   // Display file space usage
    // DumpDir();                // Display flash card files and directories

    while (1)
    {
        iprintf("\r\n\r\n---------------------------------\r\n\r\n");
        iprintf("B - Create a single file\r\n");
        iprintf("C - Create files test (%d files)\r\n", CREATE_FILES_NUM);
        iprintf("D - Delete all files\r\n");
        iprintf("L - List files\r\n");
        iprintf("F - Format drive (takes a long time)\r\n");
        iprintf("R - Read speed (%d bytes)\r\n", BUFFER_SIZE);
        iprintf("S - Drive statistics\r\n");
        iprintf("W - Write speed (%d bytes)\r\n", BUFFER_SIZE);
        iprintf("> ");

        char c = getchar();
        iprintf("\r\n");

        switch (toupper(c))
        {
            case 'B':
            {
                testCreateSingleFile("B", "Hello World");
                break;
            }
            case 'C':
            {
                testCreateFiles(CREATE_FILES_NUM, "C", "Hello World");
                break;
            }
            case 'D':
            {
                DeleteAllFiles();
                break;
            }
            case 'L':
            {
                iprintf("List Start -------------\r\n");
                DumpDir();
                iprintf("List End -------------\r\n");
                break;
            }
            case 'F':
            {
                FormatExtFlash();
                break;
            }
            case 'R':
            {
                for (int i = 0; i < BUFFER_SIZE; i++)
                {
                    buffer[i] = 0;
                }
                testReadSpeed(buffer, "write.bin", BUFFER_SIZE);
                break;
            }
            case 'S':
            {
                DisplayEffsSpaceStats();
                break;
            }
            case 'W':
            {
                for (int i = 0; i < BUFFER_SIZE; i++)
                {
                    buffer[i] = i;
                }
                testWriteSpeed(buffer, "write.bin", BUFFER_SIZE);
                testWriteSpeedMultiple(buffer, "writem.bin", BUFFER_SIZE);
                break;
            }
            default: { break;
            }
        }
    }
}
