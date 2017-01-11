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

#include "TVConfig.h"
//#include <wpe/tvcontrol-backend.h>
#include "event-queue.h"
#include "ChannelBackend.h"

using namespace std;
#define TV_DEBUG 1 //TODO remove

struct TunerData {
    std::string     tunerId;
    uint64_t        modulation;
    vector<long>    frequency;
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
    void parseAtscExtendedChannelNameDescriptor(char **name, const unsigned char *buf);
    void startPlayBack(int frequency, uint64_t modulation, int pmtPid, int videoPid, int audioPid);
    void execute(char **argv);
    ChannelBackend* getChannelByLCN(uint64_t channelNo);
    bool tuneToFrequency(int frequency, uint64_t modulation, struct dvbfe_handle* feHandle);
    uint32_t getBits(const uint8_t *buf, int startbit, int bitlen);
    void atscScan(int frequency, uint64_t modulation);
    void mpegScan(int programNumber, std::map<int, int>& streamInfo);
    void dvbScan();
    bool processTVCT(int dmxfd, int frequency);
    bool processPAT(int patFd, int programNumber, struct pollfd *pollfd, std::map<int, int>& streamInfo);
    void processPMT(int pmtFd, std::map<int, int>& streamInfo);
    int  createSectionFilter(uint16_t pid, uint8_t tableId);
    void clearChannelList();
    void clearChannelVector();
    void scanningThread();
    void setCurrentChannelThread();

    std::map<uint64_t, ChannelBackend*> m_channelList;
    EventQueue<wpe_tvcontrol_event*>*   m_eventQueue;

    SourceType               m_sType;
    TunerData*               m_tunerData;
    thread                   m_scanningThread;
    thread                   m_setCurrentChannelThread;
    int                      m_adapter;
    int                      m_demux;

    int                      m_scanIndex;
    bool                     m_isRescanned;
    bool                     m_isScanStopped;
    bool                     m_isRunning;
    bool                     m_isScanInProgress;
    mutex                    m_scanMutex;
    condition_variable_any   m_scanCondition;
    pid_t                    m_pid;

    uint64_t                 m_channelNo;
    mutex                    m_channelChangeMutex;
    condition_variable_any   m_channelChangeCondition;
    bool                     m_currentPlaybackState;
};


} // namespace BCMRPi
#endif //SOURCE_BACKEND_H_
