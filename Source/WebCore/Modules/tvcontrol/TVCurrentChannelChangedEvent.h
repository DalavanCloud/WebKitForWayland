#ifndef TVCurrentChannelChangedEvent_h
#define TVCurrentChannelChangedEvent_h

#if ENABLE(TV_CONTROL)

#include "Event.h"

namespace WebCore {

class TVCurrentChannelChangedEvent : public Event {
public:

    static Ref<TVCurrentChannelChangedEvent> create ();
    ~TVCurrentChannelChangedEvent ();

    TVChannel*                               channel () const { return nullptr; }

private:
    TVCurrentChannelChangedEvent ();
};

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // TVCurrentChannelChangedEvent_h
