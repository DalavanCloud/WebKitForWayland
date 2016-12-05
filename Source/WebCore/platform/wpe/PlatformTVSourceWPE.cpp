#include "config.h"
#include "PlatformTVSource.h"

#if ENABLE(TV_CONTROL)

namespace WebCore {

RefPtr<PlatformTVSource> PlatformTVSource::create(PlatformTVControlBackend* tvBackend, String tunerId)
{
    return adoptRef(new PlatformTVSource(tvBackend, tunerId));
}

PlatformTVSource::PlatformTVSource(PlatformTVControlBackend* tvBackend, String tunerId)
    : m_tunerId(tunerId)
    , m_tvBackend(tvBackend)
    , m_platformTVSourceClient(nullptr)
    , m_channelListIsInitialized(false)
{
}

PlatformTVSource::~PlatformTVSource()
{
}

void PlatformTVSource::setSourceClient(PlatformTVSourceClient* client)
{
    m_platformTVSourceClient = client;
}

const Vector<RefPtr<PlatformTVChannel>>& PlatformTVSource::getChannels () {
    if (!m_channelListIsInitialized) {
        ASSERT(m_channelList.isEmpty());
        //Do steps to identify tuners;
        m_channelListIsInitialized = true;
    }
    return m_channelList;
}

PlatformTVChannel* PlatformTVSource::setCurrentChannel (const String& channelNumber) {
    //Do steps to setCurrentChannel based on ChannelNumber
    return m_currentChannel;
}

void PlatformTVSource::startScanning () {

}

void PlatformTVSource::stopScanning () {

}

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
