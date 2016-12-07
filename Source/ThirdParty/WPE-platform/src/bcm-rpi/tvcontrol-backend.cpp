#include <wpe/tvcontrol-backend.h>

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>


#include <fcntl.h>
#include <stdint.h>

#include "TunerBackend.h"
using namespace std;

namespace BCMRPi {

struct TvControlBackend {
public:
    TvControlBackend(struct wpe_tvcontrol_backend* backend);
    virtual ~TvControlBackend() {}
    void getTunerList(struct wpe_tvcontrol_string_vector*);
    void getSupportedSourceTypesList(const char*, struct wpe_tvcontrol_src_types_vector*);

private:
    struct wpe_tvcontrol_backend* m_backend;
    void handleTunerChangedEvent(struct wpe_tvcontrol_tuner_event);
    void handleSourceChangedEvent(struct wpe_tvcontrol_source_event);
    void handleChannelChangedEvent(struct wpe_tvcontrol_channel_event);
    void handleScanningStateChangedEvent(struct wpe_tvcontrol_channel_event);

    void checkRegion();

    Country m_country;
    std::vector<TvTunerBackend*> m_tunerList;
    //void GetTunerCapabilites();
    void initializeTuners();
};

TvControlBackend::TvControlBackend (struct wpe_tvcontrol_backend* backend)
    : m_backend(backend) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    // Initialize Tuner list
    initializeTuners();
    // Identify Region
    checkRegion();
    // Read Tuner Capabilities
    // Configure Tuners
}

void TvControlBackend::initializeTuners () {

}

void TvControlBackend::checkRegion ()
{
    string country, data;
    printf("Country");
    fstream fObj;
    fObj.open("TVConfig.txt", ios::in);
    fObj >> data;

    while (!fObj.eof()) {
        if (!data.find("REGION")) {
            fObj.seekp(3, ios::cur);
               cout << m_country << "\n";
            }
            else{
               cout << "Country Not Found...Setting Default to GB";
               m_country = US;
            }
            break;
    }
}

void TvControlBackend::handleTunerChangedEvent(struct wpe_tvcontrol_tuner_event event)
{
    wpe_tvcontrol_backend_dispatch_tuner_event(m_backend, event);
}

void TvControlBackend::handleSourceChangedEvent(struct wpe_tvcontrol_source_event event)
{
    wpe_tvcontrol_backend_dispatch_source_event(m_backend, event);
}

void TvControlBackend::handleChannelChangedEvent(struct wpe_tvcontrol_channel_event event)
{
    wpe_tvcontrol_backend_dispatch_channel_event(m_backend, event);
}

void TvControlBackend::handleScanningStateChangedEvent(struct wpe_tvcontrol_channel_event event)
{
    wpe_tvcontrol_backend_dispatch_scanning_state_event(m_backend, event);
}

void TvControlBackend::getTunerList(struct wpe_tvcontrol_string_vector* out_tuner_list) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
}

void TvControlBackend::getSupportedSourceTypesList(const char* tuner_id, struct wpe_tvcontrol_src_types_vector* out_source_types_list) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
}

} // namespace BCMRPi
extern "C" {

struct wpe_tvcontrol_backend_interface bcm_rpi_tvcontrol_backend_interface = {
    // create
    [](struct wpe_tvcontrol_backend* backend) -> void*
    {
        printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        return new BCMRPi::TvControlBackend(backend);
    },
    // destroy
    [](void* data)
    {
        printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        auto* backend = static_cast<BCMRPi::TvControlBackend*>(data);
        delete backend;
    },
    // get_tuner_list
    [](void* data, struct wpe_tvcontrol_string_vector* out_tuner_list)
    {
        printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        auto& backend = *static_cast<BCMRPi::TvControlBackend*>(data);
        backend.getTunerList(out_tuner_list);
    }, 
    // get_supported_source_types_list
    [](void* data, const char* tuner_id, struct wpe_tvcontrol_src_types_vector* out_source_types_list)
    {
        printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        auto& backend = *static_cast<BCMRPi::TvControlBackend*>(data);
        backend.getSupportedSourceTypesList(tuner_id, out_source_types_list);
    },
};

}
