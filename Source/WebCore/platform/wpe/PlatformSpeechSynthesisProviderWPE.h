/*
 * Copyright (C) 2016 TATA ELXSI
 * Copyright (C) 2016 Metrological
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef PlatformSpeechSynthesisProviderWPE_h
#define PlatformSpeechSynthesisProviderWPE_h
#if ENABLE(SPEECH_SYNTHESIS)

#include <Logging.h>
#include <wtf/MainThread.h>
#include <wtf/PassRefPtr.h>
#include <wtf/Vector.h>
#include <wtf/text/WTFString.h>
#include <wtf/Deque.h>
#include <wtf/threads/BinarySemaphore.h>
#include <wtf/Threading.h>

/* Aplay Utils*/
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <locale.h>
#include <alsa/asoundlib.h>

#ifndef LLONG_MAX
#define LLONG_MAX    9223372036854775807LL
#endif

/* FFMPEG Utils */
extern "C" {
#include <sys/stat.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/avcodec.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
}

#define HEADER_SIZE      46
#define FORMAT_SIZE      18
#define SAMPLE_SIZE      2
#define DATA_SIZE_POS    42
#define CHUNK_SIZE_POS   4
#define HEADER_DESC      8
#define AUDIO_FORMAT_PCM      0x0001

#define INTERMEDIATE_WAV_FILE  "/tmp/speechsynth.wav"
#define FINAL_WAV_FILE   "/tmp/speechspeed.wav"
#define AUDIO_DEVICE    "plughw:1"

/* Volume change */
#define ITERATIONS   32
#define NUMBERS      64
#define LOCATION     40
#define INDEX        2

#define A_CHUNK    1024 //Audio chunk size
#define BUFF_SIZE  4

#ifndef le16toh
#include <asm/byteorder.h>
#define le16toh(x) __le16_to_cpu(x)
#define be16toh(x) __be16_to_cpu(x)
#define le32toh(x) __le32_to_cpu(x)
#define be32toh(x) __be32_to_cpu(x)
#endif

#define DEFAULT_FORMAT          SND_PCM_FORMAT_U8
#define DEFAULT_SPEED           8000
#define FORMAT_WAVE             2
#define BLOCK                   0
#define OPENMODE                0

#define COMPOSE_ID(a,b,c,d) ((a) | ((b)<<8) | ((c)<<16) | ((d)<<24))

#define WAV_RIFF                COMPOSE_ID('R','I','F','F')
#define WAV_FMT                 COMPOSE_ID('f','m','t',' ')
#define WAV_DATA                COMPOSE_ID('d','a','t','a')
#define BE_INT(v)               bswap_32(v)
#define LE_INT(v)               (v)

#define TO_CPU_INT(v, be) \
    ((be) ? BE_INT(v) : LE_INT(v))

typedef struct {
    u_int magic;            /* 'RIFF' */
    u_int length;           /* filelen */
    u_int type;             /* 'WAVE' */
} WaveHeader;

typedef struct {
    u_int type;             /* 'data' */
    u_int length;           /* samplecount */
} WaveChunkHeader;

typedef struct voc_header {
    u_char  magic[20];      /* must be MAGIC_STRING */
    u_short headerlen;      /* Headerlength, should be 0x1A */
    u_short version;        /* VOC-file version */
    u_short coded_ver;      /* 0x1233-version */
} VocHeader;

typedef struct au_header {
    u_int magic;            /* '.snd' */
    u_int hdr_size;         /* size of header (min 24) */
    u_int data_size;        /* size of data */
    u_int encoding;         /* see to AU_FMT_XXXX */
    u_int sample_rate;      /* sample rate */
    u_int channels;         /* number of channels (voices) */
} AuHeader;

typedef struct {
    u_short format;         /* see WAV_FMT_* */
    u_short channels;
    u_int   sample_fq;        /* frequence of sample */
    u_int   byte_p_sec;
    u_short byte_p_spl;     /* samplesize; 1 or 2 bytes */
    u_short bit_p_spl;      /* 8, 12 or 16 bit */
} WaveFmtBody;



namespace WebCore {

class PlatformSpeechSynthesizer;
class PlatformSpeechSynthesisUtterance;
class PlatformSpeechSynthesisVoice;
class AplayWPE{

public:
    AplayWPE(PlatformSpeechSynthesisProviderWPE*);
    AplayWPE();
    ~AplayWPE();
    void playback(const char *);
    int  setVolumeChange(float );
private:
    PlatformSpeechSynthesisProviderWPE* m_speechSynthesisProviderWPE;
    int m_stopDelay;
    int m_wavFd;

    unsigned m_periodTime;
    unsigned m_bufferTime;
    size_t   m_bitsPerSample;
    size_t   m_bitsPerFrame;
    size_t   m_chunkBytes;

    snd_pcm_uframes_t m_periodFrames;
    snd_pcm_uframes_t m_bufferFrames;

    off64_t m_pbrecCount;
    off64_t m_fdcount;

    int     convertInteger(float, int, int);
    void    setHWParams(void);
    ssize_t pcmWrite(u_char *, size_t);
    ssize_t safeRead(int, void *, size_t);
    ssize_t checkWavefile(int, u_char *, size_t);
    void    playbackStart(int fd, size_t, off64_t, const char *);
    size_t  checkWavefileRead(int, u_char *, size_t *, size_t);

};

class FFmpegWPE {
public:
    FFmpegWPE();
    ~FFmpegWPE();
    int speedChangeFunction(float);
private:
    AVFormatContext *m_fmtCtx;
    AVCodecContext  *m_decCtx;
    AVFilterContext *m_bufferSinkCtx;
    AVFilterContext *m_bufferSrcCtx;
    AVFilterGraph   *m_filterGraph;

    FILE *        m_outFile;
    unsigned int  m_count;
    int           m_audioStreamIndex;
    const char *  m_filterDescr;

    void put16(int16_t);
    void put32(uint32_t);
    void dumpFrame(const AVFrame *);
    int  initFilters(const char*);
    int  openInputFile( const char *);
    void putWavHeader(int, int, int);

};

class PlatformSpeechSynthesisProviderWPE {
public:
    int m_canPause;
    int m_isPaused;
    int m_cancelled;

    snd_pcm_t *       m_handle;
    u_char *          m_audioBuf;
    snd_pcm_uframes_t m_chunkSize;

    explicit PlatformSpeechSynthesisProviderWPE(PlatformSpeechSynthesizer*);
    ~PlatformSpeechSynthesisProviderWPE();

    void initializeVoiceList(Vector<RefPtr<PlatformSpeechSynthesisVoice>>&);
    void pause();
    void resume();
    void speak(PassRefPtr<PlatformSpeechSynthesisUtterance>);
    void cancel();

    static void  speakFunctionThread (void*);

private:
    PlatformSpeechSynthesizer*               m_platformSpeechSynthesizer;
    RefPtr<PlatformSpeechSynthesisUtterance> m_utterance;

    ThreadIdentifier m_speakThread;
    snd_pcm_stream_t m_stream;
    enum SpeechEvent {
        SpeechError,
        SpeechCancel,
        SpeechPause,
        SpeechResume,
        SpeechStart,
        SpeechEnd
    };

    int  speechMain();
    void fireSpeechEvent(SpeechEvent);
    void doPause(void);
    void doResume(void);

};

} // namespace WebCore

#endif // ENABLE(SPEECH_SYNTHESIS)

#endif // PlatformSpeechSynthesisProviderWPE_h
