#include "config.h"
#include "PlatformTVSource.h"

#if ENABLE(TV_CONTROL)

#include "PlatformTVControl.h"
#include <wpe/tvcontrol-backend.h>

namespace WebCore {

RefPtr<PlatformTVSource> PlatformTVSource::create(PlatformTVControlBackend* tvBackend, String tunerId, Type type)
{
    return adoptRef(new PlatformTVSource(tvBackend, tunerId, type));
}

PlatformTVSource::PlatformTVSource(PlatformTVControlBackend* tvBackend, String tunerId, Type type)
    : m_tunerId(tunerId)
    , m_type(type)
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

        /* Get available channel list*/
        struct wpe_tvcontrol_channel_vector channelList;
        channelList.length = 0;
        wpe_tvcontrol_backend_get_channel_list(m_tvBackend->m_backend, m_tunerId.utf8().data(), (SourceType)m_type, &channelList);

        if (channelList.length) {
            for (uint64_t i = 0; i < channelList.length; i++) {
                m_tvBackend->m_channel = &channelList.channels[i];
                m_channelList.append(PlatformTVChannel::create(m_tvBackend, m_tunerId.utf8().data()));
            }
            m_channelListIsInitialized = true;
        }
    }
    return m_channelList;
}

RefPtr<PlatformTVChannel> PlatformTVSource::setCurrentChannel (const String& channelNumber) {
    m_currentChannel = nullptr;
    //Do steps to setCurrentChannel based on ChannelNumber
    wpe_tvcontrol_backend_set_current_channel(m_tvBackend->m_backend, m_tunerId.utf8().data(), (SourceType)m_type, atoi(channelNumber.utf8().data()));
    /* Parse the channel list and set current channel */
    for(auto& channel : m_channelList){
        if(channel->number() == channelNumber ){
            m_currentChannel = channel;
        }
    }
    return m_currentChannel;
}

void PlatformTVSource::startScanning () {
    wpe_tvcontrol_backend_start_scanning(m_tvBackend->m_backend, m_tunerId.utf8().data(), (SourceType)m_type);
}

void PlatformTVSource::stopScanning () {
    wpe_tvcontrol_backend_stop_scanning(m_tvBackend->m_backend, m_tunerId.utf8().data());
}

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
