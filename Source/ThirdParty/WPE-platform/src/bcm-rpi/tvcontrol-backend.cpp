
#ifdef TVCONTROL_BACKEND_LINUX_DVB
#include "TVConfig.h"
#include "TunerBackend.h"
#endif
#include <stdio.h>
#include <string>
#include <thread>
#include <inttypes.h>
#include <libudev.h>
#include <wpe/event-queue.h>
#include <wpe/tvcontrol-backend.h>

#define TV_DEBUG 1
using namespace std;

namespace BCMRPi {

struct TvControlBackend {
public:
    EventQueue<wpe_tvcontrol_event* > eventQueue;
    TvControlBackend(struct wpe_tvcontrol_backend* backend);
    virtual ~TvControlBackend();
    void getTuners(struct wpe_tvcontrol_string_vector*);
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
    void eventProcessor();
    void TunerChangedListener();  //Listens and triggers the TunerChangedEvent
    void handleTunerChangedEvent(struct wpe_tvcontrol_event*);
    void handleSourceChangedEvent(struct wpe_tvcontrol_event*);
    void handleChannelChangedEvent(struct wpe_tvcontrol_event*);
    void handleScanningStateChangedEvent(struct wpe_tvcontrol_event*);

    uint64_t                     m_tunerCount;
    wpe_tvcontrol_string*        m_strPtr;
    bool                         m_isRunning;
    thread                       m_eventThread;
    thread                       m_tunerThread;
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
    m_eventThread = thread(&TvControlBackend::eventProcessor, this);
    m_tunerThread = thread(&TvControlBackend::TunerChangedListener, this);
    initializeTuners();
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
    m_isEventProcessing = false;
    m_isRunning = false;
    m_tunerThread.join();
    m_eventThread.join();

#ifdef TVCONTROL_BACKEND_LINUX_DVB
    /*Clear private tuner  list*/
    m_isRunning = false;
    // TODO : call pthread join to wait for thread exit
    m_tunerList.clear();
#endif
}

void TvControlBackend::eventProcessor () {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    while (m_isEventProcessing) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        wpe_tvcontrol_event* event = eventQueue.getEvents();
        if(!event)
            continue;
        switch (event->eventID) {
            case TUNER_CHANGED:
                handleTunerChangedEvent(event);
                break;
            case SOURCE_CHANGED:
                handleSourceChangedEvent(event);
                break;
            case CHANNEL_CHANGED:
                handleChannelChangedEvent(event);
                break;
            case SCANNING_CHANGED:
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
            TvTunerBackend* tInfo = (TvTunerBackend*) new TvTunerBackend(m_tunerCount, tunerData);
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
}

void TvControlBackend::handleSourceChangedEvent(struct wpe_tvcontrol_event* event)
{
    wpe_tvcontrol_backend_dispatch_source_event(m_backend, event);
}

void TvControlBackend::handleChannelChangedEvent(struct wpe_tvcontrol_event* event)
{
    wpe_tvcontrol_backend_dispatch_channel_event(m_backend, event);
}

void TvControlBackend::handleScanningStateChangedEvent(struct wpe_tvcontrol_event* event)
{
    wpe_tvcontrol_backend_dispatch_scanning_state_event(m_backend, event);
}

void TvControlBackend::getTuners(struct wpe_tvcontrol_string_vector* outTunerList) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int i = 0;

    if (!m_strPtr && m_tunerCount) {
        /*Create an array of  tuner id struct */
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

/* TunerChangedListener waits for the addition/removal of Tuner and triggers the event*/
void TvControlBackend::TunerChangedListener() {
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;

    sleep(10);  // Sleep for 10 seconds so that the device goes through the initial setup process.

    /* Create the udev object */
    udev = udev_new();
    if (!udev) {
        printf("Can't create udev\n");
        exit(1);
    }

#ifdef TVCONTROL_BACKEND_LINUX_DVB
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

        printf("  VID/PID: %s %s\n", udev_device_get_sysattr_value(dev,"idVendor"), udev_device_get_sysattr_value(dev, "idProduct"));
        printf("  %s\n  %s\n", udev_device_get_sysattr_value(dev,"idVendor"), udev_device_get_sysattr_value(dev, "idProduct"));
        printf("  %s\n  %s\n", udev_device_get_sysattr_value(dev,"manufacturer"), udev_device_get_sysattr_value(dev,"product"));
        printf("  serial: %s\n", udev_device_get_sysattr_value(dev, "serial"));
        udev_device_unref(dev);
    }
    /* Free the enumerator object */
    udev_enumerate_unref(enumerate);

    int fd;
    struct udev_monitor* mon;
    mon = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(mon, "dvb", NULL);
    udev_monitor_enable_receiving(mon);
    fd = udev_monitor_get_fd(mon);
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);

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

        ret = select(fd+1, &fds, NULL, NULL, &tv);

        if (ret > 0 && FD_ISSET(fd, &fds)) {
            printf("\nselect() says there should be data\n");

            dev = udev_monitor_receive_device(mon);
            if (dev) {
               std::string tunerId;
               string fpath;
               int i, j;
               printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
               struct wpe_tvcontrol_event* event = reinterpret_cast<struct wpe_tvcontrol_event*>(malloc(sizeof(struct wpe_tvcontrol_event)));
               event->eventID = TUNER_CHANGED;
               fpath = udev_device_get_devnode(dev);
               // The data from udev is in string format. So.converting the operation into the appropriate data type.
               if (strcmp("add", udev_device_get_action(dev)) == 0) { // Case when DVB adapter is added.
                  event->operation = Added;
               }
               else if (strcmp("remove", udev_device_get_action(dev)) == 0) { // Case when DVB Adapter is closed.
                  event->operation = Removed;
               }

               printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
               found = fpath.find("frontend");
               if (found != std::string::npos) { // if frontend found, do the following and dipatch the event.
                  sscanf (udev_device_get_devnode(dev), "/%*[a-z-A-Z]/%*[a-z-A-Z]/%*[a-z-A-Z]%d/%*[a-z-A-Z]%d", &i, &j);
                  createTunerId(i, i, tunerId);
                  event->tuner_id.data = strdup(tunerId.c_str());
                  event->tuner_id.length = tunerId.length();
                  printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
                  eventQueue.pushEvent(event);// Sensed a Tuner Change Event....Triggering handleTunerChangedEvent.
                  printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
               }
               printf("***Got Device***\n");
               printf("   Node: %s\n", udev_device_get_devnode(dev));
               printf("   Subsystem: %s\n", udev_device_get_subsystem(dev));
               printf("   Devtype: %s\n", udev_device_get_devtype(dev));
               printf("   Action: %s\n", udev_device_get_action(dev));
               udev_device_unref(dev);
            }
            else {
               printf("No Device from receive_device(). An error occured.\n");
            }
        }
        usleep(250*1000);
        printf(".");
        fflush(stdout);
    }
#endif
    udev_unref(udev);
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
        backend.getTuners(out_tuner_list);
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
