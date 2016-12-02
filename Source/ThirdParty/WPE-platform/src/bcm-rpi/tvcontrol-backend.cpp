#include <wpe/tvcontrol-backend.h>

#include <cstdio>

namespace BCMRPi {

struct TvControlBackend {
    TvControlBackend() {}
    virtual ~TvControlBackend() {}
    void getTunerList(struct wpe_tvcontrol_string_vector*);
    void getSupportedSourceTypesList(char*, struct wpe_tvcontrol_string_vector*);
};

void TvControlBackend::getTunerList(struct wpe_tvcontrol_string_vector* out_tuner_list) {
   
}

void TvControlBackend::getSupportedSourceTypesList(char* tuner_id, struct wpe_tvcontrol_string_vector* out_source_types_list) {

}

} // namespace BCMRPi
extern "C" {

struct wpe_tvcontrol_backend_interface bcm_rpi_tvcontrol_backend_interface = {
    // create
    []() -> void*
    {
        return new BCMRPi::TvControlBackend();
    },
    // destroy
    [](void* data)
    {
        auto* backend = static_cast<BCMRPi::TvControlBackend*>(data);
        delete backend;
    },
    // get_tuner_list
    [](void* data, struct wpe_tvcontrol_string_vector* out_tuner_list)
    {
        auto& backend = *static_cast<BCMRPi::TvControlBackend*>(data);
        backend.getTunerList(out_tuner_list);
    }, 
    // get_supported_source_types_list
    [](void* data, char* tuner_id, struct wpe_tvcontrol_string_vector *out_source_types_list)
    {
        auto& backend = *static_cast<BCMRPi::TvControlBackend*>(data);
        backend.getSupportedSourceTypesList(tuner_id, out_source_types_list);
    },
};

}
