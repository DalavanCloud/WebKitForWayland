#ifndef TUNER_BACKEND_H_
#define TUNER_BACKEND_H_

#include <fstream>
#include <math.h>
#include <sstream>
#include "TVConfig.h"
#include "SourceBackend.h"

using namespace std;

#define DVB_ADAPTER_SCAN 6
#define DVB_FE_SCAN 6
#define DVB_MAX_TUNER 6
#define FE_STATUS_PARAMS (DVBFE_INFO_LOCKSTATUS|DVBFE_INFO_FEPARAMS|DVBFE_INFO_SIGNAL_STRENGTH|DVBFE_INFO_BER|DVBFE_INFO_SNR|DVBFE_INFO_UNCORRECTED_BLOCKS)
#define CONFIGFILE "TVConfig.txt"
typedef std::map<std::string, std::string> ConfigInfo;

namespace BCMRPi {

class TvTunerBackend {
public:
   TvTunerBackend(struct dvbfe_handle*, int);
   virtual ~TvTunerBackend();

   SourceType getSrcType() { return m_sType; };
   void setSrcType(SourceType sType){ m_sType = sType; };
   void getTunerInfo();
   void getSignalStrength(double*);
   void startScanning();
   void stopScanning();
   void getChannels(SourceType, struct wpe_tvcontrol_channel_vector*);
   void setCurrentChannel(SourceType, uint64_t);
   void setCurrentSource(SourceType sType);

   struct dvbfe_handle*    m_feHandle;

   std::vector<SourceBackend*>  m_sourceList; //List of source objects

   void getSupportedSrcTypeList(wpe_tvcontrol_src_types_vector* out_source_types_list);

private:

   ChannelList              m_channel;

   struct dvb_frontend_info m_feInfo;
   struct dvbfe_info fe_info;//TODO check and remove

   SourceType                      m_sType;
   SourceType*                     m_srcTypeListPtr;
   uint64_t                        m_supportedSysCount;
   wpe_tvcontrol_src_types_vector  m_srcList; //List of src type
   ConfigInfo                      m_configValues;

   int baseOffset(int channel, int channelList);
   int freqStep(int channel, int channelList);
   int getSupportedSourcesTypeList(wpe_tvcontrol_src_types_vector*);

   void getAvailableSrcList(wpe_tvcontrol_src_types_vector*);
   void initializeSourceList();
   void getSources();
   void getSource(SourceType, SourceBackend**);
   void getSourceType(SourceType sType, fe_delivery_system*);
   void configureTuner(std::string& modulation);
   void setModulation(std::string& modulation);
   void configureTuner(int tunerCnt);
   void getConfiguration();
   void getCapabilities();
   void populateFreq();
};

} // namespace BCMRPi
#endif //TUNER_BACKEND_H_
