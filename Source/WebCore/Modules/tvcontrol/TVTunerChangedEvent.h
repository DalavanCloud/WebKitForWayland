#ifndef TVTunerChangedEvent_h
#define TVTunerChangedEvent_h

#if ENABLE(TV_CONTROL)

#include "Event.h"

namespace WebCore {

class TVTunerChangedEvent : public Event {
public:

    static Ref<TVTunerChangedEvent> create ();
    ~TVTunerChangedEvent ();

    enum TVTunerChangedEventOperation { 
        ADDED, 
        REMOVED
    };

    TVTunerChangedEventOperation  operation () const { return REMOVED; }
    const String&                 id () const { return "emptyStr"; }

private:
    TVTunerChangedEvent ();
};

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // TVTunerChangedEvent_h