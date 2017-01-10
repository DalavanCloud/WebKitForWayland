#include <cstdio>
#include <string>
#include <sstream>
#include <thread>

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
    void dispatchScanStateEvent(scanning_state scanState, wpe_tvcontrol_channel *pChan = nullptr);
    void scanThread(int n);

    private:
    struct wpe_tvcontrol_backend* m_backend;
    wpe_tvcontrol_string* m_strPtr;
    SourceType* m_srcTypeListPtr;
    wpe_tvcontrol_channel* m_channels;
    BcmTVManager tvm;
    ChannelMap chanMap;
    std::string currentTunerId;
    SourceType  currentSource;
    bool bScanInprogress;
};

TvControlBackend::TvControlBackend(struct wpe_tvcontrol_backend* backend)
    : m_backend(backend)
    , m_strPtr(nullptr)
    , m_srcTypeListPtr(nullptr)
    , m_channels(nullptr)
    , tvm(true)
    , bScanInprogress (false)
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

void TvControlBackend::scanThread(int n)
{
    int rc = 0;
    int freq[2] = {156, 362};           // XXX: Pass freq list from JS or read from config file.
    
    myprintf("%s: starting\n", __FUNCTION__);
    for (int i = 0; i < 2; ++i) {
        rc = tvm.scan(freq[i]);
        if (rc == 0) {
            wpe_tvcontrol_channel *pChan = nullptr;    // XXX: fill-in chan info
            dispatchScanStateEvent(Scanned, pChan);
        }
        myprintf("%s: completed for freq %d rc=%d\n", __FUNCTION__, freq[i], rc);
    }

    dispatchScanStateEvent(Completed);
    bScanInprogress = false;
}

void TvControlBackend::startScanning(const char* tuner_id, SourceType srcType)
{
    if (!bScanInprogress) {
        bScanInprogress = true;
        std::thread t(&TvControlBackend::scanThread, this, 0);
        t.detach();
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
    int rc = tvm.getChannelMap(chanMap);
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

            m_channels[index].networkId = nullptr; 
            m_channels[index].name = nullptr; 

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

void TvControlBackend::dispatchScanStateEvent(scanning_state scanState, wpe_tvcontrol_channel *pChan)
{
    wpe_tvcontrol_event event;
    event.eventID = ScanningChanged;
    event.state = scanState;
    event.tuner_id.data = (char *)currentTunerId.c_str();
    event.tuner_id.length = currentTunerId.size();
    event.channel_info = pChan;
    wpe_tvcontrol_backend_dispatch_scanning_state_event(m_backend, &event);
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
    // set current source
    [](void* data, const char* tuner_id, SourceType type) -> tvcontrol_return
    {
        tvcontrol_return rc = TVControlSuccess;
        auto& backend = *static_cast<BCMNexus::TvControlBackend*>(data);
        backend.setCurrentSource(tuner_id, type);
        return rc;
    },
    // get_channel_list
    [](void* data, const char* tuner_id, SourceType type, struct wpe_tvcontrol_channel_vector* out_channel_list) -> tvcontrol_return
    {
        tvcontrol_return rc = TVControlSuccess;
        auto& backend = *static_cast<BCMNexus::TvControlBackend*>(data);
        backend.getChannels(tuner_id, type, out_channel_list);
        return rc;
    },
};

}
