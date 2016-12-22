#ifndef SOURCE_BACKEND_H_
#define SOURCE_BACKEND_H_

#include <string>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <fcntl.h>
#include <wpe/tvcontrol-backend.h>
#include <vector>
#include <map>
#include "ChannelBackend.h"
#include <linux/dvb/frontend.h>
#include <linux/dvb/dmx.h>
#include <libdvbapi/dvbfe.h>
#include <libdvbapi/dvbdemux.h>
#include <libucsi/dvb/section.h>
#include <libucsi/atsc/section.h>
#include <libucsi/atsc/types.h>
#include <libucsi/atsc/extended_channel_name_descriptor.h>

using namespace std;
#define TV_DEBUG 1 //TODO remove

/******************************************************************************
 * table ids as defined by standards.
 *
 *****************************************************************************/

enum table_id {
    TABLE_PAT                               = 0x00, // program_association_section
    TABLE_CAT                               = 0x01, // conditional_access_section
    TABLE_PMT                               = 0x02, // program_map_section
    TABLE_TSDT                              = 0x03, // transport_stream_description_section
    TABLE_NIT_ACT                           = 0x40, // network_information_section - actual_network
    TABLE_NIT_OTH                           = 0x41, // network_information_section - other_network
    TABLE_SDT_ACT                           = 0x42, // service_description_section - actual_transport_stream
    TABLE_SDT_OTH                           = 0x46, // service_description_section - other_transport_stream
    TABLE_BAT                               = 0x4A, // bouquet_association_section
    TABLE_EIT_ACT                           = 0x4E, // event_information_section - actual_transport_stream, present/following
    TABLE_EIT_OTH                           = 0x4F, // event_information_section - other_transport_stream, present/following
    TABLE_EIT_SCHEDULE_ACT_50               = 0x50, // 0x50 to 0x5F event_information_section - actual_transport_stream, schedule
    TABLE_EIT_SCHEDULE_ACT_5F               = 0x5F, //
    TABLE_EIT_SCHEDULE_OTH_60               = 0x60, // 0x60 to 0x6F event_information_section - other_transport_stream, schedule
    TABLE_EIT_SCHEDULE_OTH_6F               = 0x6F, //
    TABLE_TDT                               = 0x70, // time_date_section
    TABLE_RST                               = 0x71, // running_status_section
    TABLE_STUFFING                          = 0x72, // stuffing_section
    TABLE_TOT                               = 0x73, // time_offset_section
    TABLE_AIT                               = 0x74, // application information section (TS 102 812 [17])
    TABLE_CST                               = 0x75, // container section (TS 102 323 [15])
    TABLE_RCT                               = 0x76, // related content section (TS 102 323 [15])
    TABLE_CIT                               = 0x77, // content identifier section (TS 102 323 [15])
    TABLE_MPE_FEC                           = 0x78, //
    TABLE_RNS                               = 0x79, // resolution notification section (TS 102 323 [15])
    TABLE_DIT                               = 0x7E, // discontinuity_information_section
    TABLE_SIT                               = 0x7F, // selection_information_section
    TABLE_PREMIERE_CIT                      = 0xA0, // premiere content information section
    TABLE_VCT_TERR                          = 0xC8, // ATSC VCT VSB (terr)
    TABLE_VCT_CABLE                         = 0xC9, // ATSC VCT QAM (cable)
};


/******************************************************************************
 * PIDs as defined for accessing tables.
 *
 *****************************************************************************/
enum pid_type {
    PID_PAT                                 = 0x0000,
    PID_CAT                                 = 0x0001,
    PID_TSDT                                = 0x0002,
    PID_NIT_ST                              = 0x0010,
    PID_SDT_BAT_ST                          = 0x0011,
    PID_EIT_ST_CIT                          = 0x0012,
    PID_RST_ST                              = 0x0013,
    PID_TDT_TOT_ST                          = 0x0014,
    PID_RNT                                 = 0x0016,
    PID_DIT                                 = 0x001E,
    PID_SIT                                 = 0x001F,
    PID_VCT                                 = 0x1FFB,
};

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
