#ifndef TVCurrentSourceChangedEvent_h
#define TVCurrentSourceChangedEvent_h

#if ENABLE(TV_CONTROL)

#include "Event.h"

namespace WebCore {

class TVSource;

class TVCurrentSourceChangedEvent : public Event {
public:

    static Ref<TVCurrentSourceChangedEvent> create (const AtomicString&, TVSource*);
    ~TVCurrentSourceChangedEvent ();

    TVSource*                               source () const { return m_source; }

private:
    TVCurrentSourceChangedEvent (const AtomicString&, TVSource*);

    TVSource*   m_source;
};

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // TVCurrentSourceChangedEvent_h
