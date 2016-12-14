#ifndef SOURCE_BACKEND_H_
#define SOURCE_BACKEND_H_

#include <string>
#include <libdvbapi/dvbfe.h>
#include <wpe/tvcontrol-backend.h>

using namespace std;
#define TV_DEBUG 1 //TODO remove

struct dvbfe_handle {
    int             fd;
    enum dvbfe_type type;
    char           *name;
    std::string     tunerId;
};

namespace BCMRPi {

class SourceBackend {

public:
   SourceBackend(SourceType, dvbfe_handle*);
   virtual ~SourceBackend() {}

   void startScanning();
   void stopScanning();
   void setCurrentChannel(uint64_t channelNo);
   void getChannels();
   SourceType srcType() { return m_sType; }

private:
    SourceType    m_sType;
    dvbfe_handle* m_feHandle;
};


} // namespace BCMRPi
#endif //SOURCE_BACKEND_H_
