#include "config.h"
#include "TVChannel.h"

#if ENABLE(TV_CONTROL)

namespace WebCore {

Ref<TVChannel> TVChannel::create (RefPtr<PlatformTVChannel> platformTVChannel) {
    return adoptRef(*new TVChannel(platformTVChannel));
}

TVChannel::TVChannel (RefPtr<PlatformTVChannel> platformTVChannel) {

}

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
