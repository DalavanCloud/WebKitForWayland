#if ENABLE(TV_CONTROL)

#include "TVCurrentChannelChangedEvent.h"

namespace WebCore {

Ref<TVCurrentChannelChangedEvent> TVCurrentChannelChangedEvent::create () {
    return adoptRef(*new TVCurrentChannelChangedEvent);
}

TVCurrentChannelChangedEvent::TVCurrentChannelChangedEvent () {

}

TVCurrentChannelChangedEvent::~TVCurrentChannelChangedEvent () {

}

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)