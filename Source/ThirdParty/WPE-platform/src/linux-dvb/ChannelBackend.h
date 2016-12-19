#ifndef CHANNEL_BACKEND_H_
#define CHANNEL_BACKEND_H_

#include <string>
#include <wpe/tvcontrol-backend.h>

using namespace std;
#define TV_DEBUG 1 //TODO remove

namespace BCMRPi {

class ChannelBackend {

public:
    ChannelBackend(){}
    virtual ~ChannelBackend() {}
    void setNetworkId(string networkId){m_networkId = networkId;}
    void setTransportStreamId(string transportStreamId){m_transportStreamId = transportStreamId;}
    void setServiceId(string serviceId){m_serviceId = serviceId;}
    void setName(string name){m_name = name;}
    void setNumber(string number){m_number = number;}
    void setProgramNumber(int programNumber){m_programNumber = programNumber;}
    void setFrequency(int frequency){m_frequency = frequency;}

private:
    string m_networkId;
    string m_transportStreamId;
    string m_serviceId;
    string m_name;
    string m_number;
    int    m_programNumber;
    int    m_frequency;

    bool       m_isEmergency;
    bool       m_isFree;
    bool       m_isParentalLocked;
};


} // namespace BCMRPi
#endif //CHANNEL_BACKEND_H_

