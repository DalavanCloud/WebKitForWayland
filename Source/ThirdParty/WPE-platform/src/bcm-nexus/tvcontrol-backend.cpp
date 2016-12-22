#include <wpe/tvcontrol-backend.h>

#include <cstdio>

#define myprintf(...) printf(__VA_ARGS__)
#define mytrace() myprintf("%s: %s:%d\n", __FUNCTION__, __FILE__, __LINE__)

namespace BCMNexus {

struct TvControlBackend {
    TvControlBackend(struct wpe_tvcontrol_backend* backend);
    ~TvControlBackend();
    void getTunerList(struct wpe_tvcontrol_string_vector*);
    void getSupportedSourceTypesList(const char*, struct wpe_tvcontrol_src_types_vector*);
    void getSourceList(const char*, struct wpe_tvcontrol_src_types_vector*);
    void getSignalStrength(const char*, double* signalStrength);
    void startScanning(const char*, SourceType);
    void stopScanning(const char*);
    void setCurrentChannel(const char*, SourceType, uint64_t);
    void getChannelList(const char*, SourceType, struct wpe_tvcontrol_channel_vector*);
};

TvControlBackend::TvControlBackend(struct wpe_tvcontrol_backend* backend)
{
}

TvControlBackend::~TvControlBackend()
{
}

void TvControlBackend::getTunerList(struct wpe_tvcontrol_string_vector*)
{
    mytrace();
}

void TvControlBackend::getSupportedSourceTypesList(const char*, struct wpe_tvcontrol_src_types_vector*)
{
    mytrace();
}

void TvControlBackend::getSourceList(const char*, struct wpe_tvcontrol_src_types_vector*)
{
    mytrace();
}

void TvControlBackend::getSignalStrength(const char*, double* signalStrength)
{
    mytrace();
}

void TvControlBackend::startScanning(const char*, SourceType)
{
    mytrace();
}

void TvControlBackend::stopScanning(const char*)
{
    mytrace();
}

void TvControlBackend::setCurrentChannel(const char*, SourceType, uint64_t)
{
    mytrace();
}

void TvControlBackend::getChannelList(const char*, SourceType, struct wpe_tvcontrol_channel_vector*)
{
    mytrace();
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
    [](void* data, struct wpe_tvcontrol_string_vector* out_tuner_list)
    {
        auto& backend = *static_cast<BCMNexus::TvControlBackend*>(data);
        backend.getTunerList(out_tuner_list);
    }, 
    // get_supported_source_types_list
    [](void* data, const char* tuner_id, struct wpe_tvcontrol_src_types_vector* out_source_types_list)
    {
        auto& backend = *static_cast<BCMNexus::TvControlBackend*>(data);
        backend.getSupportedSourceTypesList(tuner_id, out_source_types_list);
    },
    // get_source_list
    [](void* data, const char* tuner_id, struct wpe_tvcontrol_src_types_vector* out_source_list)
    {
        auto& backend = *static_cast<BCMNexus::TvControlBackend*>(data);
        backend.getSourceList(tuner_id, out_source_list);
    },
    // get_signal_strength
    [](void* data, const char* tuner_id, double* out_signal_strength)
    {
        auto& backend = *static_cast<BCMNexus::TvControlBackend*>(data);
        backend.getSignalStrength(tuner_id, out_signal_strength);
    },
    // start_scanning
    [](void* data, const char* tuner_id, SourceType type)
    {
        auto& backend = *static_cast<BCMNexus::TvControlBackend*>(data);
        backend.startScanning(tuner_id, type);
    },
    // stop_scanning
    [](void* data, const char* tuner_id)
    {
        auto& backend = *static_cast<BCMNexus::TvControlBackend*>(data);
        backend.stopScanning(tuner_id);
    },
    // set_current_channel
    [](void* data, const char* tuner_id, SourceType type, uint64_t channel_number)
    {
        auto& backend = *static_cast<BCMNexus::TvControlBackend*>(data);
        backend.setCurrentChannel(tuner_id, type, channel_number);
    },
    // get_channel_list
    [](void* data, const char* tuner_id, SourceType type, struct wpe_tvcontrol_channel_vector* out_channel_list)
    {
        auto& backend = *static_cast<BCMNexus::TvControlBackend*>(data);
        backend.getChannelList(tuner_id, type, out_channel_list);
    },
};

}
