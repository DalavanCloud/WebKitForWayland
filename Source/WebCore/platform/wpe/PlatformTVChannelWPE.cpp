#include "config.h"
#include "PlatformTVChannel.h"

#if ENABLE(TV_CONTROL)

namespace WebCore {

RefPtr<PlatformTVChannel> PlatformTVChannel::create(PlatformTVControlBackend* tvBackend, String tunerId)
{
    return adoptRef(new PlatformTVChannel(tvBackend, tunerId));
}

PlatformTVChannel::PlatformTVChannel(PlatformTVControlBackend* tvBackend, String tunerId)
    : m_tunerId(tunerId)
    , m_PlatformTVChannelClient(nullptr)
    , m_tvBackend(tvBackend)
{
}

PlatformTVChannel::~PlatformTVChannel()
{
}

void PlatformTVChannel::setChannelClient(PlatformTVChannelClient* client)
{
    m_PlatformTVChannelClient = client;
}

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
