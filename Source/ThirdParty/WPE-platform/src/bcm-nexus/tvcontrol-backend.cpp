#include <cstdio>
#include <string>
#include <sstream>

#include <string.h>

#include <wpe/tvcontrol-backend.h>

#include "BcmTVManager.h"
#define myprintf(...) printf(__VA_ARGS__)
#define mytrace() myprintf("%s: %s:%d\n", __FUNCTION__, __FILE__, __LINE__)

namespace BCMNexus {

// XXX: cleanup after wpe_tvcontrol_string is replaced with std::string & wpe_tvcontrol_XXX_vector with std::vector
class TvControlBackend {
    public:
    TvControlBackend(struct wpe_tvcontrol_backend* backend);
    ~TvControlBackend();
    void getTuners(struct wpe_tvcontrol_string_vector*);
    void getSupportedSourceTypes(const char* tunerId, struct wpe_tvcontrol_src_types_vector*);
    void getSources(const char* tunerId, struct wpe_tvcontrol_src_types_vector*);
    void getSignalStrength(const char* tunerId, double* signalStrength);
    void startScanning(const char* tunerId, SourceType);
    void stopScanning(const char* tunerId);
    void setCurrentChannel(const char* tunerId, SourceType sType, uint64_t);
    void getChannels(const char* tunerId, SourceType sType, struct wpe_tvcontrol_channel_vector*);
    void setCurrentSource(const char* tunerId, SourceType sType);

    private:
    void dispatchScanStateEvent(scanning_state scanState);

    private:
    struct wpe_tvcontrol_backend* m_backend;
    wpe_tvcontrol_string* m_strPtr;
    SourceType* m_srcTypeListPtr;
    wpe_tvcontrol_channel* m_channels;
    BcmTVManager tvm;
    ChannelMap chanMap;
    std::string currentTunerId;
    SourceType  currentSource;
};

TvControlBackend::TvControlBackend(struct wpe_tvcontrol_backend* backend)
    : m_backend(backend)
    , m_strPtr(nullptr)
    , m_srcTypeListPtr(nullptr)
    , m_channels(nullptr)
{
    mytrace();
    int rc = tvm.init();
}

TvControlBackend::~TvControlBackend()
{
    mytrace();
    tvm.deinit();
}

void TvControlBackend::getTuners(struct wpe_tvcontrol_string_vector* outTunerList)
{
    mytrace();
    int count = 4;                      // XXX: get it thru nexus API
    if (!m_strPtr) {
        m_strPtr = (wpe_tvcontrol_string *) new wpe_tvcontrol_string[count];
        for (int i = 0; i < count; ++i) {
            std::stringstream ss;
            ss << "tuner" << i;
            myprintf("%s: %d. %s\n", __FUNCTION__, i, ss.str().c_str());
            m_strPtr[i].data = strdup(ss.str().c_str());
            m_strPtr[i].length = strlen(m_strPtr[i].data);
        }
    }
    outTunerList->length = count;
    outTunerList->strings = m_strPtr;
    myprintf("%s: tuner count=%d\n", __FUNCTION__, outTunerList->length);
}

void TvControlBackend::getSupportedSourceTypes(const char* tunerId, struct wpe_tvcontrol_src_types_vector* out_source_types_list)
{
    mytrace();
    myprintf("%s: tunerId=%s\n", __FUNCTION__, tunerId);
    if (!m_srcTypeListPtr) {
        int count = 0;
        m_srcTypeListPtr = (SourceType *)new SourceType[1];
        m_srcTypeListPtr[count++] =  DvbC;
    }

    int sz = sizeof(m_srcTypeListPtr)/sizeof(SourceType);
    out_source_types_list->length = sz;
    out_source_types_list->types = m_srcTypeListPtr;
}

void TvControlBackend::getSources(const char* tunerId, struct wpe_tvcontrol_src_types_vector* out_source_types_list)
{
    mytrace();
    getSupportedSourceTypes(tunerId, out_source_types_list);
}

void TvControlBackend::getSignalStrength(const char*, double* signalStrength)
{
    mytrace();
}

void TvControlBackend::startScanning(const char* tuner_id, SourceType)
{
    mytrace();
    int rc = 0;
    rc = tvm.scan(156);                 // XXX: Pass freq list from JS or read from config file.
    rc = tvm.scan(362);
    
    if (rc == 0) {
        dispatchScanStateEvent(Completed);
    }
}

void TvControlBackend::stopScanning(const char* tuner_id)
{
    mytrace();
}

void TvControlBackend::setCurrentChannel(const char* tuner_id, SourceType sType, uint64_t id)
{
    myprintf("%s: tunerId=%s sourceType=%d, id=%d\n", __FUNCTION__, tuner_id, sType, id);
    Channel chan = chanMap.at(id-1);
    tvm.disconnect();
    int rc = tvm.tune(chan.frequency, chan.programNumber);
}

void TvControlBackend::getChannels(const char* tuner_id, SourceType, struct wpe_tvcontrol_channel_vector* chan_list)
{
    mytrace();
    int rc = tvm. getChannelMap(chanMap);
    if ((rc == 0) && (!m_channels)) {
        m_channels = new wpe_tvcontrol_channel[chanMap.size()];
        int index = 0;
        for (auto it = std::begin(chanMap); it!=std::end(chanMap); ++it) {
            Channel &chan = *it;
            std::stringstream ss;
            
            ss << chan.frequency;
            m_channels[index].transportSId = strdup(ss.str().c_str());
            
            ss.clear();
            ss << chan.programNumber;
            m_channels[index].serviceId = strdup(ss.str().c_str());
            m_channels[index].number = index+1;
            switch (chan.type) {
            case Channel::Normal: 
                m_channels[index].type = Tv;
                break;
            case Channel::Radio: 
                m_channels[index].type = Radio;
                break;
            default:
                m_channels[index].type = Data;
                break;
            };

            m_channels[index].networkId = strdup("abc");
            m_channels[index].name = strdup("xyz");

            ++index;
            printf("\t%5d. %6d MHz %8d %6d %d %d\n", index, chan.frequency, chan.programNumber, chan.modulation, chan.symbolRate,
                chan.type);
        }
    }
    chan_list->length = chanMap.size();
    chan_list->channels = m_channels;
    mytrace();
}

void TvControlBackend::setCurrentSource(const char* tunerId, SourceType sType)
{
    myprintf("%s: tunerId=%s sType=%d\n", __FUNCTION__, tunerId, sType);
    currentTunerId = tunerId;
    currentSource = sType;
}

void TvControlBackend::dispatchScanStateEvent(scanning_state scanState)
{
    myprintf("%s: scanState=%d\n", __FUNCTION__, scanState);
#if 0
    struct wpe_tvcontrol_channel_event scanEvent;
    scanEvent.state = scanState;
    scanEvent.tuner_id.data = strdup(currentTunerId.c_str()); // ??? who frees
    scanEvent.tuner_id.length = currentTunerId.size();
    scanEvent.source_id.data = nullptr;  //currentSource;
    scanEvent.source_id.length = 0;
    //myprintf("%s: calling wpe_tvcontrol_backend_dispatch_scanning_state_event\n", __FUNCTION__);
    //wpe_tvcontrol_backend_dispatch_scanning_state_event(m_backend, scanEvent);
#endif
}

} // namespace BCMNexus

extern "C" {

struct wpe_tvcontrol_backend_interface bcm_tvcontrol_backend_interface = {
    // create
    [](struct wpe_tvcontrol_backend* backend) -> void*
    {
        return new BCMNexus::TvControlBackend(backend);
    },
    // destroy
    [](void* data)
    {
        auto* backend = static_cast<BCMNexus::TvControlBackend*>(data);
        delete backend;
    },
    // get_tuner_list
    [](void* data, struct wpe_tvcontrol_string_vector *out_tuner_list) -> tvcontrol_return
    {
        tvcontrol_return rc = TVControlSuccess;
        auto& backend = *static_cast<BCMNexus::TvControlBackend*>(data);
        backend.getTuners(out_tuner_list);
        return rc;
    }, 
    // get_supported_source_types_list
    [](void* data, const char* tuner_id, struct wpe_tvcontrol_src_types_vector *out_source_types_list)
    {
        auto& backend = *static_cast<BCMNexus::TvControlBackend*>(data);
        backend.getSupportedSourceTypes(tuner_id, out_source_types_list);
    },
    // get_source_list
    [](void* data, const char* tuner_id, struct wpe_tvcontrol_src_types_vector *out_source_list) -> tvcontrol_return
    {
        tvcontrol_return rc = TVControlSuccess;
        auto& backend = *static_cast<BCMNexus::TvControlBackend*>(data);
        backend.getSources(tuner_id, out_source_list);
        return rc;
    },
    // get_signal_strength
    [](void* data, const char* tuner_id, double* out_signal_strength)
    {
        auto& backend = *static_cast<BCMNexus::TvControlBackend*>(data);
        backend.getSignalStrength(tuner_id, out_signal_strength);
    },
    // start_scanning
    [](void* data, const char* tuner_id, SourceType type, bool isRescanned) -> tvcontrol_return
    {
        tvcontrol_return rc = TVControlSuccess;
        auto& backend = *static_cast<BCMNexus::TvControlBackend*>(data);
        backend.startScanning(tuner_id, type);
        return rc;
    },
    // stop_scanning
    [](void* data, const char* tuner_id) -> tvcontrol_return
    {
        tvcontrol_return rc = TVControlSuccess;
        auto& backend = *static_cast<BCMNexus::TvControlBackend*>(data);
        backend.stopScanning(tuner_id);
        return rc;
    },
    // set_current_channel
    [](void* data, const char* tuner_id, SourceType type, uint64_t channel_number) -> tvcontrol_return
    {
        tvcontrol_return rc = TVControlSuccess;
        auto& backend = *static_cast<BCMNexus::TvControlBackend*>(data);
        backend.setCurrentChannel(tuner_id, type, channel_number);
        return rc;
    },
#if 0
    // get_channel_list
    [](void* data, const char* tuner_id, SourceType type, struct wpe_tvcontrol_channel_vector* out_channel_list) -> tvcontrol_return
    {
        tvcontrol_return rc = TVControlSuccess;
        auto& backend = *static_cast<BCMNexus::TvControlBackend*>(data);
        backend.getChannels(tuner_id, type, out_channel_list);
    },
#endif
    // set current source
    [](void* data, const char* tuner_id, SourceType type) -> tvcontrol_return
    {
        tvcontrol_return rc = TVControlSuccess;
        auto& backend = *static_cast<BCMNexus::TvControlBackend*>(data);
        backend.setCurrentSource(tuner_id, type);
        return rc;
    },
};

}
