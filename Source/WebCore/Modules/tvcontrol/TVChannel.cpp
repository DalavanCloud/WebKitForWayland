#include "config.h"
#include "TVChannel.h"

#if ENABLE(TV_CONTROL)

namespace WebCore {

Ref<TVChannel> TVChannel::create (RefPtr<PlatformTVChannel> platformTVChannel, TVSource* parentTVSource) {
    return adoptRef(*new TVChannel(platformTVChannel, parentTVSource));
}

TVChannel::TVChannel (RefPtr<PlatformTVChannel> platformTVChannel, TVSource* parentTVSource)
    : m_platformTVChannel(platformTVChannel)
    , m_parentTVSource(parentTVSource) {
}

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
