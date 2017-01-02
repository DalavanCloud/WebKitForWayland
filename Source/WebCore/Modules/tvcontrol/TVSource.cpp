#include "config.h"
#include "ExceptionCode.h"
#include "TVSource.h"
#include <inttypes.h>

#if ENABLE(TV_CONTROL)

namespace WebCore {

Ref<TVSource> TVSource::create(ScriptExecutionContext* context, RefPtr<PlatformTVSource> platformTVSource, TVTuner* parentTVTuner) {
    return adoptRef(*new TVSource(context, platformTVSource, parentTVTuner));
}

TVSource::TVSource (ScriptExecutionContext* context, RefPtr<PlatformTVSource> platformTVSource, TVTuner* parentTVTuner)
    : ActiveDOMObject(context)
    , m_platformTVSource(platformTVSource)
    , m_parentTVTuner(parentTVTuner)
    , m_scanState(SCANNING_NOT_INITIALISED)
    , m_isScanning(false) {

}

void TVSource::getChannels(TVChannelPromise&& promise) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);

    if (SCANNING_STARTED == m_scanState) {
        promise.reject(INVALID_STATE_ERR, "Invalid state: scanning in progress");
        return;
    }

    if (m_channelList.size()){
        promise.resolve(m_channelList);
        return;
    }

    if (m_platformTVSource) {
        for (auto& channel : m_platformTVSource->getChannels())
            m_channelList.append(TVChannel::create(channel, this));
        promise.resolve(m_channelList);
        return;
    }
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    promise.reject(nullptr);
}

void TVSource::setCurrentChannel (const String& channelNumber, TVPromise&& promise) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (SCANNING_STARTED == m_scanState) {
        promise.reject(INVALID_STATE_ERR, "Invalid state: scanning in progress");
        return;
    }
    if (m_platformTVSource) {
         printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
         if (m_platformTVSource->setCurrentChannel(channelNumber)) {
             promise.resolve(nullptr);
             return;
         }
    }
    promise.reject(nullptr);
}

void TVSource::startScanning (const StartScanningOptions& scanningOptions, TVPromise&& promise) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (SCANNING_STARTED == m_scanState) { //scanning is in progress
        printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        promise.reject(nullptr);//TODO replace with state values
        return;
    }
    if (m_platformTVSource) {
        printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        m_scanState = SCANNING_STARTED;
        m_isScanning = true;
        m_platformTVSource->startScanning(scanningOptions.isRescanned);
        m_scanState = SCANNING_COMPLETED;
        m_isScanning = false;
        promise.resolve(nullptr);
        printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        return;
    }
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    promise.reject(nullptr);
}

void TVSource::stopScanning (TVPromise&& promise) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (m_platformTVSource) {
        if (SCANNING_COMPLETED != m_scanState) { //scanning is already finished
            printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
            m_platformTVSource->stopScanning();
            m_scanState = SCANNING_COMPLETED;
            promise.resolve(nullptr);
        }
    }
    promise.reject(nullptr);
}

void TVSource::dispatchScanningStateChangedEvent(uint16_t state) {

    if (state != SCANNING_STARTED) {
        dispatchEvent(TVScanningStateChangedEvent::create(eventNames().scanningstatechangedEvent, 
                                                          (TVScanningStateChangedEvent::State)state, NULL));
    }
}

ScriptExecutionContext* TVSource::scriptExecutionContext() const
{
    return ActiveDOMObject::scriptExecutionContext();
}

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
