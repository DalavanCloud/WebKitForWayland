#ifndef TUNER_BACKEND_H_
#define TUNER_BACKEND_H_

#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/dvb/frontend.h>
#include <wpe/tvcontrol-backend.h>
#include "TVConfig.h"
#include "SourceBackend.h"

using namespace std;

#define DVB_ADAPTER_SCAN 6
#define DVB_FE_SCAN 6
#define DVB_MAX_TUNER 6
#define FE_STATUS_PARAMS (DVBFE_INFO_LOCKSTATUS|DVBFE_INFO_FEPARAMS|DVBFE_INFO_SIGNAL_STRENGTH|DVBFE_INFO_BER|DVBFE_INFO_SNR|DVBFE_INFO_UNCORRECTED_BLOCKS)

namespace BCMRPi {

class TvTunerBackend {
public:
   TvTunerBackend(struct dvbfe_handle*, int);
   virtual ~TvTunerBackend();
   void getTunerInfo();
   void getCapabilities();
   void setModulation(int);
   void populateFreq(ChannelList);
   void getSignalStrength(double*);
   void startScanning(SourceType);

   struct dvbfe_handle*    m_feHandle;

   std::vector<SourceBackend*>  m_sourceList; //List of source objects

   void getSupportedSrcTypeList(wpe_tvcontrol_src_types_vector* out_source_types_list);

private:

   ChannelList              m_channel;

   struct dvb_frontend_info m_feInfo;
   struct dvbfe_info fe_info;//TODO check and remove

   SourceType*                     m_srcTypeListPtr;
   uint64_t                        m_supportedSysCount;
   wpe_tvcontrol_src_types_vector m_srcList; //List of src type

   int baseOffset(int channel, int channelList);
   int freqStep(int channel, int channelList);
   int getSupportedSourcesTypeList(wpe_tvcontrol_src_types_vector*);

   void getAvailableSrcList(wpe_tvcontrol_src_types_vector*);
   void initializeSourceList();
   void getSources();
   void getSource(SourceType, SourceBackend**);
};

} // namespace BCMRPi
#endif //TUNER_BACKEND_H_
