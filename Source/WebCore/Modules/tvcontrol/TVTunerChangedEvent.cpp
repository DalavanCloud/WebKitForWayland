#if ENABLE(TV_CONTROL)

#include "TVTunerChangedEvent.h"

namespace WebCore {

Ref<TVTunerChangedEvent> TVTunerChangedEvent::create () {
    return adoptRef(*new TVTunerChangedEvent);
}

TVTunerChangedEvent::TVTunerChangedEvent () {

}

TVTunerChangedEvent::~TVTunerChangedEvent () {

}

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
