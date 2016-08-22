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

#include "config.h"
#if ENABLE(SPEECH_SYNTHESIS)

#ifndef FLITE_C_INCLUDED
#define FLITE_C_INCLUDED
extern "C" {
#include <flite_voice_list.c>
}
#endif

#include <NotImplemented.h>
#include <PlatformSpeechSynthesisUtterance.h>
#include <PlatformSpeechSynthesisVoice.h>
#include <PlatformSpeechSynthesizer.h>
#include <wtf/text/CString.h>
#include <wtf/MainThread.h>
#include <unistd.h>

#include <wtf/CurrentTime.h>
#include <wtf/MainThread.h>
#include <wtf/text/WTFString.h>
#include <byteswap.h>

#include "PlatformSpeechSynthesisProviderWPE.h"

#define checkWavefileSpace(buffer, len, blimit) \
    if (len > blimit) { \
        blimit = len; \
        if ((buffer = realloc(buffer, blimit)) == NULL) { \
            LOG(SpeechSynthesis, "ERROR Not enough memory");           \
        } \
    }

static struct {
    snd_pcm_format_t format;
    unsigned int     channels;
    unsigned int     rate;
} hwparams;

namespace WebCore {

PlatformSpeechSynthesisProviderWPE::PlatformSpeechSynthesisProviderWPE(PlatformSpeechSynthesizer* client)
    : m_canPause(0)
    , m_isPaused(0)
    , m_cancelled(0)
    , m_handle(NULL)
    , m_audioBuf(NULL)
    , m_chunkSize(0)
    , m_platformSpeechSynthesizer(client)
    , m_speakThread(0)
    , m_stream(SND_PCM_STREAM_PLAYBACK)
    {
        flite_init(); 
        LOG(SpeechSynthesis, "%s%s%d \n", __FILE__, __func__, __LINE__);
    }

PlatformSpeechSynthesisProviderWPE::~PlatformSpeechSynthesisProviderWPE()
{
    if (m_speakThread) {
        detachThread(m_speakThread);
        m_speakThread = 0;
    }
    delete_val(flite_voice_list);
    flite_voice_list = 0;
    m_cancelled = 0;
    LOG(SpeechSynthesis, "%s%s%d \n", __FILE__, __func__, __LINE__);
}

void PlatformSpeechSynthesisProviderWPE::initializeVoiceList(Vector<RefPtr<PlatformSpeechSynthesisVoice>>& voiceList)
{
    cst_voice *voice;
    const cst_val *v;
    WTF::String voiceId     = "default";
    WTF::String langSupport = "en-US";

    flite_voice_list = flite_set_voice_list();
    for (v = flite_voice_list; v; v = val_cdr(v)) {
        voice = val_voice(val_car(v));
        /*Convert the  voice to human readable form */

        String voiceName = voice->name;
        if (voiceName.contains("kal", true)) {
            LOG(SpeechSynthesis, "Kal - en-US - US English Male \n");
            langSupport = "en-US";
            voiceName = "US English Male";
        } else if (voiceName.contains("rms", true)) {
            LOG(SpeechSynthesis, "rms - en-US -US English Male \n");
            langSupport = "en-US";
            voiceName = "US English Male";
        } else if (voiceName.contains("awb", true)) {
            LOG(SpeechSynthesis, "awb - en-Scott - Scottish English Male \n");
            langSupport = "en-Scott";
            voiceName = "Scottish English Male";
        } else if (voiceName.contains("slt", true)) {
            LOG(SpeechSynthesis, "slt - en-US - US English Female \n");
            langSupport = "en-US";
            voiceName = "US English Female";
        } else {
            LOG(SpeechSynthesis, "Support for this language has to be added \n");
        }

        voiceList.append(PlatformSpeechSynthesisVoice::create(String(voiceId), voiceName, langSupport, true, true));
    }

}

void PlatformSpeechSynthesisProviderWPE::pause()
{
    LOG(SpeechSynthesis, "%s%s%d \n", __FILE__, __func__, __LINE__);
    if (!m_isPaused) {
       doPause();
       m_isPaused = 1;
       fireSpeechEvent(SpeechPause);
    } else {
        LOG(SpeechSynthesis, "Already in Paused state ");
    }
}

void PlatformSpeechSynthesisProviderWPE::resume()
{
    LOG(SpeechSynthesis, "%s%s%d \n", __FILE__, __func__, __LINE__);
    if (m_isPaused) {
        doResume();
        m_isPaused = 0;
        fireSpeechEvent(SpeechResume);
    } else {
          LOG(SpeechSynthesis, "Already in Resume state ");
    }
}

void PlatformSpeechSynthesisProviderWPE::speak(PassRefPtr<PlatformSpeechSynthesisUtterance> utteranceWrapper)
{
    m_utterance = utteranceWrapper;

    if (!m_speakThread) {
        if (!(m_speakThread = createThread(speakFunctionThread ,
              this, "WebCore: PlatformSpeechSynthesisProviderWPE"))) {
            LOG(SpeechSynthesis, "ERROR  in creating speaking  Thread\n");
        }
    } else {
            LOG(SpeechSynthesis, "Speak thread is  already created\n");
    }

}

void PlatformSpeechSynthesisProviderWPE::cancel()
{
    int err;

    if (m_cancelled == 0) {
        m_cancelled = 1;
        if (m_isPaused == 1) {
            err = snd_pcm_drop(m_handle); // To clear the buffer for pause->stop
            if (err < 0)
                LOG(SpeechSynthesis,  "ERROR  pause release error \n ");
        }
        if (m_speakThread) {
            waitForThreadCompletion(m_speakThread);
            m_speakThread = 0;
        }
        LOG(SpeechSynthesis, "%s%s%d \n", __FILE__, __func__, __LINE__);
        fireSpeechEvent(SpeechCancel);
        m_utterance = nullptr;
    } else {
            LOG(SpeechSynthesis, "ERROR Already Cancelled \n");
    }

}

void PlatformSpeechSynthesisProviderWPE::fireSpeechEvent(SpeechEvent speechEvent)
{
    switch (speechEvent) {
        case SpeechStart:
            m_platformSpeechSynthesizer->client()->didStartSpeaking(m_utterance);
            break;
        case SpeechPause:
            m_platformSpeechSynthesizer->client()->didPauseSpeaking(m_utterance);
            break;
        case SpeechResume:
            m_platformSpeechSynthesizer->client()->didResumeSpeaking(m_utterance);
            break;
        case SpeechError:
        case SpeechCancel:
            m_platformSpeechSynthesizer->client()->speakingErrorOccurred(m_utterance);
            break;
        case SpeechEnd:
            if (m_speakThread) {
                detachThread(m_speakThread);
                m_speakThread = 0;
            }
            m_platformSpeechSynthesizer->client()->didFinishSpeaking(m_utterance);
            break;
        default:
            ASSERT_NOT_REACHED();
    };
}

void PlatformSpeechSynthesisProviderWPE::speakFunctionThread(void* context)
{
    PlatformSpeechSynthesisProviderWPE *providerContext = (PlatformSpeechSynthesisProviderWPE*) context;
    AplayWPE aplay;
    FFmpegWPE ffmpeg;
    int status = 0;
    int  ret = 0;
    cst_voice *vs;
    char name[BUFF_SIZE];
    PlatformSpeechSynthesisVoice* sVoice;

    memset(name,0,sizeof(name));
    sVoice = providerContext->m_utterance->voice();
    if(sVoice) {
       /* switch  the voice type to FLITE  compatilble voice type  */
       if (!strcmp(sVoice->name().utf8().data(), "US English Male")) {
           LOG(SpeechSynthesis, "US English Male -> kal \n");
           strncpy(name, "kal", 3);
       } else if (!strcmp (sVoice->name().utf8().data(), "US English Female")) {
                LOG(SpeechSynthesis, "US English Female -> slt \n");
                strncpy(name, "slt", 3);
       } else if (!strcmp(sVoice->name().utf8().data(), "Scottish English Male")) {
                LOG(SpeechSynthesis, "Scottish English Male -> awb \n");
                strncpy(name,"awb", 3);
       } else {
          LOG(SpeechSynthesis, "WARNING :: Voice name  is not metioned.Setting to default value \n");
          strncpy(name, "kal", 3);
       }
    } else {
         LOG(SpeechSynthesis, "WARNING :: Voice name  is not metioned.Setting to default value \n");
         strncpy(name, "kal", 3);
    }
    flite_voice_list = flite_set_voice_list();
    if (flite_voice_list == NULL) {
        flite_set_voice_list();
    }
    LOG(SpeechSynthesis, "Going tro  set  %s : \n",name);

    /* Set Voice type */
    vs = flite_voice_select(name);
    if (vs == 0)
    vs = flite_voice_select(NULL);

    /* Creating Wav File of the text */
    flite_text_to_speech(providerContext->m_utterance->text().utf8().data(), vs, INTERMEDIATE_WAV_FILE);

    /* Volume change */
    ret=aplay.setVolumeChange(providerContext->m_utterance->volume());
    if (ret < 0)
        LOG(SpeechSynthesis, "Failed to set volume \n ");

    ret=ffmpeg.speedChangeFunction(providerContext->m_utterance->rate());
    if (ret < 0)
        LOG(SpeechSynthesis, "Failed to set speed \n ");
    providerContext->m_cancelled = 0;
    providerContext->m_isPaused  = 0;

    /* Play back Wav File */
    providerContext->speechMain();
    status = remove(INTERMEDIATE_WAV_FILE);
    if (status) {
        LOG(SpeechSynthesis, "Unable to delete the intermediate WAV file \n");
        perror("Error");
    }
    status = remove(FINAL_WAV_FILE);
    if (status) {
        LOG(SpeechSynthesis, "Unable to delete the final WAV file \n");
        perror("Error");
    }

    if (providerContext->m_cancelled == 0)
        providerContext->fireSpeechEvent(SpeechEnd);
}

void PlatformSpeechSynthesisProviderWPE::doPause(void)
{
    int err;
    if (!m_canPause) {
        fprintf(stderr, ("\rPAUSE command ignored (no hw support)\n"));
        return;
    }
    err = snd_pcm_pause(m_handle, 1);
    if (err < 0) {
        LOG(SpeechSynthesis, "%s ERROR Pause push error \n", snd_strerror(err));
        return;
    }
}

void PlatformSpeechSynthesisProviderWPE::doResume(void)
{
    int err;
    if (!m_canPause) {
        LOG(SpeechSynthesis, "ERROR RESUME command ignored (no hw support)\n");
        return;
    }
    err = snd_pcm_pause(m_handle, 0);
    if (err < 0)
        LOG(SpeechSynthesis, "ERROR pause release error \n ");
    return;
}

/* Aplay playback */
int PlatformSpeechSynthesisProviderWPE::speechMain()
{
    int err;
    AplayWPE aplay(this);
    const char *pcm_name = AUDIO_DEVICE;

    hwparams.rate   = DEFAULT_SPEED;
    hwparams.format = DEFAULT_FORMAT;
    hwparams.channels = 1;
    err = snd_pcm_open(&m_handle, pcm_name, m_stream, OPENMODE);
    if (err < 0) {
        LOG(SpeechSynthesis, "%s ERROR Audio open error \n ", snd_strerror(err));
        return 1;
    }

    m_chunkSize = A_CHUNK;
    m_audioBuf = (u_char *)malloc(A_CHUNK);
    if (m_audioBuf == NULL) {
        LOG(SpeechSynthesis, "ERROR not enough memory \n");
        return 1;
    }

    /* Begin playback */
    fireSpeechEvent(SpeechStart);
    aplay.playback(FINAL_WAV_FILE);

    /* Exit playback and release audio buffer */
    snd_pcm_close(m_handle);
    m_handle = NULL;
    free(m_audioBuf);
    snd_config_update_free_global();
    return EXIT_SUCCESS;
}

AplayWPE::AplayWPE(PlatformSpeechSynthesisProviderWPE* provider)
    : m_speechSynthesisProviderWPE(provider)
    , m_stopDelay(0)
    , m_wavFd(-1)
    , m_periodTime(0)
    , m_bufferTime(0)
    , m_periodFrames(0)
    , m_bufferFrames(0)
    , m_pbrecCount(LLONG_MAX)
    , m_fdcount(0)
{
}

AplayWPE::AplayWPE()
{
}

AplayWPE::~AplayWPE()
{
}

ssize_t AplayWPE::safeRead(int fd, void *buf, size_t count)
{
    ssize_t result = 0, res;

    while (count > 0  && (m_speechSynthesisProviderWPE->m_cancelled == 0) ) {
        if ((res = read(fd, buf, count)) == 0)
             break;
        if (res < 0)
            return result > 0 ? result : res;
        count -= res;
        result += res;
        buf = (char *)buf + res;
    }
    return result;
}

size_t AplayWPE::checkWavefileRead(int fd, u_char *buffer, size_t *size, size_t reqsize)
{
    if (*size >= reqsize)
        return *size;
    if ((size_t)safeRead(fd, buffer + *size, reqsize - *size) != reqsize - *size) {
        LOG(SpeechSynthesis, "ERROR Safe read failed  error \n");
    }
    return *size = reqsize;
}

ssize_t AplayWPE::checkWavefile(int fd, u_char *_buffer, size_t size)
{
    WaveHeader *h = (WaveHeader *)_buffer;
    void *buffer = NULL;
    size_t blimit = 0;
    WaveChunkHeader *c;
    u_int type, len;
    int bigEndian, nativeFormat;
    WaveFmtBody *f; /*for rate control of  stream */

    if (size < sizeof(WaveHeader))
        return -1;
    if (h->magic == WAV_RIFF) {
        bigEndian = 0;
    }
    if (size > sizeof(WaveHeader)) {
        checkWavefileSpace(buffer, size - sizeof(WaveHeader), blimit);
        memcpy((u_char *)buffer, _buffer + sizeof(WaveHeader), size - sizeof(WaveHeader));
    }
    size -= sizeof(WaveHeader);
    while (1) {
        checkWavefileSpace(buffer, sizeof(WaveChunkHeader), blimit);
        checkWavefileRead(fd, (u_char*)buffer, &size, sizeof(WaveChunkHeader));
        c = (WaveChunkHeader*)buffer;
        type = c->type;
        len = TO_CPU_INT(c->length, bigEndian);
        len += len % 2;
        if (size > sizeof(WaveChunkHeader))
            memmove((u_char*)buffer, (u_char*)buffer + sizeof(WaveChunkHeader), size - sizeof(WaveChunkHeader));
        size -= sizeof(WaveChunkHeader);
        if (type == WAV_FMT)
            break;
        checkWavefileSpace(buffer, len, blimit);
        checkWavefileRead(fd, (u_char *)buffer, &size, len);
        if (size > len)
            memmove((u_char *)buffer, (u_char *)buffer + len, size - len);
        size -= len;
    }
    checkWavefileSpace(buffer, len, blimit);
    checkWavefileRead(fd, (u_char *)buffer, &size, len);
    nativeFormat = SND_PCM_FORMAT_S16_LE; //format
    if (hwparams.format != DEFAULT_FORMAT &&
                hwparams.format != nativeFormat)
        fprintf(stderr, ("Warning: format is changed to %s\n"),
                    snd_pcm_format_name((snd_pcm_format_t)nativeFormat));    
    hwparams.format = (snd_pcm_format_t)nativeFormat;
    f = (WaveFmtBody*) buffer;
    hwparams.rate = TO_CPU_INT(f->sample_fq, bigEndian); //setiing the rate param for voices

    if (size > len)
        memmove((u_char *)buffer, (u_char *)buffer + len, size - len);
    size -= len;

    while (1) {
        LOG(SpeechSynthesis, "%s%s%d \n", __FILE__, __func__, __LINE__);
        u_int type, len;
        checkWavefileSpace(buffer, sizeof(WaveChunkHeader), blimit);
        checkWavefileRead(fd, (u_char *)buffer, &size, sizeof(WaveChunkHeader));

        c = (WaveChunkHeader*)buffer;
        type = c->type;
        len = TO_CPU_INT(c->length, bigEndian);

        if (size > sizeof(WaveChunkHeader))
             memmove((u_char *)buffer, (u_char *)buffer + sizeof(WaveChunkHeader),
                      size - sizeof(WaveChunkHeader));
        size -= sizeof(WaveChunkHeader);

        if (type == WAV_DATA) {
            if (len < m_pbrecCount && len < 0x7ffffffe)
                m_pbrecCount = len;
            if (size > 0)
                memcpy(_buffer, buffer, size);
            free(buffer);
            return size;
        }

        len += len % 2;
        checkWavefileSpace(buffer, len, blimit);
        checkWavefileRead(fd, (u_char *)buffer, &size, len);

        if (size > len)
            memmove((u_char *)buffer, (u_char *)buffer + len, size - len);

        size -= len;
    }

    return -1;
}

void AplayWPE::setHWParams(void)  //TODO R-ework  needs to done to the function
{                                  //to integrate utterence attributes -Pending Cleanup
    int err;
    size_t n;
    unsigned int rate;
    char plugex[64];
    const char *pcmname ;

    snd_pcm_hw_params_t *params;
    snd_pcm_sw_params_t *swparams;
    snd_pcm_uframes_t bufferSize;
    snd_pcm_uframes_t startThreshold, stopThreshold;
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_sw_params_alloca(&swparams);

    /* Set hardware params */
    err = snd_pcm_hw_params_any(m_speechSynthesisProviderWPE->m_handle, params);
    if (err < 0) {
        LOG(SpeechSynthesis, "ERROR Broken configuration for this PCM: no configurations available \n");
    }

    err = snd_pcm_hw_params_set_access(m_speechSynthesisProviderWPE->m_handle, params,
                                           SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err < 0) {
        LOG(SpeechSynthesis, "ERROR Access type not available \n");
    }

    err = snd_pcm_hw_params_set_format(m_speechSynthesisProviderWPE->m_handle, params, hwparams.format);
    if (err < 0) {
        LOG(SpeechSynthesis, "ERROR Sample format non available \n");
    }

    err = snd_pcm_hw_params_set_channels(m_speechSynthesisProviderWPE->m_handle, params, hwparams.channels);
    if (err < 0) {
        LOG(SpeechSynthesis, "ERROR Channels count non available \n");
    }
    rate = hwparams.rate;
    err = snd_pcm_hw_params_set_rate_near(m_speechSynthesisProviderWPE->m_handle, params, &hwparams.rate, 0);
    assert(err >= 0);
    if ((float)rate * 1.05 < hwparams.rate || (float)rate * 0.95 > hwparams.rate) {
        pcmname = snd_pcm_name(m_speechSynthesisProviderWPE->m_handle);
        fprintf(stderr, ("WARNING: rate is not accurate (requested = %iHz, got = %iHz)\n"), 
                         rate, hwparams.rate);
        if (! pcmname || strchr(snd_pcm_name(m_speechSynthesisProviderWPE->m_handle), ':'))
            *plugex = 0;
        else
            snprintf(plugex, sizeof(plugex), "(-Dplug:%s)",
                     snd_pcm_name(m_speechSynthesisProviderWPE->m_handle));
        fprintf(stderr, ("ERROR please, try the plug plugin %s\n"),
                    plugex);
    }
    rate = hwparams.rate;
    if (m_bufferTime == 0 && m_bufferFrames == 0) {
        err = snd_pcm_hw_params_get_buffer_time_max(params,
                    &m_bufferTime, 0);
        assert(err >= 0);
        if (m_bufferTime > 500000)
            m_bufferTime = 500000;
    }
    if (m_periodTime == 0 && m_periodFrames == 0) {
        if (m_bufferTime > 0)
            m_periodTime = m_bufferTime / 4;
        else
            m_periodFrames = m_bufferFrames / 4;
    }

    if (m_periodTime > 0)
        err = snd_pcm_hw_params_set_period_time_near(m_speechSynthesisProviderWPE->m_handle, params,
                    &m_periodTime, 0);
    else
        err = snd_pcm_hw_params_set_period_size_near(m_speechSynthesisProviderWPE->m_handle, params,
                    &m_periodFrames, 0);

    assert(err >= 0);
    if (m_bufferTime > 0) {
        err = snd_pcm_hw_params_set_buffer_time_near(m_speechSynthesisProviderWPE->m_handle, params,
                    &m_bufferTime, 0);
    } else {
         err = snd_pcm_hw_params_set_buffer_size_near(m_speechSynthesisProviderWPE->m_handle, params,
                    &m_bufferFrames);
    }

    assert(err >= 0);

    m_speechSynthesisProviderWPE->m_canPause = snd_pcm_hw_params_can_pause(params);
    err = snd_pcm_hw_params(m_speechSynthesisProviderWPE->m_handle, params);
    if (err < 0) {
        LOG(SpeechSynthesis, "ERROR Unable to install hw params: \n");
    }
    snd_pcm_hw_params_get_period_size(params, &(m_speechSynthesisProviderWPE->m_chunkSize), 0);//ref
    snd_pcm_hw_params_get_buffer_size(params, &bufferSize);
    if (m_speechSynthesisProviderWPE->m_chunkSize == bufferSize) {
        LOG(SpeechSynthesis, "(%lu == %lu) ERROR Use period equal to buffer size \n",
                m_speechSynthesisProviderWPE->m_chunkSize, bufferSize);
    }

    /* Set software params */
    snd_pcm_sw_params_current(m_speechSynthesisProviderWPE->m_handle, swparams);
    n = m_speechSynthesisProviderWPE->m_chunkSize;
    err = snd_pcm_sw_params_set_avail_min(m_speechSynthesisProviderWPE->m_handle, swparams, n);

    /* round up to closest transfer boundary */
    n = bufferSize;
    startThreshold = n / 1000000;
    if (startThreshold < 1)
        startThreshold = 1;
    if (startThreshold > n)
        startThreshold = n;

    err = snd_pcm_sw_params_set_start_threshold(m_speechSynthesisProviderWPE->m_handle, 
                                                swparams, startThreshold);
    assert(err >= 0);
    if (m_stopDelay <= 0)
        stopThreshold = bufferSize + (double) rate * m_stopDelay / 1000000;
    else
        stopThreshold = (double) rate * m_stopDelay / 1000000;
    err = snd_pcm_sw_params_set_stop_threshold(m_speechSynthesisProviderWPE->m_handle, 
                                               swparams, stopThreshold);
    assert(err >= 0);
    if (snd_pcm_sw_params(m_speechSynthesisProviderWPE->m_handle, swparams) < 0) {
        LOG(SpeechSynthesis, "ERROR Unable to install sw params \n");
    }

    /* Update the currently set  params */
    m_bitsPerSample = snd_pcm_format_physical_width(hwparams.format);
    m_bitsPerFrame = m_bitsPerSample * hwparams.channels;
    m_chunkBytes = (m_speechSynthesisProviderWPE->m_chunkSize)* m_bitsPerFrame / 8;
    m_speechSynthesisProviderWPE->m_audioBuf = (u_char*)realloc(m_speechSynthesisProviderWPE->m_audioBuf, 
                                                                m_chunkBytes);

    if (m_speechSynthesisProviderWPE->m_audioBuf == NULL) {
        LOG(SpeechSynthesis, "ERROR :: Not enough memory \n");
    }
    m_bufferFrames = bufferSize;    /* for position test */
}

ssize_t AplayWPE::pcmWrite(u_char *data, size_t count)
{
    ssize_t r;
    ssize_t result = 0;

    if (count < m_speechSynthesisProviderWPE->m_chunkSize) {
        snd_pcm_format_set_silence(hwparams.format, data + count * m_bitsPerFrame / 8,
                            (m_speechSynthesisProviderWPE->m_chunkSize - count) * hwparams.channels);
        count = m_speechSynthesisProviderWPE->m_chunkSize;
    }
    while (count > 0 && (m_speechSynthesisProviderWPE->m_cancelled == 0)  ) {
        r = snd_pcm_writei(m_speechSynthesisProviderWPE->m_handle, data, count);
        if (r == -EAGAIN || (r >= 0 && (size_t)r < count)) {
            snd_pcm_wait(m_speechSynthesisProviderWPE->m_handle, 100);
        }
        if (r > 0) {
            result += r;
            count -= r;
            data += r * m_bitsPerFrame / 8;
        }
    }
    return result;
}

void AplayWPE::playbackStart(int fd, size_t loaded, off64_t count, const char *name)
{
    int l, r;
    off64_t c, written = 0;

    setHWParams();
    while (loaded > m_chunkBytes && written < count ) {
        if (pcmWrite(m_speechSynthesisProviderWPE->m_audioBuf + written,
                         m_speechSynthesisProviderWPE->m_chunkSize) <= 0)
            return;
        written += m_chunkBytes;
        loaded -= m_chunkBytes;
    }
    if (written > 0 && loaded > 0)
            memmove(m_speechSynthesisProviderWPE->m_audioBuf,
                    m_speechSynthesisProviderWPE->m_audioBuf + written, loaded);

    l = loaded;
    while (written < count && (m_speechSynthesisProviderWPE->m_cancelled == 0) ) {
        do {
            c = count - written;
            if (c > m_chunkBytes)
                c = m_chunkBytes;
            c -= l;
            if (c == 0)
                break;
            r = safeRead(fd,m_speechSynthesisProviderWPE->m_audioBuf + l, c);
            if (r < 0) {
                perror(name);
            }

            m_fdcount += r;
            if (r == 0)
                break;
            l += r;

        } while ((size_t)l < m_chunkBytes);
        l = l * 8 / m_bitsPerFrame;
        r = pcmWrite(m_speechSynthesisProviderWPE->m_audioBuf, l);

        if (r != l)
            break;
        r = r * m_bitsPerFrame / 8;
        written += r;
        l = 0;
    }
    snd_pcm_nonblock(m_speechSynthesisProviderWPE->m_handle, BLOCK);
    snd_pcm_drain(m_speechSynthesisProviderWPE->m_handle);
    snd_pcm_nonblock(m_speechSynthesisProviderWPE->m_handle, BLOCK);
}

void AplayWPE::playback(const char *name)
{
    size_t dta;
    ssize_t dtaWave;

    if ((m_wavFd = open(name, O_RDONLY, 0)) == -1) {
        LOG(SpeechSynthesis, "ERROR Failed to open the wav file \n");
        perror(name);
    }

    /* read the file header and validate it is wav file*/
    dta = sizeof(AuHeader);
    if ((size_t)safeRead(m_wavFd, m_speechSynthesisProviderWPE->m_audioBuf, dta) != dta) {
        LOG(SpeechSynthesis, "ERROR Failed to read audio header \n");
    }

    dta = sizeof(VocHeader);
    if ((size_t)safeRead(m_wavFd, m_speechSynthesisProviderWPE->m_audioBuf + sizeof(AuHeader),
                         dta - sizeof(AuHeader)) != dta - sizeof(AuHeader)) {
        LOG(SpeechSynthesis, "ERROR Failed to read voice header \n");
    }

    /* read bytes for WAVE-header */
    if ((dtaWave = checkWavefile(m_wavFd, m_speechSynthesisProviderWPE->m_audioBuf, dta)) >= 0) {
        playbackStart(m_wavFd, dtaWave, m_pbrecCount, name);
    }
    if (m_wavFd != 0)
        close(m_wavFd);
}

int AplayWPE::setVolumeChange(float volv)
{
    int err     = 0;
    int integer = 0;
    int pmin    = 0;
    int pmax    = 0;
    int itr     = 0;

    char buf[LOCATION];
    const char index[INDEX] = "1";//TODO select device dynamically
    static char card[NUMBERS] = "default";
    static snd_ctl_t *handle = NULL;

    snd_ctl_elem_info_t *info;
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_value_t *control;
    snd_ctl_elem_info_alloca(&info);
    snd_ctl_elem_id_alloca(&id);
    snd_ctl_elem_value_alloca(&control);
    snd_hctl_t *hctl;

    itr = snd_card_get_index(index);
    if (itr >= 0 && itr < ITERATIONS) {
        sprintf(card, "hw:%i", itr);
    } else {
       fprintf(stderr, "Invalid card number.\n");
    }

    if (snd_ctl_ascii_elem_id_parse(id, "name=Speaker Playback Volume")) {
        fprintf(stderr, "Wrong control identifier:\n" );
        return -EINVAL;
    }
    if (handle == NULL &&
        (err = snd_ctl_open(&handle, card, 0)) < 0) {
        LOG(SpeechSynthesis, "Control %s open error: %s\n", card, snd_strerror(err));
        return err;
    }
    snd_ctl_elem_info_set_id(info, id);
    if ((err = snd_ctl_elem_info(handle, info)) < 0) {
         LOG(SpeechSynthesis, "Cannot find the given element from control %s\n",card);
         snd_ctl_close(handle);
         handle = NULL;
         return err;
    }
    snd_ctl_elem_info_get_id(info, id);
    snd_ctl_elem_value_set_id(control, id);
    if ((err = snd_ctl_elem_read(handle, control)) < 0) {
         LOG(SpeechSynthesis, "Cannot read the given element from control %s\n", card);
         snd_ctl_close(handle);
         handle = NULL;
         return err;
    }

    pmin    = snd_ctl_elem_info_get_min(info);
    pmax    = snd_ctl_elem_info_get_max(info);
    integer = convertInteger(volv, pmax, pmin);
    sprintf(buf, "%d", integer);

    err = snd_ctl_ascii_value_parse(handle, control, info, buf);
    if (err < 0) {
        LOG(SpeechSynthesis, "Control %s parse error: %s\n", card, snd_strerror(err));
        snd_ctl_close(handle);
        handle = NULL;
        return err;
    }

    if ((err = snd_ctl_elem_write(handle, control)) < 0) {
         LOG(SpeechSynthesis, "Control %s element write error: %s\n", card, snd_strerror(err));
         snd_ctl_close(handle);
         handle = NULL;
         return err;
    }
    snd_ctl_close(handle);
    handle = NULL;
    if ((err = snd_hctl_open(&hctl, card, 0)) < 0) {
         LOG(SpeechSynthesis, "Control %s open error: %s\n", card, snd_strerror(err));
         return err;
    }
    if ((err = snd_hctl_load(hctl)) < 0) {
         LOG(SpeechSynthesis, "Control %s load error: %s\n", card, snd_strerror(err));
         return err;
    }
    snd_hctl_close(hctl);
    return 0;
}

int AplayWPE::convertInteger(float oldval, int newmax, int newmin)
{
    int oldrange = 0;
    int oldmax   = 1;
    int oldmin   = 0;
    int newvalue = 0;
    int newrange = 0;

    oldrange = oldmax-oldmin;
    newrange = newmax-newmin;
    newvalue = (((oldval - oldmin) * newrange) / oldrange) + newmin;
    return newvalue;
}

FFmpegWPE::FFmpegWPE()
    : m_fmtCtx(NULL)
    , m_decCtx(NULL)
    , m_bufferSinkCtx(NULL)
    , m_bufferSrcCtx(NULL)
    , m_filterGraph(NULL)
    , m_count(0)
    , m_audioStreamIndex(-1)
    , m_filterDescr(NULL)
{
}

void FFmpegWPE::put16(int16_t v)
{
    fputc( v       & 0xff, m_outFile);
    fputc((v >> 8) & 0xff, m_outFile);
}

void FFmpegWPE::put32(uint32_t v)
{
    fputc( v        & 0xff, m_outFile);
    fputc((v >>  8) & 0xff, m_outFile);
    fputc((v >> 16) & 0xff, m_outFile);
    fputc((v >> 24) & 0xff, m_outFile);
}

int FFmpegWPE::openInputFile(const char *filename)
{
    int ret = 0;
    AVCodec *dec;

    /* Open intermediate WAV file */
    if ((ret = avformat_open_input(&m_fmtCtx, filename, NULL, NULL)) < 0) {
        LOG(SpeechSynthesis, "Cannot open input file\n");
        return ret;
    }

    if ((ret = avformat_find_stream_info(m_fmtCtx, NULL)) < 0) {
        LOG(SpeechSynthesis, "Cannot find stream information\n");
        return ret;
    }

    /* Select the audio stream */
    ret = av_find_best_stream(m_fmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, &dec, 0);
    if (ret < 0) {
        LOG(SpeechSynthesis, "Cannot find a audio stream in the input file\n");
        return ret;
    }
    m_audioStreamIndex = ret;
    m_decCtx = (m_fmtCtx)->streams[m_audioStreamIndex]->codec;
    av_opt_set_int(m_decCtx, "refcounted_frames", 1, 0);

    /* Init the audio decoder */
    if ((ret = avcodec_open2(m_decCtx, dec, NULL)) < 0) {
        LOG(SpeechSynthesis, "Cannot open audio decoder\n");
        return ret;
    }

    return ret;
}

int FFmpegWPE::initFilters(const char *filtersDescr)
{
    char args[512];
    int ret = 0;

    AVFilter *abuffersrc   = avfilter_get_by_name("abuffer");
    AVFilter *abuffersink  = avfilter_get_by_name("abuffersink");
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs  = avfilter_inout_alloc();
    AVRational time_base   = (m_fmtCtx)->streams[m_audioStreamIndex]->time_base;

    const AVFilterLink *outlink;
    /* Set parameters for output wave file */
    static const enum    AVSampleFormat out_sample_fmts[] = { AV_SAMPLE_FMT_S16,AVSampleFormat(-1) };
    static const int64_t out_channel_layouts[] = { AV_CH_LAYOUT_MONO, -1 };
    static const int     out_sample_rates[] = { 8000, -1 };

    m_filterGraph = avfilter_graph_alloc();
    if (!outputs || !inputs || !m_filterGraph) {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    /* buffer audio source: the decoded frames from the decoder will be inserted here. */
    if (!m_decCtx->channel_layout)
        m_decCtx->channel_layout = av_get_default_channel_layout(m_decCtx->channels);
    std::snprintf(args, sizeof(args),
                  "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%" PRIx64,
                  time_base.num, time_base.den, m_decCtx->sample_rate,
                  av_get_sample_fmt_name(m_decCtx->sample_fmt), m_decCtx->channel_layout);

    ret = avfilter_graph_create_filter(&m_bufferSrcCtx, abuffersrc, "in",
                                       args, NULL, m_filterGraph);
    if (ret < 0) {
        LOG(SpeechSynthesis, "Cannot create audio buffer source\n");
        goto end;
    }

    /* buffer audio sink: to terminate the filter chain. */
    ret = avfilter_graph_create_filter(&m_bufferSinkCtx, abuffersink, "out",
                                         NULL, NULL, m_filterGraph);
    if (ret < 0) {
        LOG(SpeechSynthesis, "Cannot create audio buffer sink\n");
        goto end;
    }

    ret = av_opt_set_int_list(m_bufferSinkCtx, "sample_fmts", out_sample_fmts,
                              -1, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        LOG(SpeechSynthesis, "Cannot set output sample format\n");
        goto end;
    }

    ret = av_opt_set_int_list(m_bufferSinkCtx, "channel_layouts", out_channel_layouts,
                              -1, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        LOG(SpeechSynthesis, "Cannot set output channel layout\n");
        goto end;
    }

    ret = av_opt_set_int_list(m_bufferSinkCtx, "sample_rates", out_sample_rates,
                              -1, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        LOG(SpeechSynthesis,  "Cannot set output sample rate\n");
        goto end;
    }

    /*
     * Set the endpoints for the filter graph. The filter_graph will
     * be linked to the graph described by filtersDescr.
     * The buffer source output must be connected to the input pad of
     * the first filter described by filtersDescr; since the first
     * filter input label is not specified, it is set to "in" by
     * default.
     */
    outputs->name       = av_strdup("in");
    outputs->filter_ctx = m_bufferSrcCtx;
    outputs->pad_idx    = 0;
    outputs->next       = NULL;

    /*
     * The buffer sink input must be connected to the output pad of
     * the last filter described by filtersDescr; since the last
     * filter output label is not specified, it is set to "out" by
     * default.
     */
    inputs->name       = av_strdup("out");
    inputs->filter_ctx = m_bufferSinkCtx;
    inputs->pad_idx    = 0;
    inputs->next       = NULL;

    if ((ret = avfilter_graph_parse_ptr(m_filterGraph, filtersDescr,
                                        &inputs, &outputs, NULL)) < 0)
        goto end;

    if ((ret = avfilter_graph_config(m_filterGraph, NULL)) < 0)
        goto end;

    /* Print summary of the sink buffer
     * Note: args buffer is reused to store channel layout string
     */

    outlink = m_bufferSinkCtx->inputs[0];
    av_get_channel_layout_string(args, sizeof(args), -1, outlink->channel_layout);

    LOG(SpeechSynthesis,  "Output: srate:%dHz fmt:%s chlayout:%s\n",
           (int)outlink->sample_rate,
           (char *)av_x_if_null(av_get_sample_fmt_name(AVSampleFormat(outlink->format)),
           "?"),args);
end:
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);
    return ret;
}

void FFmpegWPE::dumpFrame(const AVFrame *frame)
{
    const int n = frame->nb_samples * av_get_channel_layout_nb_channels(av_frame_get_channel_layout(frame));

    const uint16_t *p     = (uint16_t*)frame->data[0];
    const uint16_t *pEnd  = p + n;

    while (p < pEnd) {

        fputc(*p    & 0xff, m_outFile);
        m_count++;
        fputc(*p>>8 & 0xff, m_outFile);
        m_count++;
        p++;
    }
}

void FFmpegWPE::putWavHeader(int sampleRate, int channels, int nbSamples)
{
    int blockAlign = SAMPLE_SIZE * channels;
    int dataSize   = blockAlign * nbSamples;

    fputs("RIFF", m_outFile);
    put32(HEADER_SIZE + dataSize); /*Total chunck size*/
    fputs("WAVEfmt ", m_outFile);
    put32(FORMAT_SIZE);              /*Format chunk size*/
    put16(AUDIO_FORMAT_PCM);           /*Audio type*/
    put16(channels);
    put32(sampleRate);
    put32(blockAlign * sampleRate);
    put16(blockAlign);
    put16(SAMPLE_SIZE * 8);
    put16(0);
    fputs("data", m_outFile);
    put32(dataSize);              /*data chunck size*/
}

int FFmpegWPE::speedChangeFunction(float speed)
{
    int ret        =  0;
    int gotFrame   =  0;
    int sampleRate = 8000;  /*8000HZ*/
    int nbChannels = 1;     /*mono*/
    int count_46   = 0;
    int count_8    = 0;
    float value    = speed;

    struct stat st;
    std::string rateString,filterDescrDummy,filterDescr;
    AVPacket packet0, packet;

    AVFrame *frame      = av_frame_alloc();
    AVFrame *filt_frame = av_frame_alloc();

    av_register_all();
    avfilter_register_all();

    /* Create Filter descriptor */
    filterDescr= ",aresample=8000,aformat=sample_fmts=s16:channel_layouts=mono"; /*Constant params*/
    rateString = std::to_string(value);
    LOG(SpeechSynthesis, "%s is converted value of speed \nFILTER: ", rateString.c_str());

    if (value <= 0.5) {
        filterDescr.insert(0, "atempo=0.5");
    } else if (value > 0.5 && value <= 2) {
        filterDescrDummy.insert(0, "atempo=");
        filterDescrDummy.append(rateString);
        filterDescr.insert(0, filterDescrDummy);
    } else {
        filterDescr.insert(0, "atempo=2,atempo=2");
    }

    LOG(SpeechSynthesis, "%s\n", filterDescr.c_str());
    m_filterDescr = filterDescr.c_str();

    if ((ret = openInputFile(INTERMEDIATE_WAV_FILE)) < 0)
        goto end;
    if ((ret = initFilters(m_filterDescr)) < 0)
        goto end;

    /* read all packets */
    packet0.data = NULL;
    packet.data  = NULL;

    m_outFile = fopen(FINAL_WAV_FILE, "wb");
    putWavHeader(sampleRate, nbChannels, 6 * sampleRate);

    while (1) {
        if (!packet0.data) {
            if ((ret = av_read_frame(m_fmtCtx, &packet)) < 0)
                break;
            packet0 = packet;
        }

        if (packet.stream_index == m_audioStreamIndex) {
            gotFrame = 0;
            ret = avcodec_decode_audio4(m_decCtx, frame, &gotFrame, &packet);
            if (ret < 0) {
                LOG(SpeechSynthesis, "Error decoding audio\n");
                continue;
            }
            packet.size -= ret;
            packet.data += ret;

            if (gotFrame) {
                /* push the audio data from decoded frame into the filtergraph */
                if (av_buffersrc_add_frame_flags(m_bufferSrcCtx, frame, 0) < 0) {
                    LOG(SpeechSynthesis, "Error while feeding the audio filtergraph\n");
                    break;
                }

                /* pull filtered audio from the filtergraph */
                while (1) {
                    ret = av_buffersink_get_frame(m_bufferSinkCtx, filt_frame);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                        break;
                    if (ret < 0)
                        goto end;

                    dumpFrame(filt_frame);
                    av_frame_unref(filt_frame);
                }
            }
            if (packet.size <= 0)
                av_free_packet(&packet0);
        } else {
            /* discard non-wanted packets */
            av_free_packet(&packet0);
        }
    }

    /* size of wave file  */
    stat(FINAL_WAV_FILE,&st);
    m_count  = st.st_size;
    count_46 = m_count - HEADER_SIZE;
    count_8  = m_count - HEADER_DESC;

    /*Open wave file and update header*/

    fseek(m_outFile, CHUNK_SIZE_POS, SEEK_SET);
    fwrite(&count_8, sizeof(int), 1, m_outFile);

    fseek(m_outFile, DATA_SIZE_POS, SEEK_SET);
    fwrite(&count_46, sizeof(int), 1, m_outFile);

    fclose(m_outFile);

end:
    avfilter_graph_free(&m_filterGraph);
    avcodec_close(m_decCtx);
    avformat_close_input(&m_fmtCtx);
    av_frame_free(&frame);
    av_frame_free(&filt_frame);
    return ret;
}

FFmpegWPE::~FFmpegWPE()
{

}

} // namespace WebCore

#endif // ENABLE(SPEECH_SYNTHESIS)
