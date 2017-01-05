#include "config.h"
#include "TVManager.h"

#if ENABLE(TV_CONTROL)

#include "TVTunerChangedEvent.h"

#include "Document.h"
#include "Frame.h"
#include "Navigator.h"
#include "EventNames.h"

namespace WebCore {

Ref<TVManager> TVManager::create(ScriptExecutionContext* context) {
    return adoptRef(*new TVManager(context));
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
}

TVManager::TVManager(ScriptExecutionContext* context)
   : ActiveDOMObject(context)
   , m_platformTVManager(nullptr) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
}

TVManager::~TVManager() {

}
void TVManager::didTunerOperationChanged (String tunerId, uint16_t event) {
    int position;
    if (event) { // Case when DVB adapter is added.
        m_tunerList.append(TVTuner::create(PlatformTVTuner::create(tunerId.utf8().data(), m_platformTVManager->m_tvBackend)));
        printf("Found and Added the Tuner");
    } else { // Case when DVB Adapter is closed.
        position = 0;
        //Iterate  private tuner list and get the particular tuner info
        for (auto& element : m_tunerList) {
            printf("Id of this tuner %s \n",(element->id()).utf8().data());
            if (strncmp((element->id()).utf8().data(), tunerId.utf8().data(), 3) == 0) {
                m_tunerList.remove(position);
                printf("Found and Deleted the Tuner");
                break;
            }
            position++;
        }
    }
    dispatchEvent(TVTunerChangedEvent::create(eventNames().tunerchangedEvent, tunerId, (TVTunerChangedEvent::Operation)event));
}

void TVManager::didCurrentSourceChanged(String tunerId, String sourceId) {
    //Implement logic to identify corresponding tuner instance and source instance
    //Create event using idenified instance details
}

void TVManager::didCurrentChannelChanged(String tunerId, String sourceId, String channelId) {
    //Implement logic to identify corresponding tuner instance, source instance and channel instance
    //Create event using idenified instance details
}

void TVManager::didScanningStateChanged(String tunerId, String sourceId, String channelId, uint16_t state) {
    //Implement logic to identify corresponding tuner instance, source instance and channel instance
    //Create event using idenified instance details
}

void TVManager::getTuners(TVTunerPromise&& promise) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (m_tunerList.size())
    {
        promise.resolve(m_tunerList);
        return;
    }
    if (!m_platformTVManager)
        m_platformTVManager = std::make_unique<PlatformTVManager>(this);

    if (m_platformTVManager) {
        Vector<RefPtr<PlatformTVTuner>> platformTunerList;
        if (!m_platformTVManager->getTuners(platformTunerList)) {
            promise.reject(nullptr);
            return;
        }
        if (platformTunerList.size()) {
            printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
            for (auto& tuner : platformTunerList) {
                printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
                m_tunerList.append(TVTuner::create(tuner));
            }
            platformTunerList.clear();
        }
        if (m_tunerList.size())
        {
            promise.resolve(m_tunerList);
            return;
        }
    }
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    promise.reject(nullptr);
}

ScriptExecutionContext* TVManager::scriptExecutionContext() const
{
    return ActiveDOMObject::scriptExecutionContext();
}

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
