#ifndef TVCurrentChannelChangedEvent_h
#define TVCurrentChannelChangedEvent_h

#if ENABLE(TV_CONTROL)

#include "Event.h"
#include "JSTVChannel.h"
namespace WebCore {

class TVChannel;

class TVCurrentChannelChangedEvent : public Event {
public:

    static Ref<TVCurrentChannelChangedEvent> create (const AtomicString&, RefPtr<TVChannel>);
    ~TVCurrentChannelChangedEvent ();

    RefPtr<TVChannel>                               channel () const { return m_channel; }
    virtual EventInterface eventInterface() const { return TVCurrentChannelChangedEventInterfaceType; }
private:
    TVCurrentChannelChangedEvent (const AtomicString&, RefPtr<TVChannel>);

    RefPtr<TVChannel>   m_channel;
};

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // TVCurrentChannelChangedEvent_h
