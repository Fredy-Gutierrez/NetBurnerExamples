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

#ifndef  ___WAV_PLAYER_H
#define  ___WAV_PLAYER_H

#include <nbrtos.h>
#include <sim.h>

// Comment out this macro to remove filesystem requirement
#define WAV_PLAYER_FILESYSTEM (1)

// Sim1 Miscellaneous Control Register
#define MISCCR2_DAC1SEL 0x0040   // Enable DAC1 drive output
#define MISCCR2_DAC0SEL 0x0020   // Enable DAC0 drive output
#define MISCCR2_ADCEN   0x0010   // Enable ADC 6-4 and 2-0
#define MISCCR2_ADC7EN  0x0008   // Enable ADC 7
#define MISCCR2_ADC3EN  0x0004   // Enable ADC 3

// ADC Calibration Register
#define ADC_CAL_DAC1  0x0002     // Selects the source of the ADCA7 input as DAC1 output.
#define ADC_CAL_DAC0  0x0001     // Selects the source of the ADCA3 input as DAC0 output.

// DAC Control Register
#define DAC_CR_RESET    0x1101
#define DAC_CR_FILT     0x1000
#define DAC_CR_WMLVL_M  0x0300
#define DAC_CR_WMLVL_S  8
#define DAC_CR_DMA      0x0080
#define DAC_CR_HSLS     0x0040
#define DAC_CR_UP       0x0020
#define DAC_CR_DOWN     0x0010
#define DAC_CR_AUTO     0x0008
#define DAC_CR_SYNC     0x0004
#define DAC_CR_FMT      0x0002
#define DAC_CR_PDN      0x0001       // Power down. 0 = power on, 1 = power down

// DAC Trigger Select Register DACTSR
// Channels
#define DAC1_CH0  0x0000               // Source channel 0
#define DAC1_CH1  0x0800               // Source channel 1
#define DAC1_CH2  0x1000               // Source channel 2
#define DAC1_CH3  0x1800               // Source channel 3

#define DAC0_CH0  0x0000               // Source channel 0
#define DAC0_CH1  0x0008               // Source channel 1
#define DAC0_CH2  0x0010               // Source channel 2
#define DAC0_CH3  0x0018               // Source channel 3

// DAC Trigger Sources
#define DAC1_SRC_PWMA      0x0000      // PWMs
#define DAC1_SRC_PWMB      0x0100
#define DAC1_SRC_PWMX      0x0200
#define DAC1_SRC_PWMTRIG1  0x0300
#define DAC1_SRC_PWMTRIG0  0x0400
#define DAC1_SRC_TnOUT     0x0500      // Timers
#define DAC1_SRC_TnIN      0x0600

#define DAC0_SRC_PWMA      0x0000      // PWMs
#define DAC0_SRC_PWMB      0x0001
#define DAC0_SRC_PWMX      0x0002
#define DAC0_SRC_PWMTRIG1  0x0003
#define DAC0_SRC_PWMTRIG0  0x0004
#define DAC0_SRC_TnOUT     0x0005      // Timers
#define DAC0_SRC_TnIN      0x0006


#define EDMA_CH_DAC0        62
#define EDMA_CH_DAC1        63
#define EDMA_CH_DAC0_INT    0x40000000
#define EDMA_CH_DAC1_INT    0x80000000

#define TCD_POOL_SIZE       20
#define DAC_COUNT           2
#define MAX_SAMPLE_RATE     100000


namespace WAVFILE {
    struct Chunk {
        char        id[4];
        uint32_t    size;
    };
    struct RIFFChunk {
        char        ChunkID[4]; // 'R', 'I', 'F', 'F'
        uint32_t    ChunkSize;
        char        Format[4]; // 'W', 'A', 'V', 'E'
    };
    struct formatChunk {
        char        ChunkID[4]; // 'f', 'm', 't', ' '
        uint32_t    SubChunkSize;
        uint16_t    AudioFormat; // 1 = PCM, others are handled
        uint16_t    ChannelCount; // How many channels are recorded in this file?
        uint32_t    SampleRate; // Samples per second
        uint32_t    ByteRate;   // Bytes per second = Sample Rate * ChannelCount * BitsPerSample
        uint16_t    BlockAlign; // Bytes per sample set = BitsPerSample * ChannelCount
        uint16_t    BitsPerSample; // Bits of data per sample
                                    // Note: this is rounded up to multiples of 8-bits
                                    // for BlockAlign and ByteRate
    };
    struct dataChunk {
        char        ChunkID[4]; // 'd', 'a', 't', 'a'
        uint32_t    SubChunkSize;
        uint8_t     data[]; // Arbitrary length array definition
    };

    // Structure definition for the 'canonical' WAV file
    struct WavFile {
        RIFFChunk   riff; // Leading chunk defining the data block as a RIFF chunk
        formatChunk fmt;  // First subchunk, describing the format of the data section
        dataChunk   data; // The actual data section
    };
}


class WavPlayer {
public:
// Public enum definitions
    enum playState {
        STATE_NO_DAC,
        STATE_NO_TIMER,
        STATE_NOT_LOADED,
        STATE_NOT_PROCESSED,
        STATE_BUFFER_RESET,
        STATE_PROCESSED,
        STATE_READY,
        STATE_PLAYING,
        STATE_UNMUTE,
        STATE_MUTE,
        STATE_PAUSED_UNMUTE,
        STATE_PAUSED_MUTE,
        STATE_PAUSED,
        STATE_FINISHED
    };

    enum wavError {
        ERROR_NONE,
        ERROR_PLAYING,
        ERROR_FILE,
        ERROR_FILE_SIZE,
        ERROR_TYPE,
        ERROR_SHORT,
        ERROR_LONG,
        ERROR_FORMAT,
        ERROR_RATE,
        ERROR_BITSIZE,
        ERROR_SIZE_MISMATCH,
        ERROR_CHANNEL,
        ERROR_DACNUM,
        ERROR_TIMER,
        ERROR_IN_USE,
        ERROR_OTHER
    };

private:
    enum readMode {
        MODE_FILE,
        MODE_BUFFER,
        MODE_NONE
    };

    struct channelControl {
        int                     dacNum;
        uint32_t                dataRem;
        uint16_t                transfersRem;
        uint16_t                finalTransferSize;
        bool                    finished;
        volatile dacstruct      *dac;
        volatile edma_tcdstruct *tcd;
    };

    struct wavData {
        uint32_t SampleRate;
        uint16_t BitsPerSample;
        uint16_t ChannelCount;
        uint32_t dataSize;
    };

    struct initialPlaySettings {
        uint16_t channel;
        uint16_t transfersRem;
        uint16_t transferSize;
    };

    static WavPlayer    *s_players[DAC_COUNT];

    readMode            m_mode;
    playState           m_state;
    wavData             m_wavInfo;
    channelControl      m_channel[DAC_COUNT];
    initialPlaySettings m_initSettings[DAC_COUNT];
    uint32_t            m_timer;
    int                 m_playsRem[DAC_COUNT];
    timerstruct         timerSettings;
    WAVFILE::WavFile   *m_pWav;
    WAVFILE::RIFFChunk *m_pWav_riff; // Leading chunk defining the data block as a RIFF chunk
    WAVFILE::formatChunk *m_pWav_fmt;  // First subchunk, describing the format of the data section
    WAVFILE::dataChunk *m_pWav_data; // The actual data section
    OS_SEM             *m_finishedSem;

    // Prepare the data for sending to DAC
    wavError PrepareData();

    const uint8_t *FindChunk(const char *chunkID, const uint8_t *data, uint32_t dataLen);
    wavError ParseHeader();
    // Helper functions for swapping endianess of header segments
    void PrepChunk_RIFF();
    void PrepChunk_fmt();
    void PrepChunk_data();

    void SetLengths();
    void ConfigDAC();
    void ConfigDMA();
    void ConfigTimer();
    void ISR( int channelNum );
    void SoftMuteISR();

public:
    WavPlayer();
    ~WavPlayer();

    static void RunISR( );
    static void RunSoftMuteISR(uint32_t vector);

#ifdef WAV_PLAYER_FILESYSTEM
    /* OpenFile
     * Opens and readies a WAV file from disk
     *
     *  Args:
     *  - fileName: pointer to C string containing the name of the file to be opened
     *  - dataBuffer: pointer to a buffer to load the file into
     *  - bufferSize: the size of the buffer being used
     *
     *  Returns:
     *  - ERROR_NONE:   File loaded successfully
     *  - Other:        Failure while loading file, value indicates type
     */
    wavError OpenFile( const char * fileName, uint8_t * dataBuffer, uint32_t bufferSize );
#endif /* #ifdef WAV_PLAYER_FILESYSTEM */
    /* OpenBuffer
     * Readies a WAV file stored in a buffer.
     * WARNING: This function mutates the passed buffer if successful. Until
     * either a new WAV file is loaded, ResetBuffer() is called, or the
     * WavPlayer is destroyed, the buffer should not be read or modified.
     *
     *  Args:
     *  - data: pointer to a buffer containing a WAV file
     *
     *  Returns:
     *  - ERROR_NONE:   File loaded successfully
     *  - Other:        Failure while loading file, value indicates type
     */
    wavError OpenBuffer( uint8_t * data );

    /* SetChannelDAC
     * Sets the association between an audio channel and the DAC that is used
     * to play it. Note: Only 2 channel WAVs are supported.
     *
     *  Args:
     *  - channel:  WAV sample channel to read from
     *  - dacNum:   DAC channel to output to
     *
     *  Returns:
     *  - ERROR_NONE:   Channel Associated
     *  - Other:        Failure, value indicates type
     */
    wavError SetChannelDAC( int channel = 0, int dacNum = 0);
    /* SetTimer
     * Selects which DMA Timer is used as the trigger source for the DAC(s)
     *
     *  Args:
     *  - timerNum: the dma timer to use
     *
     *  Returns:
     *  - ERROR_NONE:   Channel Associated
     *  - Other:        Failure, value indicates type
     */
    wavError SetTimer( int timerNum = 3 );

    /* Play
     * Plays the previously loaded WAV file
     *
     *  Args:
     *  - wavFinishedSem:   Optional semaphore posted to upon completion
     *
     *  Returns:
     *  NONE
     */
    wavError Play( OS_SEM *wavFinishedSem = NULL);

    /* Loop
     * Repeatedly plays the open file.
     *
     *  Args:
     *  - playCount: the number of times to play the file, 0 = loop forever
     *
     *  Returns:
     *
     */
    wavError Loop( uint32_t playCount = 0, OS_SEM *wavFinishedSem = NULL );

    /* Stop
     * Immediately stops playback
     *
     */
    wavError Stop();

    /* StopGraceful
     * Stops playback once current loop iteration is finished.
     */
    wavError StopGraceful();

    wavError Pause();

    wavError Resume();

    /* ResetBuffer
     * Reset the data buffer back to its unmutated state. Only needed when the
     * WAV file was loaded using OpenBuffer and the buffer is not reverted from
     * other sources.
     *
     *  Args:
     *  NONE
     *
     *  Returns:
     *  NONE
     */
    wavError ResetBuffer();

    /* GetState
     * Gets the current state of the WavPlayer.
     *
     *  Args:
     *  NONE
     *
     *  Returns:
     *  NONE
     */
    playState GetState();
};

#endif   /* ----- #ifndef ___WAV_PLAYER_H  ----- */

