#include "config.h"
#include "TVCurrentChannelChangedEvent.h"

#if ENABLE(TV_CONTROL)

namespace WebCore {

Ref<TVCurrentChannelChangedEvent> TVCurrentChannelChangedEvent::create (const AtomicString& type, TVChannel* channel) {
    return adoptRef(*new TVCurrentChannelChangedEvent (type, channel));
}

TVCurrentChannelChangedEvent::TVCurrentChannelChangedEvent (const AtomicString& type, TVChannel* channel)
    : Event(type, false, false)
    , m_channel(channel) {

}

TVCurrentChannelChangedEvent::~TVCurrentChannelChangedEvent () {

}

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
