#include "config.h"
#include "PlatformTVChannel.h"

#if ENABLE(TV_CONTROL)

namespace WebCore {

RefPtr<PlatformTVChannel> PlatformTVChannel::create()
{
    return adoptRef(new PlatformTVChannel());
}

PlatformTVChannel::PlatformTVChannel()
    : m_PlatformTVChannelClient(nullptr)
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
