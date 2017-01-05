#ifndef SOURCE_BACKEND_H_
#define SOURCE_BACKEND_H_

#include <poll.h>
#include "ChannelBackend.h"
#include <linux/dvb/dmx.h>
#include <libdvbapi/dvbfe.h>
#include <libdvbapi/dvbdemux.h>
#include <libucsi/dvb/section.h>
#include <libucsi/atsc/section.h>
#include <libucsi/atsc/types.h>
#include <libucsi/atsc/extended_channel_name_descriptor.h>
#include <signal.h>
#include "TVConfig.h"

using namespace std;
#define TV_DEBUG 1 //TODO remove

struct TunerData {
    std::string     tunerId;
    uint64_t        modulation;
    vector<long>    frequency;
};

namespace BCMRPi {

class SourceBackend {

public:
    SourceBackend(SourceType, TunerData*);
    virtual ~SourceBackend() {}

    tvcontrol_return startScanning(bool isRescanned);
    tvcontrol_return stopScanning();
    tvcontrol_return setCurrentChannel(uint64_t channelNo);
    tvcontrol_return getChannels(wpe_tvcontrol_channel_vector* channelVector);
    SourceType srcType() { return m_sType; }

private:
    void parseAtscExtendedChannelNameDescriptor(char **name, const unsigned char *buf);
    void startPlayBack(int frequency, uint64_t modulation, int pmtPid, int videoPid, int audioPid);
    void execute(char **argv);
    ChannelBackend* getChannelByLCN(uint64_t channelNo);
    bool tuneToFrequency(int frequency, uint64_t modulation, struct dvbfe_handle* feHandle);
    uint32_t getBits(const uint8_t *buf, int startbit, int bitlen);
    tvcontrol_return atscScan(int frequency, uint64_t modulation);
    void mpegScan(int programNumber, std::map<int, int>& streamInfo);
    tvcontrol_return dvbScan();
    bool  processTVCT(int dmxfd, int frequency);
    bool processPAT(int patFd, int programNumber, struct pollfd *pollfd, std::map<int, int>& streamInfo);
    void processPMT(int pmtFd, std::map<int, int>& streamInfo);
    int  createSectionFilter(uint16_t pid, uint8_t tableId);
    void clearChannelList();

    std::vector<ChannelBackend*> m_channelList;
    SourceType    m_sType;
    TunerData*    m_tunerData;
    int           m_adapter;
    int           m_demux;
    int           m_scanIndex;
    bool          m_isScanStopped;
    pid_t         m_pid;
};


} // namespace BCMRPi
#endif //SOURCE_BACKEND_H_
