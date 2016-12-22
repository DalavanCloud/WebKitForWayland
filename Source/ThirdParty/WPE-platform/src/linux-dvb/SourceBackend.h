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
#include "TVConfig.h"

using namespace std;
#define TV_DEBUG 1 //TODO remove

struct dvbfe_handle {
    int             fd;
    enum dvbfe_type type;
    char           *name;
    std::string     tunerId;
    uint64_t        modulation;
    vector<long>    frequency;
};

namespace BCMRPi {

class SourceBackend {

public:
    SourceBackend(SourceType, dvbfe_handle*);
    virtual ~SourceBackend() {}

    void startScanning();
    void stopScanning();
    void setCurrentChannel(uint64_t channelNo);
    void getChannels(wpe_tvcontrol_channel_vector* channelVector);
    SourceType srcType() { return m_sType; }

private:
    ChannelBackend* getChannelByLCN(uint64_t channelNo);
    bool tuneToFrequency(int frequency, uint64_t modulation);
    uint32_t getBits(const uint8_t *buf, int startbit, int bitlen);
    void atscScan(int frequency, uint64_t modulation);
    void mpegScan(int programNumber, std::map<int, int>& streamInfo);
    void dvbScan();
    int  processTVCT(int dmxfd, int frequency);
    bool processPAT(int patFd, int programNumber, struct pollfd *pollfd);
    void processPMT(int pmtFd, std::map<int, int>& streamInfo);
    int  createSectionFilter(uint16_t pid, uint8_t tableId);

    std::vector<ChannelBackend*> m_channelList;
    SourceType    m_sType;
    dvbfe_handle* m_feHandle;
    int           m_adapter;
    int           m_demux;
};


} // namespace BCMRPi
#endif //SOURCE_BACKEND_H_
