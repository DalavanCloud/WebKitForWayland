#if ENABLE(TV_CONTROL)

#include "TVScanningStateChangedEvent.h"

namespace WebCore {

Ref<TVScanningStateChangedEvent> TVScanningStateChangedEvent::create () {
    return adoptRef(*new TVScanningStateChangedEvent);
}

TVScanningStateChangedEvent::TVScanningStateChangedEvent () {

}

TVScanningStateChangedEvent::~TVScanningStateChangedEvent () {

}

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
