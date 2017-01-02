#include "config.h"
#include "TVCurrentSourceChangedEvent.h"

#if ENABLE(TV_CONTROL)

namespace WebCore {

Ref<TVCurrentSourceChangedEvent> TVCurrentSourceChangedEvent::create (const AtomicString& type, RefPtr<TVSource> source) {
   return adoptRef(*new TVCurrentSourceChangedEvent (type, source));
}

TVCurrentSourceChangedEvent::TVCurrentSourceChangedEvent (const AtomicString& type, RefPtr<TVSource> source)
    : Event(type, false, false)
    , m_source(source) {
}

TVCurrentSourceChangedEvent::~TVCurrentSourceChangedEvent () {

}

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
