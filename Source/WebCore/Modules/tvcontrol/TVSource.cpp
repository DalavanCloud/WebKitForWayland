#include "config.h"
#include "ExceptionCode.h"
#include "TVSource.h"

#if ENABLE(TV_CONTROL)

namespace WebCore {

Ref<TVSource> TVSource::create(ScriptExecutionContext* context, RefPtr<PlatformTVSource> platformTVSource, TVTuner* parentTVTuner) {
    return adoptRef(*new TVSource(context, platformTVSource, parentTVTuner));
}

TVSource::TVSource (ScriptExecutionContext* context, RefPtr<PlatformTVSource> platformTVSource, TVTuner* parentTVTuner)
    : m_platformTVSource(platformTVSource)
    , m_parentTVTuner(parentTVTuner)
    , m_scanState(SCANNING_NOT_INITIALISED)
    , ContextDestructionObserver(context)
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
        if (m_platformTVSource->startScanning(scanningOptions.isRescanned)) {
            m_scanState = SCANNING_COMPLETED;
            m_isScanning = false;
            promise.resolve(nullptr);
            printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
            return;
        }
        m_scanState = SCANNING_COMPLETED;
    }
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    promise.reject(nullptr);
}

void TVSource::stopScanning (TVPromise&& promise) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (m_platformTVSource) {
        if (SCANNING_COMPLETED != m_scanState) { //scanning is already finished
            printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
            if (m_platformTVSource->stopScanning()) {
                m_scanState = SCANNING_COMPLETED;
                promise.resolve(nullptr);
                return;
            }
        }
    }
    promise.reject(nullptr);
}

void TVSource::dispatchScanningStateChangedEvent(RefPtr<PlatformTVChannel> platformTVChannel, uint16_t state) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    RefPtr<TVChannel> channel = nullptr;
    if (m_platformTVSource && platformTVChannel) {
            channel = TVChannel::create(platformTVChannel, this);
            m_channelList.append(channel);
    }
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    scriptExecutionContext()->postTask([=](ScriptExecutionContext&) {
        dispatchEvent(TVScanningStateChangedEvent::create(eventNames().scanningstatechangedEvent,
                                                         (TVScanningStateChangedEvent::State)state, channel));
    });
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
}

ScriptExecutionContext* TVSource::scriptExecutionContext() const
{
    return ContextDestructionObserver::scriptExecutionContext();
}

void TVSource::contextDestroyed()
{
    ContextDestructionObserver::contextDestroyed();
}

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
