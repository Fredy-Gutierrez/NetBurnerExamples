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
#include <predef.h>
#include <stdio.h>
#include <ctype.h>
#include <init.h>
#include <taskmon.h>
#include <nbrtos.h>
#include <pins.h>
#include <stdlib.h>
#include "wavPlayer.h"
#ifdef WAV_PLAYER_FILESYSTEM
    #include "FileSystemUtils.h"
#endif /* #ifdef WAV_PLAYER_FILESYSTEM */

#define WAV_BUFFER_SIZE     4000000UL // 4*10^6 bytes, 4MiB
#define INPUT_BUFFER_SIZE   1024
#define CWD_BUFFER_SIZE     256

/*
 * IMPORTANT: since these files are external references, they must be aligned on an
 * even boundary. One way to do this is the aligned attribute. For example:
 * uint8_t sinewav_440[] __attribute__(( aligned( 16 ))) = { ..... }
 * You can see the full declaration in the nbwav_44k .cpp file
 */
extern uint8_t nbwav_44k[];
extern uint8_t nbwav_8k[];
extern uint8_t sinewav_440[];

uint8_t wavBuffer[WAV_BUFFER_SIZE];
char    inputBuf[INPUT_BUFFER_SIZE];
char    currDirBuf[CWD_BUFFER_SIZE];

OS_SEM  playbackSem;
bool    extFlashOpened = false;
WavPlayer testPlayer;
char demoFileName[] = "44k/nbdemo.wav";

extern "C" {
void UserMain(void * pd);
}


const char * AppName="wav_player";

#ifdef WAV_PLAYER_FILESYSTEM
static void setUpFlash()
{
    f_enterFS();
#ifdef MOD5441X
    int drv = OpenOffBoardFlash();
    int rv = f_chdrive( drv );
    if (rv == F_NO_ERROR)
    {
        printf("External Flash opened.\r\n");
        extFlashOpened = true;
    }
    else
    {
        printf("FAILED TO OPEN EXTERNAL FLASH.\r\n");
    }
#elif defined NANO54415
    InitExtFlash();
    extFlashOpened = true;
#else
#error "Platform not supported"
#endif
}
#endif /* #ifdef WAV_PLAYER_FILESYSTEM */

void ListDir()
{
    F_FIND finder;    // location to store the information retreived

    /*  Find first file or subdirectory in specified directory. First call the
        f_findfirst function, and if file was found get the next file with
        f_findnext function. Files with the system attribute set will be ignored.
        Note: If f_findfirst() is called with "*.*" and the current directory is
        not the root directory, the first entry found will be "." - the current
        directory.
    */
    volatile int rc = f_findfirst( "*.*", &finder );
    if ( rc == F_NO_ERROR )  // found a file or directory
    {
        f_getcwd( currDirBuf, CWD_BUFFER_SIZE );
        iprintf("\r\n");
        iprintf("Current Directory: %s\r\n", currDirBuf);
        do
        {
            if ( ( finder.attr & F_ATTR_DIR ) )
            {
                iprintf( "[%8s/]     d\r\n", finder.filename );
            }
            else
            {
                iprintf( "%12.12s    f  %ld\r\n", finder.filename, finder.filesize );
            }
        }
        while ( !f_findnext( &finder ) );
        iprintf("\r\n");
    }
    else {
        iprintf("\r\nError in listing directory\r\n");
    }
}

void ProcessCommand( char c )
{
    WavPlayer::wavError ret;
    int loopCount;
    char key;
    switch ( c ) {
        case '1':
            iprintf("Playing Audio...");
            testPlayer.OpenBuffer( nbwav_44k );
            testPlayer.Play( &playbackSem );
            OSSemPend( &playbackSem, 0 );
            iprintf(" Done\r\n");
            break;
        case '2':
            iprintf("Playing Audio...");
            testPlayer.OpenBuffer( nbwav_8k );
            testPlayer.Play( &playbackSem );
            OSSemPend( &playbackSem, 0 );
            iprintf(" Done\r\n");
            break;
        case '3':
            iprintf("Playing Audio...");
            testPlayer.OpenBuffer( sinewav_440 );
            testPlayer.Play( &playbackSem );
            OSSemPend( &playbackSem, 0 );
            iprintf(" Done\r\n");
            break;
        case '4':
            iprintf("Looping Audio...\r\nPress P to pause, R to resume, any other key to stop...");
            testPlayer.OpenBuffer( sinewav_440 );
            testPlayer.Loop( 0, NULL);
            while (testPlayer.GetState() != WavPlayer::STATE_FINISHED) {
                key = getchar();
                switch (key) {
                    case 'P':
                    case 'p':
                        testPlayer.Pause();
                        break;
                    case 'R':
                    case 'r':
                        testPlayer.Resume();
                        break;
                    default:
                        testPlayer.Stop();
                        break;
                }
            }
            iprintf(" Done\r\n");
            break;
#ifdef WAV_PLAYER_FILESYSTEM
        case '5':
            if (!extFlashOpened) {
                setUpFlash();
            }
            ListDir();
            break;
        case '6':
            if (!extFlashOpened) {
                setUpFlash();
            }
            iprintf("Enter directory name: ");
            gets( inputBuf );
            iprintf("\r\n");
            f_chdir( inputBuf );
            ListDir();
            break;
        case '7':
            if (!extFlashOpened) {
                setUpFlash();
            }
            iprintf("Enter file name: ");
            gets( inputBuf );
            iprintf("\r\n");
            ret = testPlayer.OpenFile( inputBuf, wavBuffer, WAV_BUFFER_SIZE );
            if ( ret == WavPlayer::ERROR_FILE_SIZE ) {
                iprintf("Wav file too big. Increase WAV_BUFFER_SIZE or use smaller file.\r\n");
                break;
            }
            iprintf("Playing Audio...");
            testPlayer.Play( &playbackSem );
            OSSemPend( &playbackSem, 0 );
            iprintf(" Done\r\n");
            break;
        case '8':
            if (!extFlashOpened) {
                setUpFlash();
            }
            iprintf("Enter file name: ");
            gets( inputBuf );
            iprintf("\r\n");
            ret = testPlayer.OpenFile( inputBuf, wavBuffer, WAV_BUFFER_SIZE );
            if ( ret == WavPlayer::ERROR_FILE_SIZE ) {
                iprintf("Wav file too big. Increase WAV_BUFFER_SIZE or use smaller file.\r\n");
                break;
            }
            iprintf("Enter loop count (0 is infinite): ");
            gets( inputBuf );
            loopCount = atoi( inputBuf );
            iprintf("\r\nLooping Audio...");
            if (loopCount) {
                testPlayer.Loop( loopCount, &playbackSem );
                OSSemPend( &playbackSem, 0 );
            }
            else {
                iprintf("\r\nPress P to pause, R to resume, any other key to stop...");
                testPlayer.Loop( loopCount );
                while (testPlayer.GetState() != WavPlayer::STATE_FINISHED) {
                    key = getchar();
                    switch (key) {
                        case 'P':
                        case 'p':
                            testPlayer.Pause();
                            break;
                        case 'R':
                        case 'r':
                            testPlayer.Resume();
                            break;
                        default:
                            testPlayer.Stop();
                            break;
                    }
                }
            }
            iprintf(" Done\r\n");
            break;
#endif /* #ifdef WAV_PLAYER_FILESYSTEM */
        case 'p':
        case 'P':
            if (testPlayer.GetState() == WavPlayer::STATE_FINISHED) {
                iprintf("Playing Audio...");
                testPlayer.Play( &playbackSem );
                OSSemPend( &playbackSem, 0 );
                iprintf(" Done\r\n");
            }
            break;
        default:
            break;
    }
}

void DisplayMenu()
{
    iprintf("\r\n\r\nEnter selection:\r\n");
    iprintf("1. Play compiled sample voice file - 2ch, 44k samples/sec\r\n");
    iprintf("2. Play compiled sample voice file - 2ch,  8k samples/sec\r\n");
    iprintf("3. Play compiled sample tone file (440Hz) - 2ch, 8k samples/sec\r\n");
    iprintf("4. Loop compiled sample voice file - 2ch, 44k samples/sec\r\n");
#ifdef WAV_PLAYER_FILESYSTEM
    iprintf("5. List filesystem directory\r\n");
    iprintf("6. Change current directory\r\n");
    iprintf("7. Play audio file from file\r\n");
    iprintf("8. Loop audio file from file\r\n");
#endif /* #ifdef WAV_PLAYER_FILESYSTEM */
    if (testPlayer.GetState() == WavPlayer::STATE_FINISHED) {
        iprintf("P. Play previous audio\r\n");
    }
}

void UserMain(void * pd)
{
    // Initialize network stack and standard system utilities
    init();

    iprintf("Application started\n");

    OSSemInit( &playbackSem, 0 );

    WavPlayer::wavError ret;
    ret = testPlayer.SetChannelDAC( 0, 1 );
    ret = testPlayer.SetChannelDAC( 1, 0 );
    ret = testPlayer.SetTimer( 3 );
    (void) ret;

    while (1)
    {
        DisplayMenu();
        char c = getchar();
        iprintf("\r\n");
        ProcessCommand( c );
    }
}
