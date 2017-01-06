#include "config.h"
#include "TVScanningStateChangedEvent.h"

#if ENABLE(TV_CONTROL)

namespace WebCore {

Ref<TVScanningStateChangedEvent> TVScanningStateChangedEvent::create (const AtomicString& type, TVScanningStateChangedEvent::State state, RefPtr<TVChannel> channel) {
    return adoptRef(*new TVScanningStateChangedEvent (type, state, channel));
}

TVScanningStateChangedEvent::TVScanningStateChangedEvent (const AtomicString& type, TVScanningStateChangedEvent::State state, RefPtr<TVChannel> channel)
    : Event(type, false, false)
    , m_state(state)
    , m_channel(channel) {
}

TVScanningStateChangedEvent::~TVScanningStateChangedEvent () {

}

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
