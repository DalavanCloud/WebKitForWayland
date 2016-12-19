#include "config.h"
#include "TVSource.h"

#if ENABLE(TV_CONTROL)

namespace WebCore {

Ref<TVSource> TVSource::create(RefPtr<PlatformTVSource> platformTVSource, TVTuner* parentTVTuner) {
    return adoptRef(*new TVSource(platformTVSource, parentTVTuner));
}

TVSource::TVSource (RefPtr<PlatformTVSource> platformTVSource, TVTuner* parentTVTuner)
    : m_platformTVSource(platformTVSource)
    , m_parentTVTuner(parentTVTuner)
    , m_scanState(SCANNING_NOT_INITIALISED) {

}

void TVSource::getChannels(TVChannelPromise&& promise) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);

    if (SCANNING_STARTED == m_scanState) {
        promise.reject(nullptr);//TODO replace with state values
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
        promise.reject(nullptr);//TODO replace with state values
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

void TVSource::startScanning (TVPromise&& promise) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (SCANNING_STARTED == m_scanState) { //scanning is in progress
        printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        promise.reject(nullptr);//TODO replace with state values
        return;
    }
    if (m_platformTVSource) {
        printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        m_scanState = SCANNING_STARTED;
        m_platformTVSource->startScanning();
        m_scanState = SCANNING_COMPLETED;
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

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
