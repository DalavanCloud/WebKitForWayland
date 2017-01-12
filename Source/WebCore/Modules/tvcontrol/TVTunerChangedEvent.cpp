#include "config.h"
#include "TVTunerChangedEvent.h"

#if ENABLE(TV_CONTROL)

namespace WebCore {

Ref<TVTunerChangedEvent> TVTunerChangedEvent::create (const AtomicString& type, String tunerId, TVTunerChangedEvent::Operation operation)
{
    return adoptRef(*new TVTunerChangedEvent(type, tunerId, operation));
}

TVTunerChangedEvent::TVTunerChangedEvent (const AtomicString& type, String tunerId, TVTunerChangedEvent::Operation operation)
    : Event(type, false, false)
    , m_tunerId(tunerId)
    , m_operation(operation)
{

}

TVTunerChangedEvent::~TVTunerChangedEvent ()
{

}

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
