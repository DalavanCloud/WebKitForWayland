/*
 * Copyright (C) 2017 TATA ELXSI
 * Copyright (C) 2017 Metrological
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

#ifndef SOURCE_BACKEND_H_
#define SOURCE_BACKEND_H_

#include <poll.h>
#include <linux/dvb/dmx.h>
#include <libdvbapi/dvbfe.h>
#include <libdvbapi/dvbdemux.h>
#include <libucsi/dvb/section.h>
#include <libucsi/atsc/section.h>
#include <libucsi/atsc/types.h>
#include <libucsi/atsc/extended_channel_name_descriptor.h>
#include <signal.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <unistd.h>

#include "TVConfig.h"
#include "event-queue.h"
#include "ChannelBackend.h"

using namespace std;
#define TV_DEBUG 1 //TODO remove

struct TunerData {
    std::string tunerId;
    uint64_t modulation;
    vector<long> frequency;
};

namespace BCMRPi {
class TvControlBackend;

class SourceBackend {
public:
    SourceBackend(EventQueue<wpe_tvcontrol_event*>*, SourceType, TunerData*);
    ~SourceBackend();

    tvcontrol_return startScanning(bool isRescanned);
    tvcontrol_return stopScanning();
    tvcontrol_return setCurrentChannel(uint64_t channelNo);
    SourceType srcType() { return m_sType; }
    tvcontrol_return getChannels(wpe_tvcontrol_channel_vector** channelVector);

    wpe_tvcontrol_channel_vector m_channelVector;

private:
    void parseAtscExtendedChannelNameDescriptor(char** name, const unsigned char* buf);
    void startPlayBack(int frequency, uint64_t modulation, int pmtPid, int videoPid, int audioPid);
    void stopPlayBack();
    ChannelBackend* getChannelByLCN(uint64_t channelNo);
    bool tuneToFrequency(int frequency, uint64_t modulation, struct dvbfe_handle* feHandle);
    uint32_t getBits(const uint8_t* buf, int startbit, int bitlen);
    void atscScan(int frequency, uint64_t modulation);
    void mpegScan(int programNumber, std::map<int, int>& streamInfo);
    void dvbScan();
    bool processTVCT(int dmxfd, int frequency);
    bool processPAT(int patFd, int programNumber, struct pollfd* pollfd, std::map<int, int>& streamInfo);
    void processPMT(int pmtFd, std::map<int, int>& streamInfo);
    int createSectionFilter(uint16_t pid, uint8_t tableId);
    void clearChannelList();
    void clearChannelVector();
    void scanningThread();
    void setCurrentChannelThread();

    std::map<uint64_t, ChannelBackend*> m_channelList;
    EventQueue<wpe_tvcontrol_event*>* m_eventQueue;

    SourceType m_sType;
    TunerData* m_tunerData;
    thread m_scanningThread;
    thread m_setCurrentChannelThread;
    int m_adapter;
    int m_demux;

    int m_scanIndex;
    bool m_isRescanned;
    bool m_isScanStopped;
    bool m_isRunning;
    bool m_isScanInProgress;
    mutex m_scanMutex;
    condition_variable_any m_scanCondition;
    pid_t m_pid;

    uint64_t m_channelNo;
    mutex m_channelChangeMutex;
    condition_variable_any m_channelChangeCondition;
};

} // namespace BCMRPi
#endif // SOURCE_BACKEND_H_
