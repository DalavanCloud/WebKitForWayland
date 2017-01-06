#ifndef TVScanningStateChangedEvent_h
#define TVScanningStateChangedEvent_h

#if ENABLE(TV_CONTROL)

#include "Event.h"
#include "JSTVChannel.h"

namespace WebCore {

class TVChannel;

class TVScanningStateChangedEvent : public Event {
public:

    enum class State  {
        Cleared,
        Scanned,
        Completed,
        Stopped
    };

    static Ref<TVScanningStateChangedEvent> create (const AtomicString&, State, RefPtr<TVChannel>);
    ~TVScanningStateChangedEvent ();

    State        state () const { return m_state; }
    RefPtr<TVChannel>   channel () const { return m_channel; }
    virtual EventInterface eventInterface() const { return TVScanningStateChangedEventInterfaceType; }

private:
    TVScanningStateChangedEvent (const AtomicString&, State, RefPtr<TVChannel>);

    State m_state;
    RefPtr<TVChannel> m_channel;
};

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // TVScanningStateChangedEvent_h
