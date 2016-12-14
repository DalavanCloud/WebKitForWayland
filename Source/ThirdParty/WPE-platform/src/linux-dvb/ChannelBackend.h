#ifndef CHANNEL_BACKEND_H_
#define CHANNEL_BACKEND_H_

#include <string>
#include <wpe/tvcontrol-backend.h>
#include <SourceBackend.h>

using namespace std;
#define TV_DEBUG 1 //TODO remove

namespace BCMRPi {

class ChannelBackend {

public:
   ChannelBackend(){}
   virtual ~ChannelBackend() {}

private:
    string m_networkId;
    string m_transportStreamId;
    string m_serviceId;
    string m_name;
    string m_number;

    bool       m_isEmergency;
    bool       m_isFree;
    bool       m_isParentalLocked;

    SourceBackend*   m_source;
//  TVChannelType m_ctype; //TODO check type
//  string    m_casSystemId; //TODO
};


} // namespace BCMRPi
#endif //CHANNEL_BACKEND_H_

