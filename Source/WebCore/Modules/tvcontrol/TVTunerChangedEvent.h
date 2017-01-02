#ifndef TVTunerChangedEvent_h
#define TVTunerChangedEvent_h

#if ENABLE(TV_CONTROL)

#include "Event.h"

namespace WebCore {

class TVTunerChangedEvent : public Event {
public:

    enum class Operation {
        Added,
        Removed
    };

    static Ref<TVTunerChangedEvent> create (const AtomicString&, String, Operation);
    ~TVTunerChangedEvent ();

    Operation         operation () const { return m_operation; }
    const String&     id () const { return m_tunerId; }

    virtual EventInterface eventInterface() const { return TVTunerChangedEventInterfaceType; }

private:
    String     m_tunerId;
    Operation  m_operation;
    TVTunerChangedEvent (const AtomicString&, String, Operation);
};

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // TVTunerChangedEvent_h
