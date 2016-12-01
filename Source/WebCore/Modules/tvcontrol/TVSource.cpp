#include "config.h"
#include "TVSource.h"

#if ENABLE(TV_CONTROL)

namespace WebCore {

Ref<TVSource> TVSource::create(RefPtr<PlatformTVSource> platformTVSource) {
    return adoptRef(*new TVSource(platformTVSource));
}

TVSource::TVSource (RefPtr<PlatformTVSource> platformTVSource) {

}

const Vector<RefPtr<TVChannel>>& TVSource::getChannels () {
    if (m_channelList.size())
        return m_channelList;

    if (m_platformTVSource) {
        // If the voiceList is empty, that's the cue to get the voices from the platform again.
        for (auto& channel : m_platformTVSource->getChannels())
            m_channelList.append(TVChannel::create(channel));
    }

    return m_channelList;
}

void TVSource::setCurrentChannel (const String& channelNumber) {
    m_platformTVSource->setCurrentChannel(channelNumber);
}

void TVSource::startScanning (/*const Options&*/) {
    m_platformTVSource->startScanning();
}

void TVSource::stopScanning () {
    m_platformTVSource->stopScanning();
}

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
