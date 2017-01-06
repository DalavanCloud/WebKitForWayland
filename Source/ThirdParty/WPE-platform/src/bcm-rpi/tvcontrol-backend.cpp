
#ifdef TVCONTROL_BACKEND_LINUX_DVB
#include "TVConfig.h"
#include "TunerBackend.h"
#include <libudev.h>
#endif
#include <stdio.h>
#include <string>
#include <thread>
#include <inttypes.h>
#include <wpe/tvcontrol-backend.h>
#include "event-queue.h"

#define TUNER_ID_LEN 3
#define TV_DEBUG 1
using namespace std;

#define TVControlPushEvent( eventId, tunerId, evtOperation/*optional*/) \
{                                                                \
    struct wpe_tvcontrol_event* event = reinterpret_cast<struct wpe_tvcontrol_event*>(malloc(sizeof(struct wpe_tvcontrol_event))); \
    event->eventID = eventId;                                    \
    event->tuner_id.data = strndup(tunerId, TUNER_ID_LEN);       \
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);        \
    printf("Tuner id platform  =  %s \n", event->tuner_id.data); \
    event->tuner_id.length = strlen(tunerId);                    \
    event->operation  = evtOperation;                            \
    m_eventQueue.pushEvent(event);                               \
}

namespace BCMRPi {

struct TvControlBackend {
public:
    EventQueue<wpe_tvcontrol_event*> m_eventQueue;

    TvControlBackend(struct wpe_tvcontrol_backend* backend);
    virtual          ~TvControlBackend();
    tvcontrol_return getTuners(struct wpe_tvcontrol_string_vector*);
    void             getSupportedSourceTypesList(const char*, struct wpe_tvcontrol_src_types_vector*);
    tvcontrol_return getSourceList(const char*, struct wpe_tvcontrol_src_types_vector*);
    void             getSignalStrength(const char*, double* signalStrength);
    tvcontrol_return startScanning(const char*, SourceType, bool isRescanned);
    tvcontrol_return stopScanning(const char*);
    tvcontrol_return setCurrentChannel(const char*, SourceType, uint64_t);
    tvcontrol_return setCurrentSource(const char*, SourceType);
    void updateTunerList(const char *, tuner_changed_operation);

private:
    struct wpe_tvcontrol_backend* m_backend;
    void eventProcessor();
    void tunerChangedListener();  //Listens and triggers the TunerChangedEvent
    void handleTunerChangedEvent(struct wpe_tvcontrol_event*);
    void handleSourceChangedEvent(struct wpe_tvcontrol_event*);
    void handleChannelChangedEvent(struct wpe_tvcontrol_event*);
    void handleScanningStateChangedEvent(struct wpe_tvcontrol_event*);

    uint64_t                     m_tunerCount;
    wpe_tvcontrol_string*        m_strPtr;
    thread                       m_eventThread;
    thread                       m_tunerThread;
    bool                         m_isRunning;
    bool                         m_isEventProcessing;

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
    m_isEventProcessing = true;
    m_isRunning = true;

    initializeTuners();
    m_eventThread = thread(&TvControlBackend::eventProcessor, this);
    m_tunerThread = thread(&TvControlBackend::tunerChangedListener, this);
}

TvControlBackend::~TvControlBackend () {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int i ;
    /*Clear tuner id list*/
    for (i = 0; i < m_tunerCount; i++){
        free(m_strPtr[i].data);
        m_strPtr[i].data = NULL;
    }
    free(m_strPtr);
    m_strPtr = NULL;
    m_isEventProcessing = false;
    m_isRunning = false;
    m_tunerThread.join();
    m_eventThread.join();

#ifdef TVCONTROL_BACKEND_LINUX_DVB
    /*Clear private tuner  list*/
    m_isRunning = false;
    m_tunerList.clear();
#endif
}

void TvControlBackend::eventProcessor () {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    while (m_isEventProcessing) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        wpe_tvcontrol_event* event = m_eventQueue.getEvents();
        if (!event)
            continue;
        switch (event->eventID) {
        case TunerChanged:
            handleTunerChangedEvent(event);
            break;
        case SourceChanged:
            handleSourceChangedEvent(event);
            break;
        case ChannelChanged:
            handleChannelChangedEvent(event);
            break;
        case ScanningChanged:
            handleScanningStateChangedEvent(event);
            break;
        default:
            printf("Unknown Event\n");
        }
        free(event);
    }
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
}

void TvControlBackend::initializeTuners () {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int i, j;
    int feOpenMode;
    std::string tunerIdStr;

#ifdef TVCONTROL_BACKEND_LINUX_DVB
    struct dvbfe_handle *feHandleTmp;
    TunerData *tunerData;
    feOpenMode = 0;

    m_tunerCount = 0;
    for (i = 0; i < DVB_ADAPTER_SCAN; i++) {
        for (j = 0; j < DVB_ADAPTER_SCAN; j++) {

            feHandleTmp = dvbfe_open(i, j, feOpenMode);
            if (feHandleTmp == NULL)
                continue;

            tunerData = new TunerData;
            createTunerId(i, j, tunerIdStr);
#ifdef TV_DEBUG
            printf("Tuner identified as  %s \n Adapter: %d Frontend: %d \n ",
                    feHandleTmp->name, i, j);
            printf("Tuner id %s \n", tunerIdStr.c_str()) ;
#endif
            tunerData->tunerId.assign(tunerIdStr);
            dvbfe_close(feHandleTmp);
            TvTunerBackend* tInfo = (TvTunerBackend*) new TvTunerBackend(&m_eventQueue, m_tunerCount, tunerData);
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

void TvControlBackend::handleTunerChangedEvent(struct wpe_tvcontrol_event* event)
{
    wpe_tvcontrol_backend_dispatch_tuner_event(m_backend, event);
    if (event->tuner_id.data)
        free(event->tuner_id.data);
}

void TvControlBackend::handleSourceChangedEvent(struct wpe_tvcontrol_event* event)
{
    wpe_tvcontrol_backend_dispatch_source_event(m_backend, event);
    if (event->tuner_id.data)
        free(event->tuner_id.data);
}

void TvControlBackend::handleChannelChangedEvent(struct wpe_tvcontrol_event* event)
{
    wpe_tvcontrol_backend_dispatch_channel_event(m_backend, event);
    if (event->tuner_id.data)
        free(event->tuner_id.data);
}

void TvControlBackend::handleScanningStateChangedEvent(struct wpe_tvcontrol_event* event)
{
    wpe_tvcontrol_backend_dispatch_scanning_state_event(m_backend, event);

    if (event->tuner_id.data)
        free(event->tuner_id.data);
    // free channel info
    if (event->channel_info) {
        if (event->channel_info->networkId)
            free(event->channel_info->networkId);
        if (event->channel_info->transportSId)
            free(event->channel_info->transportSId);
        if (event->channel_info->serviceId)
            free(event->channel_info->serviceId);
        if (event->channel_info->name)
            free(event->channel_info->name);
        if (event->channel_info)
            free(event->channel_info);
    }
}

tvcontrol_return TvControlBackend::getTuners(struct wpe_tvcontrol_string_vector* outTunerList) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int i = 0;
    tvcontrol_return ret = TVControlFailed;
    if (!m_strPtr && m_tunerCount) {
        /*Create an array of  tuner id struct */
        ret = TVControlSuccess;
        m_strPtr = (wpe_tvcontrol_string * )new wpe_tvcontrol_string[m_tunerCount];
#ifdef TVCONTROL_BACKEND_LINUX_DVB
        /*Iterate  private tuner list and update the created array  */
        for (auto& element: m_tunerList){
            printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
            m_strPtr[i].data = strdup(element->m_tunerData->tunerId.c_str());
            printf("\n%s:%s:%d \n ID = %s  \n", __FILE__, __func__, __LINE__,element->m_tunerData->tunerId.c_str());
            m_strPtr[i].length = element->m_tunerData->tunerId.length();
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
    for (i = 0; i < outTunerList->length; i++)
        printf("%d th tuner id  = %s \n ", (i+1), outTunerList->strings[i].data);
#endif
    return ret;
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
        printf("Id of this tuner %s \n", element->m_tunerData->tunerId.c_str());
        printf("Id of required tuner %s \n",tunerId);
        printf(" \n");
#endif
        if (strncmp(element->m_tunerData->tunerId.c_str(), tunerId, 3) == 0) {
            *tuner = element;
        }
    }
}
#endif

tvcontrol_return TvControlBackend::getSourceList(const char* tunerId, struct wpe_tvcontrol_src_types_vector* outSourceList) {
    tvcontrol_return ret = TVControlFailed;
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    /* Get the tuner instance*/
#ifdef TVCONTROL_BACKEND_LINUX_DVB
    TvTunerBackend* curTuner;
    getTunner(tunerId, &curTuner);
    /* Invoke get available source type list of the particular tuner and get the data */
    ret = curTuner->getSupportedSrcTypeList(outSourceList);
#endif
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return ret;
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

tvcontrol_return TvControlBackend::startScanning(const char* tunerId, SourceType type, bool isRescanned) {
    tvcontrol_return ret = TVControlFailed;
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
#ifdef TVCONTROL_BACKEND_LINUX_DVB
    TvTunerBackend* tuner;
    //getTuner from the Tuner List
    getTunner(tunerId, &tuner);
    tuner->setSrcType(type);
    ret = tuner->startScanning(isRescanned);
#endif
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return ret;
}

tvcontrol_return TvControlBackend::stopScanning(const char* tunerId) {
    tvcontrol_return ret = TVControlFailed;
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
#ifdef TVCONTROL_BACKEND_LINUX_DVB
    TvTunerBackend* tuner;
    //getTuner from the Tuner List
    getTunner(tunerId, &tuner);
    ret = tuner->stopScanning();
#endif
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return ret;
}

tvcontrol_return TvControlBackend::setCurrentChannel(const char* tunerId, SourceType type, uint64_t channelNumber) {
    tvcontrol_return ret = TVControlFailed;
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
#ifdef TVCONTROL_BACKEND_LINUX_DVB
    TvTunerBackend* tuner;
    //getTuner from the Tuner List
    getTunner(tunerId, &tuner);
    ret = tuner->setCurrentChannel(type, channelNumber);
#endif
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return ret;
}

tvcontrol_return TvControlBackend::setCurrentSource(const char* tunerId, SourceType sType) {
    tvcontrol_return ret = TVControlFailed;
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
#ifdef TVCONTROL_BACKEND_LINUX_DVB
    TvTunerBackend* tuner;
    //getTuner from the Tuner List
    getTunner(tunerId, &tuner);
    ret = tuner->setCurrentSource(sType);

    TVControlPushEvent(SourceChanged, tunerId, (tuner_changed_operation)0/*Not relevant*/);
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);

#endif
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return ret;
}

/* tunerChangedListener waits for the addition/removal of Tuner and triggers the event*/
void TvControlBackend::tunerChangedListener() {
#ifdef TVCONTROL_BACKEND_LINUX_DVB
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;

    /* Create the udev object */
    udev = udev_new();
    if (!udev) {
        printf("Can't create udev\n");
        exit(1);
    }

    /* Create a list of the devices in the 'dvb' subsystem.(Initially found Tuners) */
    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "dvb");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);
    udev_list_entry_foreach(dev_list_entry, devices) {
        const char *path;

        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);

        printf("Device Node Path: %s\n", udev_device_get_devnode(dev));

        dev = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");
        if (!dev) {
            printf("Unable to find parent usb device.");
            exit(1);
        }
    }
    int fd;
    struct udev_monitor* mon;
    mon = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(mon, "dvb", NULL);
    udev_monitor_enable_receiving(mon);
    fd = udev_monitor_get_fd(mon);
    // Loop and wait for new Tuners. 
    while (m_isRunning) {
        fd_set fds;
        struct timeval tv;
        int ret;
        std::size_t found;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 0;

        ret = select(fd + 1, &fds, NULL, NULL, &tv);

        if (ret > 0 && FD_ISSET(fd, &fds)) {
            printf("\nselect() says there should be data\n");

            dev = udev_monitor_receive_device(mon);
            if (dev) {
                std::string tunerId;
                tuner_changed_operation operation;
                string fpath;
                int i, j;
                printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
                fpath = udev_device_get_devnode(dev);
                printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
                found = fpath.find("frontend");
                if (found != std::string::npos) { // if frontend found, do the following and dipatch the event.
                    sscanf (udev_device_get_devnode(dev), "/%*[a-z-A-Z]/%*[a-z-A-Z]/%*[a-z-A-Z]%d/%*[a-z-A-Z]%d", &i, &j);
                    createTunerId(i, j, tunerId);
                    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);

                    // The data from udev is in string format. So.converting the operation into the appropriate data type.
                    if (strcmp("add", udev_device_get_action(dev)) == 0) { // Case when DVB adapter is added.
                        operation = Added;
                    }
                    else if (strcmp("remove", udev_device_get_action(dev)) == 0) { // Case when DVB Adapter is closed.
                        operation = Removed;
                    }
                    updateTunerList(tunerId.c_str(), operation);
                    TVControlPushEvent(TunerChanged, tunerId.c_str(), operation);
                    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
                }
                printf("***Tuner***\n");
                printf("   Node: %s\n", udev_device_get_devnode(dev));
                printf("   Action: %s\n", udev_device_get_action(dev));
                udev_device_unref(dev);
            } else {
                printf("No Device from receive_device(). An error occured.\n");
            }
        }
        usleep(250*1000);
        printf(".");fflush(stdout);
    }
    udev_unref(udev);
#endif
}

void TvControlBackend::updateTunerList(const char* tunerId, tuner_changed_operation operation) {
#ifdef TVCONTROL_BACKEND_LINUX_DVB
    if (operation == Added) { // Case when DVB adapter is added.
        TunerData *tunerData = new TunerData;
        tunerData->tunerId.assign(tunerId);
        TvTunerBackend* tInfo = (TvTunerBackend*) new TvTunerBackend(&m_eventQueue, m_tunerCount, tunerData);
        m_tunerList.push_back(tInfo);
        printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        printf("Found and Added the Tuner:-)");
        m_tunerCount += 1;
    }
    else if (operation == Removed) { // Case when DVB Adapter is closed.
        int position = 0;
        /*Iterate  private tuner list and get the particular tuner info  */
        for (auto& element : m_tunerList) {
            printf("Id of this tuner %s \n", element->m_tunerData->tunerId.c_str());
            printf("Id of required tuner %s \n", tunerId);
            printf(" \n");

            if (strncmp(element->m_tunerData->tunerId.c_str(), tunerId, element->m_tunerData->tunerId.length()) == 0) {
                delete element;
                m_tunerList.erase(m_tunerList.begin() + position);
                printf("Found and deleted the Tuner:-))");
                break;
            }
            position++;
        }
    }
#endif
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
    [](void* data, struct wpe_tvcontrol_string_vector* out_tuner_list) -> tvcontrol_return
    {
        tvcontrol_return ret = TVControlFailed;
        printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        auto& backend = *static_cast<BCMRPi::TvControlBackend*>(data);
        ret = backend.getTuners(out_tuner_list);
        return ret;
    }, 
    // get_supported_source_types_list
    [](void* data, const char* tuner_id, struct wpe_tvcontrol_src_types_vector* out_source_types_list)
    {
        printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        auto& backend = *static_cast<BCMRPi::TvControlBackend*>(data);
        backend.getSupportedSourceTypesList(tuner_id, out_source_types_list);
    },
    // get_source_list
    [](void* data, const char* tuner_id, struct wpe_tvcontrol_src_types_vector* out_source_list) -> tvcontrol_return
    {
        tvcontrol_return ret = TVControlFailed;
        printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        auto& backend = *static_cast<BCMRPi::TvControlBackend*>(data);
        ret = backend.getSourceList(tuner_id, out_source_list);
        return ret;
    },
    // get_signal_strength
    [](void* data, const char* tuner_id, double* out_signal_strength)
    {
        printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        auto& backend = *static_cast<BCMRPi::TvControlBackend*>(data);
        backend.getSignalStrength(tuner_id, out_signal_strength);
    },
    // start_scanning
    [](void* data, const char* tuner_id, SourceType type, bool isRescanned) -> tvcontrol_return
    {
        tvcontrol_return ret = TVControlFailed;
        printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        auto& backend = *static_cast<BCMRPi::TvControlBackend*>(data);
        ret = backend.startScanning(tuner_id, type, isRescanned);
        return ret;
    },
    // stop_scanning
    [](void* data, const char* tuner_id) -> tvcontrol_return
    {
        tvcontrol_return ret = TVControlFailed;
        printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        auto& backend = *static_cast<BCMRPi::TvControlBackend*>(data);
        ret = backend.stopScanning(tuner_id);
        return ret;
    },
    // set_current_channel
    [](void* data, const char* tuner_id, SourceType type, uint64_t channel_number) -> tvcontrol_return
    {
        tvcontrol_return ret = TVControlFailed;
        printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        auto& backend = *static_cast<BCMRPi::TvControlBackend*>(data);
        ret = backend.setCurrentChannel(tuner_id, type, channel_number);
        return ret;
    },
    // set current source
    [](void* data, const char* tuner_id, SourceType type) -> tvcontrol_return
    {
        tvcontrol_return ret = TVControlFailed;
        printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        auto& backend = *static_cast<BCMRPi::TvControlBackend*>(data);
        ret = backend.setCurrentSource(tuner_id, type);
        return ret;
    },
};

}
