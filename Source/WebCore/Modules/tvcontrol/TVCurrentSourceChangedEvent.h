#ifndef TVCurrentSourceChangedEvent_h
#define TVCurrentSourceChangedEvent_h

#if ENABLE(TV_CONTROL)

#include "Event.h"
#include "JSTVSource.h"

namespace WebCore {

class TVSource;

class TVCurrentSourceChangedEvent : public Event {
public:

    static Ref<TVCurrentSourceChangedEvent> create (const AtomicString&, RefPtr<TVSource>);
    ~TVCurrentSourceChangedEvent ();

    RefPtr<TVSource>              source () const { return m_source; }
    virtual EventInterface eventInterface() const { return TVCurrentSourceChangedEventInterfaceType; }
private:
    TVCurrentSourceChangedEvent (const AtomicString&, RefPtr<TVSource>);

    RefPtr<TVSource>   m_source;
};

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // TVCurrentSourceChangedEvent_h
