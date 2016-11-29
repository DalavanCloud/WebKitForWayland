#include "config.h"
#include "PlatformTVManager.h"

#if ENABLE(TV_CONTROL)

namespace WebCore {

PlatformTVManager::PlatformTVManager(PlatformTVManagerClient* client)
    : m_tunerListIsInitialized(false)
    , m_platformTVManagerClient(client)
{
}

PlatformTVManager::~PlatformTVManager()
{
}

const Vector<RefPtr<PlatformTVTuner>>& PlatformTVManager::getTuners()
{
    if (!m_tunerListIsInitialized) {
        ASSERT(m_tunerList.isEmpty());
        //Do steps to identify tuners;
        m_tunerListIsInitialized = true;
    }
    return m_tunerList;
}

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
