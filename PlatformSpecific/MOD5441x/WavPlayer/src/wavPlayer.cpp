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
#include <sim.h>
#include <stdint.h>
#include <string.h>
#include <cfinter.h>
#include <intcdefs.h>
#include <nbrtoscpu.h>
#include "wavPlayer.h"
#include "edma.h"
#ifdef WAV_PLAYER_FILESYSTEM
    #include "FileSystemUtils.h"
#endif /* #ifdef WAV_PLAYER_FILESYSTEM */

#define SWPENDS(x) (((x>>8)&0xff) | /* move byte 1 to byte 0 */ \
                    ((x<<8)&0xff00)) /* move byte 0 to byte 1 */ \

#define SWPENDL(x) (((x>>24)&0xff) | /* move byte 3 to byte 0 */ \
                    ((x>>8)&0xff00) | /* move byte 2 to byte 1 */ \
                    ((x<<8)&0xff0000) | /* move byte 1 to byte 2 */ \
                    ((x<<24)&0xff000000)) /* byte 0 to byte 3 */

#define WAV_PRE_SIZE_DATASIZE   8

#ifdef _NBRTOS_H
#define INTERRUPT_WITH_VECTOR(x,y)extern "C" { void real_##x(uint32_t vector);  void x(); } void fake_##x(){\
__asm__  (".global "#x);\
__asm__  (#x":");\
__asm__  ("move.w #0x2700,%sr ");\
__asm__  ("lea      -60(%a7),%a7 ");\
__asm__  ("movem.l  %d0-%d7/%a0-%a6,(%a7) ");\
__asm__  ("move.l (OSISRLevel32),%d0 ");\
__asm__  ("move.l %d0,-(%sp) ");\
__asm__  ("move.l (OSIntNesting),%d0");\
__asm__  ("addq.l #1,%d0");\
__asm__  ("move.l %d0,(OSIntNesting)");\
__asm__  ("move.l #"#y",%d0 ");\
__asm__  ("move.w %d0,%sr ");\
__asm__  ("move.l %d0,(OSISRLevel32)");\
__asm__  ("move.l 64(%a7), %d0");\
__asm__  ("movq  #18, %d1");\
__asm__  ("lsr.l %d1, %d0");\
__asm__  ("andi.l #0xFF, %d0");\
__asm__  ("move.l %d0, %a7@-");\
__asm__  ("jsr real_"#x );\
__asm__  ("addq.l #4,%a7 ");\
__asm__  ("move.l (%sp)+,%d0 ");\
__asm__  ("move.l %d0,(OSISRLevel32)");\
__asm__  (" jsr      OSIntExit  ");\
__asm__  ("movem.l  (%a7),%d0-%d7/%a0-%a6 ");\
__asm__  ("lea    60(%a7),%a7 ");\
__asm__  ("rte");} void real_##x(uint32_t vector)

#else
#error NBRTOS must be included
#endif



extern unsigned long CPU_CLOCK;
WavPlayer *WavPlayer::s_players[DAC_COUNT];

INTERRUPT_WITH_VECTOR(wavPlayerSoftMuteISR, 0x2200)
{
    WavPlayer::RunSoftMuteISR( vector );
}
void WavPlayer::RunSoftMuteISR(uint32_t vector )
{
    uint32_t timerNum = vector - 32 - 64; // Timer0 vector = 32 + Offset for Interrupts = 64
    if (timerNum > 3) {
        return;
    }
    for (int i = 0; i < DAC_COUNT; i++) {
        if (s_players[i]->m_timer == timerNum) {
            return s_players[i]->SoftMuteISR();
        }
    }
}

uint32_t getFirstSample(uint8_t *data, uint32_t sampleWidth, uint32_t chNum)
{
    return ((*((uint32_t*)(data + (sampleWidth/8*chNum))))
                >> (32-sampleWidth))
            & ((1 << (16-(sampleWidth % 16))) - 1);
}

void WavPlayer::SoftMuteISR()
{
    uint32_t chNum = 0;
    bool dacReady = true;
    volatile timerstruct &timer = sim2.timer[m_timer];
    switch (m_state) {
    case STATE_UNMUTE:
    {
        // Calculate the sample step size needed to move full range in <= 50 msec
        uint32_t stepSiz = 0x1000/(50*(m_pWav_fmt->SampleRate)/1000);
        if (stepSiz == 0)
        {
            stepSiz++;
        }
        for (int i = 0; i < DAC_COUNT; i++) {
            if ((m_channel[i].dacNum >= 0) && (i < m_pWav_fmt->ChannelCount)) {
                channelControl &channel = m_channel[i];
                uint32_t firstSample = getFirstSample(m_pWav_data->data, m_pWav_fmt->BitsPerSample, chNum);
                uint16_t dacVal = channel.dac->data;
                switch (m_pWav_fmt->BitsPerSample) {
                    case 32:
                    case 16:
                    {
                        firstSample = (firstSample >> 4) & 0xFFF;
                        dacVal >>= 4;
                        if (firstSample < dacVal)
                        {
                            if ((dacVal - firstSample) < stepSiz)
                            {
                                channel.dac->data = firstSample<<4;
                            }
                            else
                            {
                                channel.dac->data = (dacVal-stepSiz)<<4;
                            }
                            dacReady = false;
                        }
                        else if (firstSample > dacVal)
                        {
                            if ((firstSample - dacVal) < stepSiz)
                            {
                                channel.dac->data = firstSample<<4;
                            }
                            else
                            {
                                channel.dac->data = (dacVal+stepSiz)<<4;
                            }
                            dacReady = false;
                        }
                        break;
                    }

                    case 24:
                    case 8:
                    {
                        firstSample &= 0xFF;
                        dacVal &= 0xFF;
                        if (firstSample < dacVal)
                        {
                            if (((dacVal - firstSample)) < stepSiz)
                            {
                                channel.dac->data = firstSample;
                            }
                            else
                            {
                                channel.dac->data = (dacVal-stepSiz);
                            }
                            dacReady = false;
                        }
                        else if (firstSample > dacVal)
                        {
                            if ((firstSample - dacVal) < stepSiz)
                            {
                                channel.dac->data = firstSample;
                            }
                            else
                            {
                                channel.dac->data = (dacVal+stepSiz);
                            }
                            dacReady = false;
                        }
                        break;
                    }
                }
                chNum++;
            }
        }
        timer.ter = 0x3;
        if (dacReady)
        {
            for (int i = 0; i < DAC_COUNT; i++) {
                if ((m_channel[i].dacNum >= 0) && (i < m_pWav_fmt->ChannelCount)) {
                    // Enable DMA requests
                    sim2.edma.serq = (EDMA_CH_DAC0 + m_channel[i].dacNum) & 0x3F;
                }
            }
            timer.txmr = 0x80;

            m_state = STATE_PLAYING;
        }
        break;
    }
    case STATE_MUTE:
    {
        uint32_t stepSiz = 0x1000/(50*(m_pWav_fmt->SampleRate)/1000);
        if (stepSiz == 0)
        {
            stepSiz++;
        }
        for (int i = 0; i < DAC_COUNT; i++) {
            if ((m_channel[i].dacNum >= 0) && (i < m_pWav_fmt->ChannelCount)) {
                channelControl &channel = m_channel[i];
                uint16_t dacVal = channel.dac->data;
                switch (m_pWav_fmt->BitsPerSample) {
                    case 32:
                    case 16:
                    {
                        dacVal >>= 4;
                        if (dacVal)
                        {
                            if (dacVal < stepSiz)
                            {
                                channel.dac->data = 0;
                            }
                            else
                            {
                                channel.dac->data = (dacVal-stepSiz)<<4;
                            }
                            dacReady = false;
                        }
                        break;
                    }

                    case 24:
                    case 8:
                    {
                        dacVal &= 0xFF;
                        if (dacVal)
                        {
                            if (dacVal < stepSiz)
                            {
                                channel.dac->data = 0;
                            }
                            else
                            {
                                channel.dac->data = (dacVal-stepSiz);
                            }
                            dacReady = false;
                        }
                        break;
                    }
                }
                chNum++;
            }
        }
        timer.ter = 0x3;
        if (dacReady)
        {
            timer.tmr &= ~(0x0006); // Stop DAC timer

            for (int i = 0; i < DAC_COUNT; i++) {
                if ((m_channel[i].dacNum >= 0) && (i < m_pWav_fmt->ChannelCount)) {
                    // Tristate the DAC's output
                    sim1.ccm.misccr2 &= ~(0x20 << m_channel[i].dacNum);
                }
            }

            m_state = STATE_FINISHED;
            if (m_finishedSem) {
                m_finishedSem->Post();
            }
        }
        break;
    }
    default: break;
    timer.ter = 0x3;
    }
}

INTERRUPT(wavPlayerISR, 0x2400)
{
    WavPlayer::RunISR( );
}

void WavPlayer::RunISR( )
{
    int channelNum;
    register uint32_t shadow = sim2.edma.inth;
    if (shadow & EDMA_CH_DAC0_INT) {
        channelNum = 0;
        sim2.edma.cint = EDMA_CH_DAC0;
    }
    else if (shadow & EDMA_CH_DAC1_INT) {
        channelNum = 1;
        sim2.edma.cint = EDMA_CH_DAC1;
    }
    else {
        return;
    }
    if (s_players[channelNum]) {
        s_players[channelNum]->ISR( channelNum );
    }
}

void WavPlayer::ISR( int dacNum )
{
    int channelNum = DAC_COUNT;
    for (channelNum = 0; channelNum < DAC_COUNT; channelNum++) {
        if (m_channel[channelNum].dacNum == dacNum) { break; }
    }
    if (channelNum == DAC_COUNT) {
        return; // dacNum not found
    }

    volatile edma_tcdstruct &tcd = *(m_channel[channelNum].tcd);
    channelControl &channel = m_channel[channelNum];
    if ((channel.transfersRem == 2) && (channel.finalTransferSize == 0)) {
        tcd.csr |= TCD_CSR_DREQ;
    }
    else if (channel.transfersRem == 1) {
        tcd.csr |= TCD_CSR_DREQ;
        if (channel.finalTransferSize > 0) {
            sim2.edma.cerq = (EDMA_CH_DAC0 + channel.dacNum) & 0x3F;
            tcd.biter = tcd.citer = channel.finalTransferSize;
            sim2.edma.serq = (EDMA_CH_DAC0 + channel.dacNum) & 0x3F;
        }
        else {
            channel.transfersRem--;
        }
    }

    if (channel.transfersRem > 0) {
        channel.transfersRem--;
    }
    else {
        if (m_playsRem[channelNum]) {
            initialPlaySettings &settings = m_initSettings[channelNum];

            channel.transfersRem = settings.transfersRem;

            tcd.saddr       = (uint32_t)(m_pWav_data->data + (2 * settings.channel));
            tcd.biter       = tcd.citer   = settings.transferSize;
            if (channel.transfersRem > 1) {
                tcd.csr     = 0x0002; // set the INTMAJOR bit
            }
            else {
                tcd.csr     = 0x000A; // set the D_REQ and INTMAJOR bits
            }
            sim2.edma.serq = (EDMA_CH_DAC0 + channel.dacNum) & 0x3F;
            if (m_playsRem[channelNum] != 0xFFFF) {
                m_playsRem[channelNum]--;
            }
        }
        else {
            bool allFinished = true;
            channel.finished = true;
            sim2.edma.cerq = (EDMA_CH_DAC0 + channel.dacNum) & 0x3F;
            for (int i = 0; i < DAC_COUNT; i++) {
                if ((m_channel[i].dacNum >= 0) && (!m_channel[i].finished)) {
                    allFinished = false;
                    break;
                }
            }
            if (allFinished && (m_state != STATE_MUTE)) {
                volatile timerstruct &timer = sim2.timer[m_timer];
                m_state = STATE_MUTE;
                timer.txmr = timerSettings.txmr = 0x00;
            }
        }
    }
}


WavPlayer::WavPlayer()
    : m_mode(MODE_NONE), m_state(STATE_NO_DAC), m_timer(-1), m_pWav(NULL)
{
    m_wavInfo.SampleRate       = 0;
    m_wavInfo.BitsPerSample    = 0;
    m_wavInfo.ChannelCount     = 0;
    m_wavInfo.dataSize         = 0;

    m_finishedSem              = 0;

    for (int i = 0; i < DAC_COUNT; i++) {
        m_channel[i].dacNum             = -1 ;
        m_channel[i].dataRem            = 0;
        m_channel[i].transfersRem       = 0;
        m_channel[i].finalTransferSize  = 0;
        m_channel[i].finished           = true;
        m_channel[i].dac                = NULL;
        m_channel[i].tcd                = NULL;

        m_initSettings[i].channel       = 0;
        m_initSettings[i].transfersRem  = 0;
        m_initSettings[i].transferSize  = 0;

        m_playsRem[i]                   = 0;
    }

}

WavPlayer::~WavPlayer()
{
    if (m_channel[0].dacNum >= 0) {
        m_channel[0].dac->cr &= ~DAC_CR_DMA;
        m_channel[0].dac->cr |= DAC_CR_PDN;
        s_players[m_channel[0].dacNum] = NULL;
    }
    if (m_channel[1].dacNum >= 0) {
        m_channel[1].dac->cr &= ~DAC_CR_DMA;
        m_channel[1].dac->cr |= DAC_CR_PDN;
        s_players[m_channel[1].dacNum] = NULL;
    }
    if ( (m_mode == MODE_BUFFER) && (m_state > STATE_BUFFER_RESET)) {
        ResetBuffer();
    }
}

WavPlayer::wavError WavPlayer::SetChannelDAC( int channel, int dacNum )
{
    if ((channel < 0) || (channel > 1)) {
        return ERROR_CHANNEL;
    }

    if ((dacNum < 0) || (dacNum > 1)) {
        return ERROR_DACNUM;
    }

    if (m_channel[channel].dacNum != -1) {
        s_players[m_channel[channel].dacNum] = NULL;
    }
    m_channel[channel].dacNum   = dacNum ;
    m_channel[channel].dac      = &sim2.dac[dacNum];
    m_channel[channel].tcd      = &sim2.edma.tcd[EDMA_CH_DAC0 + dacNum];

    s_players[m_channel[channel].dacNum] = this;
    if (m_timer < 0) {
        m_state = STATE_NO_TIMER;
    }
    else {
        m_state = STATE_NOT_LOADED;
    }
    return ERROR_NONE;
}

WavPlayer::wavError WavPlayer::SetTimer( int timerNum )
{
    if ((timerNum < 0) || (timerNum > 3)) {
        return ERROR_TIMER;
    }
    m_timer = timerNum;

    for (int i = 0; i < DAC_COUNT; i++) {
        if (m_channel[i].dacNum >= 0) {
            m_state = STATE_NOT_LOADED;
            break;
        }
    }

    if (m_state != STATE_NOT_LOADED) {
        m_state = STATE_NO_DAC;
    }
    return ERROR_NONE;
}

#ifdef WAV_PLAYER_FILESYSTEM
WavPlayer::wavError WavPlayer::OpenFile(
        const char * fileName, uint8_t *dataBuffer, uint32_t bufferSize)
{
    F_FILE *fWav = NULL;
    uint32_t fileLen, dataRead, closeErr;
    wavError ret;

    if (m_state < STATE_NOT_LOADED) {
        return ERROR_OTHER;
    }
    if ((m_state >= STATE_PLAYING) && (m_state <= STATE_MUTE)) {
        return ERROR_PLAYING;
    }
    fileLen = f_filelength( fileName );
    if (bufferSize < fileLen) {
        return ERROR_FILE_SIZE;
    }

    fWav = f_open( fileName, "r" );
    dataRead = f_read( dataBuffer, 1, fileLen, fWav );
    if (dataRead != fileLen) {
        return ERROR_FILE;
    }
    closeErr = f_close( fWav );
    if (closeErr != F_NO_ERROR) {
        return ERROR_FILE;
    }
    ret = OpenBuffer(dataBuffer);

    if ((ret != ERROR_PLAYING) && (m_state > STATE_NOT_LOADED)) {
        m_mode = MODE_FILE;
    }
    return ret;
}
#endif /* #ifdef WAV_PLAYER_FILESYSTEM */

WavPlayer::wavError WavPlayer::OpenBuffer( uint8_t * data )
{
    wavError ret = ERROR_NONE;
    if (m_state < STATE_NOT_LOADED) {
        return ERROR_OTHER;
    }
    if ((m_state >= STATE_PLAYING) && (m_state <= STATE_MUTE)) {
        return ERROR_PLAYING;
    }

    if ((m_state >= STATE_PROCESSED) && (m_mode == MODE_BUFFER)) {
        ResetBuffer();
    }

    m_pWav = (WAVFILE::WavFile *)data;
    m_pWav_riff = (WAVFILE::RIFFChunk *)data;

    m_state = STATE_NOT_PROCESSED;
    ret = ParseHeader();
    if (ret != ERROR_NONE) { return ret; }

    SetLengths();

    ret = PrepareData();
    m_state = STATE_PROCESSED;
    if (ret != ERROR_NONE) { return ret; }

    ConfigDAC();
    if (ret != ERROR_NONE) { return ret; }

    ConfigDMA();
    ConfigTimer();
    m_state = STATE_READY;
    m_mode = MODE_BUFFER;
    return ret;
}

void WavPlayer::SetLengths()
{
    for ( int i = 0; i < DAC_COUNT; i++ ) {
        if (m_channel[i].dacNum >= 0) {
            m_channel[i].dataRem = m_pWav_data->SubChunkSize / m_pWav_fmt->ChannelCount;
        }
    }
}

inline void WavPlayer::PrepChunk_RIFF()
{
    m_pWav_riff = (WAVFILE::RIFFChunk*)m_pWav;
    m_pWav_riff->ChunkSize = SWPENDL(m_pWav_riff->ChunkSize);
}

inline void WavPlayer::PrepChunk_fmt()
{
    m_pWav_fmt->SubChunkSize    = SWPENDL(m_pWav_fmt->SubChunkSize);
    m_pWav_fmt->AudioFormat     = SWPENDS(m_pWav_fmt->AudioFormat);
    m_pWav_fmt->ChannelCount    = SWPENDS(m_pWav_fmt->ChannelCount);
    m_pWav_fmt->SampleRate      = SWPENDL(m_pWav_fmt->SampleRate);
    m_pWav_fmt->ByteRate        = SWPENDL(m_pWav_fmt->ByteRate);
    m_pWav_fmt->BlockAlign      = SWPENDS(m_pWav_fmt->BlockAlign);
    m_pWav_fmt->BitsPerSample   = SWPENDS(m_pWav_fmt->BitsPerSample);
}

inline void WavPlayer::PrepChunk_data()
{
    m_pWav_data->SubChunkSize = SWPENDL(m_pWav_data->SubChunkSize);
}

const uint8_t * WavPlayer::FindChunk(const char *chunkID, const uint8_t *data, uint32_t dataLen)
{
    const uint8_t *end_data = data + dataLen;
    while (data < end_data) {
        WAVFILE::Chunk *chunk = (WAVFILE::Chunk*)data;
        if (strncmp(chunk->id, chunkID, 4) == 0) {
            return data;
        }
        uint32_t size = SWPENDL(chunk->size);
        size += (size & 1); // pad to even
        data += size + 8;
    }
    return NULL;
}
WavPlayer::wavError WavPlayer::ParseHeader()
{
    // RIFF chunk checks
    if (strncmp(m_pWav_riff->ChunkID, "RIFF", 4)) {
        return ERROR_TYPE;
    }
    PrepChunk_RIFF();

    if (m_pWav_riff->ChunkSize < 44) {
        PrepChunk_RIFF();
        return ERROR_SHORT;
    }
    m_pWav_fmt = (WAVFILE::formatChunk *)FindChunk("fmt ", ((uint8_t*) m_pWav_riff)+12,
                                            m_pWav_riff->ChunkSize);
    m_pWav_data = (WAVFILE::dataChunk *)FindChunk("data", ((uint8_t*) m_pWav_riff)+12,
                                            m_pWav_riff->ChunkSize);
    if (!m_pWav_fmt) {
        return ERROR_TYPE;
    }

    if (strncmp(m_pWav_fmt->ChunkID, "fmt ", 4)) {
        PrepChunk_RIFF();
        return ERROR_TYPE;
    }

    // fmt chunk checks
    PrepChunk_fmt();

    if (m_pWav_fmt->AudioFormat != 1) {
        PrepChunk_RIFF();
        PrepChunk_fmt();
        return ERROR_FORMAT;
    }
    if (m_pWav_fmt->SampleRate > MAX_SAMPLE_RATE) {
        PrepChunk_RIFF();
        PrepChunk_fmt();
        return ERROR_RATE;
    }
    if ((m_pWav_fmt->BitsPerSample != 32)
            && (m_pWav_fmt->BitsPerSample != 24)
            && (m_pWav_fmt->BitsPerSample != 16)
            && (m_pWav_fmt->BitsPerSample != 8)) {
        PrepChunk_RIFF();
        PrepChunk_fmt();
        return ERROR_BITSIZE;
    }
    // make sure that our datastructure agrees with the file's datastructure
    if (m_pWav_fmt->SubChunkSize != (sizeof(WAVFILE::formatChunk) - WAV_PRE_SIZE_DATASIZE)) {
        PrepChunk_RIFF();
        PrepChunk_fmt();
        return ERROR_SIZE_MISMATCH;
    }

    if (!m_pWav_data) {
        return ERROR_TYPE;
    }

    if (strncmp(m_pWav_data->ChunkID, "data", 4)) {
        PrepChunk_RIFF();
        PrepChunk_fmt();
        return ERROR_TYPE;
    }

    // data chunk checks
    PrepChunk_data();
    // make sure the full chunk is at least as big as the fmt and data chunks
    if (m_pWav_riff->ChunkSize <
            (4 + 8 + m_pWav_fmt->SubChunkSize + 8 + m_pWav_data->SubChunkSize)) {
        PrepChunk_RIFF();
        PrepChunk_fmt();
        PrepChunk_data();
        return ERROR_SIZE_MISMATCH;
    }

    return ERROR_NONE;
}

WavPlayer::wavError WavPlayer::PrepareData()
{
    switch (m_pWav_fmt->BitsPerSample) {
    case 32:
    {
        uint32_t *data = (uint32_t *)m_pWav_data->data;
        uint32_t *end  = (uint32_t *)(m_pWav_data->data + m_pWav_data->SubChunkSize);
        while (data < end) {
            *data = SWPENDL(*data);
            *data += 0x80000000; // convert from signed samples to unsigned samples
            data++;
        }
    }
    break;
    case 24:
//    {
//        uint32_t *data8 = (uint32_t *)m_pWav_data->data;
//        uint32_t *end  = (uint32_t *)(m_pWav_data->data + m_pWav_data->SubChunkSize);
//        uint32_t data;
//        while (data8 < end) {
//            data = (data8[0] + (data8[1] << 8) + (data8[2] << 8));
//            data += 0x800000; // convert from signed samples to unsigned samples
//            if (data>>24) {
//                data = (data & 0xFFFFFF) + (data >> 24);
//            }
//            data8[0] = (data >>  0) & 0xFF;
//            data8[1] = (data >>  8) & 0xFF;
//            data8[2] = (data >> 16) & 0xFF;
//            data8+=3;
//        }
//    }
    break;
    case 16:
    {
        uint16_t *data = (uint16_t *)m_pWav_data->data;
        uint16_t *end  = (uint16_t *)(m_pWav_data->data + m_pWav_data->SubChunkSize);
        while (data < end) {
            *data = SWPENDS(*data);
            *data += 0x8000; // convert from signed samples to unsigned samples
            data++;
        }
    }
    break;
    case 8:
    default:
        break;
    }
    return ERROR_NONE;
}

WavPlayer::wavError WavPlayer::ResetBuffer()
{
    if ((m_state >= STATE_PLAYING) && (m_state <= STATE_MUTE)) {
        return ERROR_PLAYING;
    }
    // Buffer is untouched
    if (m_state < STATE_PROCESSED) {
        return ERROR_NONE;
    }
    // Reset the data section back to the original
    switch (m_pWav_fmt->BitsPerSample) {
    case 32:
    {
        uint32_t *data = (uint32_t *)m_pWav_data->data;
        uint32_t *end  = (uint32_t *)(m_pWav_data->data + m_pWav_data->SubChunkSize);
        while (data < end) {
            *data -= 0x80000000; // convert from unsigned samples back to signed samples
            *data = SWPENDL(*data);
            data++;
        }
    }
    break;
    case 24:
//    {
//        uint32_t *data8 = (uint32_t *)m_pWav_data->data;
//        uint32_t *end  = (uint32_t *)(m_pWav_data->data + m_pWav_data->SubChunkSize);
//        uint32_t data;
//        while (data8 < end) {
//            data = (data8[0] + (data8[1] << 8) + (data8[2] << 8));
//            data += 0x800000; // convert from signed samples to unsigned samples
//            if (data>>24) {
//                data = (data & 0xFFFFFF) + (data >> 24);
//            }
//            data8[0] = (data >>  0) & 0xFF;
//            data8[1] = (data >>  8) & 0xFF;
//            data8[2] = (data >> 16) & 0xFF;
//            data8+=3;
//        }
//    }
    break;
    case 16:
    {
        uint16_t *data = (uint16_t *)m_pWav_data->data;
        uint16_t *end  = (uint16_t *)(m_pWav_data->data + m_pWav_data->SubChunkSize);
        while (data < end) {
            *data -= 0x8000; // convert from unsigned samples back to signed samples
            *data = SWPENDS(*data);
            data++;
        }
    }
    break;
    case 8:
    default:
        break;
    }

    // Reset the file header back to the original
    PrepChunk_RIFF();
    PrepChunk_fmt();
    PrepChunk_data();

    // Have to make sure that if the timer and dacs aren't configured that
    // playing fails
    if (m_state < STATE_READY) {
        m_state = STATE_NOT_PROCESSED;
    }
    else {
        m_state = STATE_BUFFER_RESET;
    }

    return ERROR_NONE;
}

void WavPlayer::ConfigDAC()
{
    for (int i = 0; i < DAC_COUNT; i++) {
        if (m_channel[i].dacNum >= 0) {
            m_channel[i].dac->cr =
                DAC_CR_FILT
                | ((0 << DAC_CR_WMLVL_S) & DAC_CR_WMLVL_M)
                | DAC_CR_DMA
                | DAC_CR_HSLS
                | DAC_CR_SYNC;
            if ((m_pWav_fmt->BitsPerSample == 32)
                    || (m_pWav_fmt->BitsPerSample == 16)) {
                m_channel[i].dac->cr |= DAC_CR_FMT; // left justified
            }
            else {
                m_channel[i].dac->cr &= ~DAC_CR_FMT; // right justified
            }
            // Configure the DAC/ADC pin for DAC output
            sim1.ccm.misccr2 &= (~(0x04)) << m_channel[i].dacNum;
            sim2.adc.cal |= 1 << m_channel[i].dacNum;
        }
    }
}


//static void DumpTCD( const volatile edma_tcdstruct &tcd )
//{
//    iprintf("TCD: %p\r\n", &tcd);
//    iprintf("  saddr:     %08X\r\n", tcd.saddr);
//    iprintf("  attr:      %04X\r\n", tcd.attr);
//    iprintf("  soff:      %04X\r\n", tcd.soff);
//    iprintf("  nbytes:    %08X\r\n", tcd.nbytes);
//    iprintf("  slast:     %08X\r\n", tcd.slast);
//    iprintf("  daddr:     %08X\r\n", tcd.daddr);
//    iprintf("  citer:     %04X\r\n", tcd.citer);
//    iprintf("  doff:      %04X\r\n", tcd.doff);
//    iprintf("  dlast_sga: %08X\r\n", tcd.dlast_sga);
//    iprintf("  biter:     %04X\r\n", tcd.biter);
//    iprintf("  csr:       %04X\r\n", tcd.csr);
//    iprintf("\r\n");
//}

void WavPlayer::ConfigDMA()
{
    int channels = 0;
    uint32_t transferSize = 0;
    for (int i = 0; i < DAC_COUNT; i++) {
        if ((m_channel[i].dacNum >= 0) && (i < m_pWav_fmt->ChannelCount)) {
            volatile edma_tcdstruct &tcd = *(m_channel[i].tcd);
            channelControl &channel = m_channel[i];
            initialPlaySettings &settings = m_initSettings[i];

            switch (m_pWav_fmt->BitsPerSample) {
                case 32: transferSize = channel.dataRem/4; break;
                case 24: transferSize = channel.dataRem/3; break;
                case 16: transferSize = channel.dataRem/2; break;
                case  8: transferSize = channel.dataRem; break;
            }
            if ((m_pWav_fmt->BitsPerSample == 16)
                   || (m_pWav_fmt->BitsPerSample == 32)) {
                channel.transfersRem = transferSize / TCD_CITER_MAX_COUNT;
                channel.finalTransferSize = transferSize % TCD_CITER_MAX_COUNT;
                settings.transfersRem = channel.transfersRem;

                if (transferSize > TCD_CITER_MAX_COUNT) {
                    transferSize = TCD_CITER_MAX_COUNT;
                }

                settings.transferSize   = transferSize;
                settings.channel        = channels;

                tcd.saddr   = (uint32_t)(m_pWav_data->data
                            + (channels
                                * ((m_pWav_fmt->BitsPerSample == 32) ? 4 : 2))
                            + ((m_pWav_fmt->BitsPerSample == 32) ? 2 : 0));
                tcd.attr    = TCD_ATTR_16BIT_TRANS;
                // If the bit depth is 32, skip the low half of the sample...
                tcd.soff    = m_pWav_fmt->ChannelCount
                                * ((m_pWav_fmt->BitsPerSample == 32) ? 4 : 2);
                tcd.nbytes  = 2;
                tcd.slast   = 0;
                tcd.doff    = 0;
                tcd.daddr   = (volatile uint32_t)&(channel.dac->data);

                tcd.dlast_sga = 0;
                tcd.biter   = tcd.citer   = transferSize;
                if (channel.transfersRem > 1) {
                    tcd.csr     = 0x0002; // set the INTMAJOR bit
                }
                else {
                    tcd.csr     = 0x000A; // set the D_REQ and INTMAJOR bits
                }
            }
            else {
                channel.transfersRem = transferSize / TCD_CITER_MAX_COUNT;
                channel.finalTransferSize = transferSize % TCD_CITER_MAX_COUNT;
                settings.transfersRem = channel.transfersRem;

                if (transferSize > TCD_CITER_MAX_COUNT) {
                    transferSize = TCD_CITER_MAX_COUNT;
                }

                settings.transferSize = transferSize;
                settings.channel        = channels;

                tcd.saddr   = (uint32_t)(m_pWav_data->data
                            + (channels
                                * ((m_pWav_fmt->BitsPerSample == 24) ? 3 : 1))
                            + ((m_pWav_fmt->BitsPerSample == 24) ? 2 : 0));
                tcd.attr    = TCD_ATTR_8BIT_TRANS;
                tcd.soff    = m_pWav_fmt->ChannelCount
                                * ((m_pWav_fmt->BitsPerSample == 24) ? 3 : 1);
                tcd.nbytes  = 1;
                tcd.slast   = 0;
                tcd.doff    = 0;
                tcd.daddr   = (volatile uint32_t)((uint8_t *)&(channel.dac->data)) + 1;

                tcd.dlast_sga = 0;
                tcd.citer   = transferSize;
                tcd.biter   = tcd.biter;
                if (channel.transfersRem > 1) {
                    tcd.csr     = 0x0002; // set the INTMAJOR bit
                }
                else {
                    tcd.csr     = 0x000A; // set the D_REQ and INTMAJOR bits
                }
            }
            channels++;
        }
    }
}

void WavPlayer::ConfigTimer()
{
    if ((m_timer < 0) || (m_timer > 3)) {
        return;
    }
    for (int i = 0; i < DAC_COUNT; i++) {
        if (m_channel[i].dacNum >= 0) {
            // Clear DACn trigger source
            sim1.ccm.dactsr &= ~( (0x1F) << (8*(m_channel[i].dacNum)) );
            // Set DACn trigger source to T<m_timer>OUT
            sim1.ccm.dactsr |= ((m_timer<<3) | (0x05)) << (8*(m_channel[i].dacNum));
        }
    }

    // configure the timer as free-running, at the sampling rate
    timerSettings.tmr   = 0x0038;
    timerSettings.txmr  = 0x00;
    // The source clock is CPU_CLOCK / 2, but we need the clock to trigger twice
    // for each DAC sample
    timerSettings.trr   = (CPU_CLOCK / 4) / m_pWav_fmt->SampleRate;
    timerSettings.tcn   = 0x00000000;
}

WavPlayer::wavError WavPlayer::Play( OS_SEM *wavFinishedSem )
{
    return Loop( 1, wavFinishedSem );
}

WavPlayer::wavError WavPlayer::Loop( uint32_t playCount, OS_SEM *wavFinishedSem )
{
    volatile timerstruct &timer = sim2.timer[m_timer];
    if (m_state == STATE_BUFFER_RESET) {
        ParseHeader();
        PrepareData();
        m_state = STATE_READY;
    }

    if ( !((m_state == STATE_READY) || (m_state == STATE_FINISHED)) ) {
        if (wavFinishedSem) {
             wavFinishedSem ->Post();
        }

        if ((m_state >= STATE_PLAYING) && (m_state <= STATE_MUTE)) {
            return ERROR_PLAYING;
        }
        if (m_state == STATE_NO_DAC) {
            return ERROR_DACNUM;
        }
        if (m_state == STATE_NO_TIMER) {
            return ERROR_TIMER;
        }
        if ( (m_state == STATE_NOT_LOADED) || (m_state == STATE_NOT_PROCESSED) ) {
            return ERROR_FILE;
        }
        return ERROR_OTHER;
    }

    SetLengths();
    ConfigDMA();

    m_finishedSem = wavFinishedSem;
    timer.tmr   = 0;
    asm ("nop");
    timer.tmr   = timerSettings.tmr;
    timer.trr   = timerSettings.trr;
    timer.txmr  = timerSettings.txmr;
    timer.tcn   = timerSettings.tcn;

    SetIntc(0,(long)wavPlayerSoftMuteISR,32+m_timer,2);
    SETUP_DMA56_63_ISR( wavPlayerISR, 2 );
    for (int i = 0; i < DAC_COUNT; i++) {
        m_playsRem[i] = playCount - 1;
        if ((m_channel[i].dacNum >= 0) && (i < m_pWav_fmt->ChannelCount)) {
            // Actively drive the DAC's output
            sim1.ccm.misccr2 |= 0x20 << m_channel[i].dacNum;
        }
    }
    m_state = STATE_UNMUTE;

    timer.tmr |= 0x0003; // start the timer

    return ERROR_NONE;
}

WavPlayer::wavError WavPlayer::Stop()
{
    if ((m_state < STATE_PLAYING) || (m_state > STATE_PAUSED)) {
        return ERROR_PLAYING;
    }

    USER_ENTER_CRITICAL();
    // Stop DAC timer
    sim2.timer[m_timer].tmr&= ~(0x0006);

    for (int i = 0; i < DAC_COUNT; i++) {
        // Tristate the DAC's output
        sim1.ccm.misccr2 &= ~(0x20 << m_channel[i].dacNum);
        m_playsRem[i] = 0;
        m_channel[i].transfersRem = 0;
    }
    USER_EXIT_CRITICAL();

    m_state = STATE_FINISHED;
    if (m_finishedSem) {
        m_finishedSem->Post();
    }

    return ERROR_NONE;
}

WavPlayer::wavError WavPlayer::StopGraceful()
{
    if ((m_state < STATE_PLAYING) || (m_state > STATE_PAUSED)) {
        return ERROR_PLAYING;
    }
    USER_ENTER_CRITICAL();
    for (int i = 0; i < DAC_COUNT; i++) {
        m_playsRem[i] = 0;
    }
    USER_EXIT_CRITICAL();
    return ERROR_NONE;
}

WavPlayer::wavError WavPlayer::Pause()
{
    if ((m_state < STATE_PLAYING) || (m_state > STATE_MUTE)) {
        return ERROR_PLAYING;
    }
    sim2.timer[m_timer].tmr&= ~(0x0006);

    switch(m_state) {
        case STATE_PAUSED_UNMUTE:
        case STATE_UNMUTE: m_state = STATE_PAUSED_UNMUTE; break;
        case STATE_PAUSED_MUTE:
        case STATE_MUTE: m_state = STATE_PAUSED_MUTE; break;
        default:
        case STATE_PLAYING: m_state = STATE_PAUSED; break;
    }

    return ERROR_NONE;
}

WavPlayer::wavError WavPlayer::Resume()
{
    if ((m_state < STATE_PAUSED_UNMUTE) || (m_state > STATE_PAUSED)) {
        return ERROR_PLAYING;
    }
    switch(m_state) {
        case STATE_PAUSED_UNMUTE:   m_state = STATE_UNMUTE; break;
        case STATE_PAUSED_MUTE:     m_state = STATE_MUTE; break;
        default:
        case STATE_PAUSED: m_state = STATE_PLAYING; break;
    }

    sim2.timer[m_timer].tmr |= 0x0002; // start the timer

    return ERROR_NONE;
}

WavPlayer::playState WavPlayer::GetState()
{
    return m_state;
}
