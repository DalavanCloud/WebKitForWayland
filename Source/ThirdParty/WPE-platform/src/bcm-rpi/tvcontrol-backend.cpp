#include <wpe/tvcontrol-backend.h>

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <fcntl.h>
#include <stdint.h>
#include <inttypes.h>

#include "TVConfig.h"

#ifdef TVCONTROL_BACKEND_LINUX_DVB
#include "TunerBackend.h"
#endif

#define TV_DEBUG 1
using namespace std;

namespace BCMRPi {

struct TvControlBackend {
public:
    TvControlBackend(struct wpe_tvcontrol_backend* backend);
    virtual ~TvControlBackend();
    void getTunerList(struct wpe_tvcontrol_string_vector*);
    void getSupportedSourceTypesList(const char*, struct wpe_tvcontrol_src_types_vector*);

private:
    struct wpe_tvcontrol_backend* m_backend;
    void handleTunerChangedEvent(struct wpe_tvcontrol_tuner_event);
    void handleSourceChangedEvent(struct wpe_tvcontrol_source_event);
    void handleChannelChangedEvent(struct wpe_tvcontrol_channel_event);
    void handleScanningStateChangedEvent(struct wpe_tvcontrol_channel_event);

    uint64_t                     m_tunerCount;
    Country                      m_country;
#ifdef TVCONTROL_BACKEND_LINUX_DVB
    std::vector<TvTunerBackend>  m_tunerList;
#endif
    wpe_tvcontrol_string*        m_strPtr;

    //void GetTunerCapabilites();
    void checkRegion();
    void initializeTuners();
    void createTunerId(int, int, std::string&);
};

TvControlBackend::TvControlBackend (struct wpe_tvcontrol_backend* backend)
    : m_backend(backend)
    , m_strPtr(nullptr)
    , m_tunerCount(0) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    // Initialize Tuner list
    initializeTuners();
    // Identify Region
    checkRegion();
    // Read Tuner Capabilities
    // Configure Tuners
}

TvControlBackend::~TvControlBackend () {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int i ;
    /*Clear tuner id list*/
    for(i = 0; i < m_tunerCount; i++){
        free(m_strPtr[i].data);
        m_strPtr[i].data = NULL;
    }
    free(m_strPtr);
    m_strPtr = NULL;

#ifdef TVCONTROL_BACKEND_LINUX_DVB
    /*Clear private tuner  list*/
    m_tunerList.clear();
#endif
}

void TvControlBackend::initializeTuners () {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int i, j;
    int feOpenMode;
    std::string tunerIdStr;

#ifdef TVCONTROL_BACKEND_LINUX_DVB
    struct dvbfe_handle* feHandle[DVB_MAX_TUNER];
    feOpenMode = O_RDWR | O_NONBLOCK;

    m_tunerCount = 0;
    for (i = 0; i < DVB_ADAPTER_SCAN; i++) {
        for (j = 0; j < DVB_ADAPTER_SCAN; j++) {
            feHandle[m_tunerCount] = dvbfe_open(i, j, feOpenMode);
            if (feHandle[m_tunerCount] == NULL)
                continue;

            createTunerId(i, j, tunerIdStr);

#ifdef TV_DEBUG
            printf("Tuner identified as  %s \n Adapter: %d Frontend: %d \n ",
                    feHandle[m_tunerCount]->name, i, j);
            printf("Tuner id %s \n", tunerIdStr.c_str()) ;
#endif
            TvTunerBackend tInfo(feHandle[m_tunerCount], m_tunerCount);
            tInfo.m_tunerId.assign(tunerIdStr);

            /*Update the  private tuner list*/
            m_tunerList.push_back(tInfo);
            m_tunerCount += 1;
            printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        }
    }
#endif
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
}

void TvControlBackend::createTunerId(int i, int j, std::string& tunerId) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    tunerId.assign(std::to_string(i));
    tunerId.append(":");           //delimiter
    tunerId.append(std::to_string(j));
    printf("Tuner id %s \n", tunerId.c_str()) ;
}

void TvControlBackend::checkRegion () {
    string country, data;
    printf("Country");
    fstream fObj;
    fObj.open(TV_CONFIG_FILE, ios::in);

    while (!fObj.eof()) {
        fObj >> data;
        if (!data.find("REGION")) {
            fObj.seekp(3, ios::cur);
            fObj >> country;
            if (!country.find("US")) {
               m_country = US;
               cout << m_country << "\n";
            }
            else{
               cout << "Country Not Found...Setting Default to GB";
            }
            break;
        }
    }
    fObj.close();
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
    int i = 0;

    if (!m_strPtr && m_tunerCount) {
        /*Create an array of  tuner id struct */
        m_strPtr = (wpe_tvcontrol_string * )new wpe_tvcontrol_string[m_tunerCount];
#ifdef TVCONTROL_BACKEND_LINUX_DVB
        /*Iterate  private tuner list and update the created array  */
        for (auto& element: m_tunerList){
            m_strPtr[i].data = strdup(element.m_tunerId.c_str());
            m_strPtr[i].length = element.m_tunerId.length();
            i++;
        }
#endif
    }

    /* update number of tuners and tuner id */
    out_tuner_list->length = m_tunerCount;
    out_tuner_list->strings = m_strPtr;

#ifdef TV_DEBUG
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    printf("Number of  tuners = ");
    printf("%" PRIu64 "\n", out_tuner_list->length);
    for(i = 0; i < out_tuner_list->length; i++)
        printf("%ld th tuner id  = %s \n ", (i+1), out_tuner_list->strings[i].data);
#endif
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
