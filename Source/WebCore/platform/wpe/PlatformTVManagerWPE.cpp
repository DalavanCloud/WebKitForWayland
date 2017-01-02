#include "config.h"
#include "PlatformTVManager.h"
#include <wtf/MainThread.h>

#if ENABLE(TV_CONTROL)

#include "PlatformTVControl.h"
#include <wpe/tvcontrol-backend.h>

namespace WebCore {

PlatformTVManager::PlatformTVManager(PlatformTVManagerClient* client)
    : m_tunerListIsInitialized(false)
    , m_platformTVManagerClient(client)
{
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    m_tvBackend = new PlatformTVControlBackend();
    m_tvBackend->m_backend = wpe_tvcontrol_backend_create();

    static struct wpe_tvcontrol_backend_manager_event_client s_eventClient = {
        // handle_tuner_event
        [](void* data, wpe_tvcontrol_tuner_event event)
        {
            callOnMainThread([data, event] {
               printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
               printf("\nEvent = %d", (int)event.operation);
               auto& tvManager = *reinterpret_cast<PlatformTVManager*>(data);
               String tmpId(event.tuner_id.data, event.tuner_id.length);
               tvManager.updateTunerList(tmpId, event.operation);
               tvManager.m_platformTVManagerClient->didTunerOperationChanged(tmpId, (uint16_t)event.operation);
            });
        },
        // handle current source changed event
        [](void* data, wpe_tvcontrol_source_event event)
        {
            auto& tvManager = *reinterpret_cast<PlatformTVManager*>(data);
            tvManager.m_platformTVManagerClient->didCurrentSourceChanged(String(event.tuner_id.data, event.tuner_id.length),
                                                                         String(event.source_id.data, event.source_id.length));
        },
        // handle current channel changed event
        [](void* data, wpe_tvcontrol_channel_event event)
        {
            auto& tvManager = *reinterpret_cast<PlatformTVManager*>(data);
            tvManager.m_platformTVManagerClient->didCurrentChannelChanged(String(event.tuner_id.data, event.tuner_id.length),
                                                                          String(event.source_id.data, event.source_id.length),
                                                                          String(event.channel_id.data, event.channel_id.length));
        },
        // handle current source changed event
        [](void* data, wpe_tvcontrol_channel_event event)
        {
            auto& tvManager = *reinterpret_cast<PlatformTVManager*>(data);
            tvManager.m_platformTVManagerClient->didScanningStateChanged(String(event.tuner_id.data, event.tuner_id.length),
                                                                         String(event.source_id.data, event.source_id.length),
                                                                         String(event.channel_id.data, event.channel_id.length),
                                                                         (uint16_t)event.state);
        },

    };
    wpe_tvcontrol_backend_set_manager_event_client(m_tvBackend->m_backend, &s_eventClient, this);
}

PlatformTVManager::~PlatformTVManager()
{
    wpe_tvcontrol_backend_destroy(m_tvBackend->m_backend);
    delete m_tvBackend;
}

const Vector<RefPtr<PlatformTVTuner>>& PlatformTVManager::getTuners()
{
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (!m_tunerListIsInitialized) {
        printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        ASSERT(m_tunerList.isEmpty());

        struct wpe_tvcontrol_string_vector tunerList;
        tunerList.length = 0;
        wpe_tvcontrol_backend_get_tuner_list(m_tvBackend->m_backend, &tunerList);

        printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        if (tunerList.length) {
            for(uint64_t i = 0; i < tunerList.length; i++) {
                String tmpId(tunerList.strings[i].data, tunerList.strings[i].length);
                m_tunerList.append(PlatformTVTuner::create(tmpId, m_tvBackend));
            }
            m_tunerListIsInitialized = true;
        }
    }
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return m_tunerList;
}

void PlatformTVManager::updateTunerList(String tunerId, uint16_t event) {
   // Add logic to update TunerList
}

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
