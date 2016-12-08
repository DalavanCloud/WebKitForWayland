#include "config.h"
#include "TVSource.h"

#if ENABLE(TV_CONTROL)

namespace WebCore {

Ref<TVSource> TVSource::create(RefPtr<PlatformTVSource> platformTVSource, TVTuner* parentTVTuner) {
    return adoptRef(*new TVSource(platformTVSource, parentTVTuner));
}

TVSource::TVSource (RefPtr<PlatformTVSource> platformTVSource, TVTuner* parentTVTuner)
    : m_platformTVSource(platformTVSource)
    , m_parentTVTuner(parentTVTuner) {

}

void TVSource::getChannels(TVChannelPromise&& promise) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
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
    m_platformTVSource->setCurrentChannel(channelNumber);
    promise.resolve(nullptr);
}

void TVSource::startScanning (TVPromise&& promise) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    m_platformTVSource->startScanning();
    promise.resolve(nullptr);
}

void TVSource::stopScanning (TVPromise&& promise) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    m_platformTVSource->stopScanning();
    promise.resolve(nullptr);
}

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
