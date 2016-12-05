#include "config.h"
#include "PlatformTVManager.h"

#if ENABLE(TV_CONTROL)

#include "PlatformTVControl.h"
#include <wpe/tvcontrol-backend.h>

namespace WebCore {

PlatformTVManager::PlatformTVManager(PlatformTVManagerClient* client)
    : m_tunerListIsInitialized(false)
    , m_platformTVManagerClient(client)
{
    m_tvBackend = new PlatformTVControlBackend();
    m_tvBackend->m_backend = wpe_tvcontrol_backend_create();
}

PlatformTVManager::~PlatformTVManager()
{
    wpe_tvcontrol_backend_destroy(m_tvBackend->m_backend);
    delete m_tvBackend;
}

const Vector<RefPtr<PlatformTVTuner>>& PlatformTVManager::getTuners()
{
    if (!m_tunerListIsInitialized) {
        ASSERT(m_tunerList.isEmpty());

        struct wpe_tvcontrol_string_vector tunerList;
        wpe_tvcontrol_backend_get_tuner_list(m_tvBackend->m_backend, &tunerList);

        if (tunerList.length) {
            for(uint64_t i = 0; i < tunerList.length; i++) {
                String tmpId(tunerList.strings[i].data, tunerList.strings[i].length);
                m_tunerList.append(PlatformTVTuner::create(tmpId, m_tvBackend));
            }
            m_tunerListIsInitialized = true;
        }
    }
    return m_tunerList;
}

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
