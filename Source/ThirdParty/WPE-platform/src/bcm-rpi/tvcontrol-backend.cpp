#include <wpe/tvcontrol-backend.h>

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <fcntl.h>
#include <stdint.h>
#include <inttypes.h>

#ifdef TVCONTROL_BACKEND_LINUX_DVB
#include "TVConfig.h"
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
    void getSourceList(const char*, struct wpe_tvcontrol_src_types_vector*);
    void getSignalStrength(const char*, double* signalStrength);
    void startScanning(const char*, SourceType);
    void stopScanning(const char*);
    void setCurrentChannel(const char*, SourceType, uint64_t);
    void getChannels(const char*, SourceType, struct wpe_tvcontrol_channel_vector*);
    void setCurrentSource(const char*, SourceType);
private:
    struct wpe_tvcontrol_backend* m_backend;
    void handleTunerChangedEvent(struct wpe_tvcontrol_tuner_event);
    void handleSourceChangedEvent(struct wpe_tvcontrol_source_event);
    void handleChannelChangedEvent(struct wpe_tvcontrol_channel_event);
    void handleScanningStateChangedEvent(struct wpe_tvcontrol_channel_event);

    uint64_t                     m_tunerCount;
    wpe_tvcontrol_string*        m_strPtr;

    //void GetTunerCapabilites();
    void checkRegion();
    void initializeTuners();
    void createTunerId(int, int, std::string&);

#ifdef TVCONTROL_BACKEND_LINUX_DVB
    Country                       m_country;
    std::vector<TvTunerBackend*>  m_tunerList;
    void getTunner( const char* tunerId, TvTunerBackend** tuner);
#endif
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
    struct dvbfe_handle *feHandle, *feHandleTmp;
    feOpenMode = 0;

    m_tunerCount = 0;
    for (i = 0; i < DVB_ADAPTER_SCAN; i++) {
        for (j = 0; j < DVB_ADAPTER_SCAN; j++) {

            feHandleTmp = dvbfe_open(i, j, feOpenMode);
            if (feHandleTmp == NULL)
                continue;

            feHandle = new dvbfe_handle;
            createTunerId(i, j, tunerIdStr);
            feHandle->fd = feHandleTmp->fd;
            feHandle->type = feHandleTmp->type;
            feHandle->name =  feHandleTmp->name;
            feHandle->tunerId.assign(tunerIdStr);
            free(feHandleTmp);

#ifdef TV_DEBUG
            printf("Tuner identified as  %s \n Adapter: %d Frontend: %d \n ",
                    feHandle->name, i, j);
            printf("Tuner id %s \n", tunerIdStr.c_str()) ;
#endif
            TvTunerBackend* tInfo = (TvTunerBackend*) new TvTunerBackend(feHandle, m_tunerCount);

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
#ifdef TVCONTROL_BACKEND_LINUX_DVB
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
#endif
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

void TvControlBackend::getTunerList(struct wpe_tvcontrol_string_vector* outTunerList) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int i = 0;

    if (!m_strPtr && m_tunerCount) {
        /*Create an array of  tuner id struct */
        m_strPtr = (wpe_tvcontrol_string * )new wpe_tvcontrol_string[m_tunerCount];
#ifdef TVCONTROL_BACKEND_LINUX_DVB
        /*Iterate  private tuner list and update the created array  */
        for (auto& element: m_tunerList){
            printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
            m_strPtr[i].data = strdup(element->m_feHandle->tunerId.c_str());
            printf("\n%s:%s:%d \n ID = %s  \n", __FILE__, __func__, __LINE__,element->m_feHandle->tunerId.c_str());
            m_strPtr[i].length = element->m_feHandle->tunerId.length();
            i++;
        }
#endif
    }
#ifdef TV_DEBUG
    printf("Number of  tuners mtc = ");
    printf("%" PRIu64 "\n", m_tunerCount);
#endif
    /* update number of tuners and tuner id */
    outTunerList->length = m_tunerCount;
    outTunerList->strings = m_strPtr;

#ifdef TV_DEBUG
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    printf("Number of tuners = ");
    printf("%" PRIu64 "\n", outTunerList->length);
    for(i = 0; i < outTunerList->length; i++)
        printf("%d th tuner id  = %s \n ", (i+1), outTunerList->strings[i].data);
#endif
}

void TvControlBackend::getSupportedSourceTypesList(const char* tunerId,
                                                   struct wpe_tvcontrol_src_types_vector* outSourceTypesList) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
#ifdef TVCONTROL_BACKEND_LINUX_DVB
    /* Get the tuner instance*/
    TvTunerBackend* curTuner;
    getTunner(tunerId, &curTuner);
    /* Invoke get supported type list of the particular tuner instance and get the data*/
    curTuner->getSupportedSrcTypeList(outSourceTypesList);
#endif
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
}

#ifdef TVCONTROL_BACKEND_LINUX_DVB
void TvControlBackend::getTunner(const char* tunerId, TvTunerBackend** tuner) {

    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    /*Iterate  private tuner list and get the particular tuner info  */
    for (auto& element: m_tunerList){
#ifdef TV_DEBUG
        printf("Id of element %s \n",element->m_feHandle->tunerId.c_str());
        printf("Id of required tuner %s \n",tunerId);
        printf(" \n");
#endif

        if (strncmp(element->m_feHandle->tunerId.c_str(), tunerId, 3) == 0) {
            *tuner = element; //TODO test and verify
#ifdef TV_DEBUG
            printf("NAME  :  %s\n",element->m_feHandle->name);
            printf("NAME at list :  %s\n",(*tuner)->m_feHandle->name);
#endif
        }
    }
}
#endif

void TvControlBackend::getSourceList(const char* tunerId, struct wpe_tvcontrol_src_types_vector* outSourceList) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    /* Get the tuner instance*/
#ifdef TVCONTROL_BACKEND_LINUX_DVB
    TvTunerBackend* curTuner;
    getTunner(tunerId, &curTuner);
    /* Invoke get available source type list of the particular tuner and get the data */
    curTuner->getSupportedSrcTypeList(outSourceList);
#endif
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
}

void TvControlBackend::getSignalStrength(const char* tunerId, double* signalStrength) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
#ifdef TVCONTROL_BACKEND_LINUX_DVB
    TvTunerBackend* tuner;
    //getTuner from the Tuner List
    getTunner(tunerId, &tuner);
    tuner->getSignalStrength(signalStrength);
#endif
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
}

void TvControlBackend::startScanning(const char* tunerId, SourceType type) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
#ifdef TVCONTROL_BACKEND_LINUX_DVB
    TvTunerBackend* tuner;
    //getTuner from the Tuner List
    getTunner(tunerId, &tuner);
    tuner->setSrcType(type);
    tuner->startScanning();
#endif
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
}

void TvControlBackend::stopScanning(const char* tunerId) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
#ifdef TVCONTROL_BACKEND_LINUX_DVB
    TvTunerBackend* tuner;
    //getTuner from the Tuner List
    getTunner(tunerId, &tuner);
    tuner->stopScanning();
#endif
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
}

void TvControlBackend::setCurrentChannel(const char* tunerId, SourceType type, uint64_t channelNumber) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
#ifdef TVCONTROL_BACKEND_LINUX_DVB
    TvTunerBackend* tuner;
    //getTuner from the Tuner List
    getTunner(tunerId, &tuner);
    tuner->setCurrentChannel(type, channelNumber);
#endif
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
}

void TvControlBackend::getChannels(const char* tunerId, SourceType type, struct wpe_tvcontrol_channel_vector* channelVector) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
#ifdef TVCONTROL_BACKEND_LINUX_DVB
    TvTunerBackend* tuner;
    //getTuner from the Tuner List
    getTunner(tunerId, &tuner);
    tuner->getChannels(type, channelVector);
#endif
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
}

void TvControlBackend::setCurrentSource(const char* tunerId, SourceType sType) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
#ifdef TVCONTROL_BACKEND_LINUX_DVB
    TvTunerBackend* tuner;
    //getTuner from the Tuner List
    getTunner(tunerId, &tuner);
    tuner->setCurrentSource(sType);
#endif
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
    // get_source_list
    [](void* data, const char* tuner_id, struct wpe_tvcontrol_src_types_vector* out_source_list)
    {
        printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        auto& backend = *static_cast<BCMRPi::TvControlBackend*>(data);
        backend.getSourceList(tuner_id, out_source_list);
    },
    // get_signal_strength
    [](void* data, const char* tuner_id, double* out_signal_strength)
    {
        printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        auto& backend = *static_cast<BCMRPi::TvControlBackend*>(data);
        backend.getSignalStrength(tuner_id, out_signal_strength);
    },
    // start_scanning
    [](void* data, const char* tuner_id, SourceType type)
    {
        printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        auto& backend = *static_cast<BCMRPi::TvControlBackend*>(data);
        backend.startScanning(tuner_id, type);
    },
    // stop_scanning
    [](void* data, const char* tuner_id)
    {
        printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        auto& backend = *static_cast<BCMRPi::TvControlBackend*>(data);
        backend.stopScanning(tuner_id);
    },
    // set_current_channel
    [](void* data, const char* tuner_id, SourceType type, uint64_t channel_number)
    {
        printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        auto& backend = *static_cast<BCMRPi::TvControlBackend*>(data);
        backend.setCurrentChannel(tuner_id, type, channel_number);
    },
    // get_channel_list
    [](void* data, const char* tuner_id, SourceType type, struct wpe_tvcontrol_channel_vector* out_channel_list)
    {
        printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        auto& backend = *static_cast<BCMRPi::TvControlBackend*>(data);
        backend.getChannels(tuner_id, type, out_channel_list);
    },
    // set current source
    [](void* data, const char* tuner_id, SourceType type)
    {
        printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        auto& backend = *static_cast<BCMRPi::TvControlBackend*>(data);
        backend.setCurrentSource(tuner_id, type);
    },
};

}
