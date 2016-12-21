#ifndef CHANNEL_BACKEND_H_
#define CHANNEL_BACKEND_H_

#include <string>
#include <wpe/tvcontrol-backend.h>

using namespace std;
#define TV_DEBUG 1 //TODO remove

namespace BCMRPi {

class ChannelBackend {

public:
    ChannelBackend() {}
    virtual ~ChannelBackend() {}

    void setNetworkId(string networkId){m_networkId = networkId;}
    void setTransportStreamId(string transportStreamId){m_transportStreamId = transportStreamId;}
    void setServiceId(string serviceId){m_serviceId = serviceId;}
    void setName(string name){m_name = name;}
    void setNumber(uint64_t number){m_number = number;}
    void setProgramNumber(int programNumber){m_programNumber = programNumber;}
    void setFrequency(int frequency){m_frequency = frequency;}

    uint64_t getLCN() const {return m_number;}
    int getFrequency() const {return m_frequency;}
    int getProgramNumber() const {return m_programNumber;}
    string getName() const {return m_name;}
    string getServiceId() const {return m_serviceId;}
    string getTransportStreamId() const {return m_transportStreamId;}
    string getNetworkId() const {return m_networkId;}

private:
    string   m_name;
    string   m_networkId;
    string   m_serviceId;
    string   m_transportStreamId;
    uint64_t m_number;
    int      m_programNumber;
    int      m_frequency;

    bool     m_isEmergency;
    bool     m_isFree;
    bool     m_isParentalLocked;
};


} // namespace BCMRPi
#endif //CHANNEL_BACKEND_H_

