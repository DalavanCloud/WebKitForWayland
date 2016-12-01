#ifndef TVScanningStateChangedEvent_h
#define TVScanningStateChangedEvent_h

#if ENABLE(TV_CONTROL)

#include "Event.h"

namespace WebCore {

class TVScanningStateChangedEvent : public Event {
public:

    static Ref<TVScanningStateChangedEvent> create ();
    ~TVScanningStateChangedEvent ();

    enum TVScanningState  {
        CLEARED,
        SCANNED,
        COMPLETED,
        STOPPED
    };

    TVScanningState   state () const { return CLEARED; }
    TVChannel*        channel () const { return nullptr; }

private:
    TVScanningStateChangedEvent ();
};

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // TVScanningStateChangedEvent_h
