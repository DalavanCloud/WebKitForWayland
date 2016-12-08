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
        //return m_tunerList;
        promise.resolve(m_tunerList);
        return;
    }
    if (!m_platformTVManager)
        m_platformTVManager = std::make_unique<PlatformTVManager>(this);

    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    for (auto& tuner : m_platformTVManager->getTuners()) {
        printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        m_tunerList.append(TVTuner::create(tuner));
    }

    if (m_tunerList.size())
    {
        promise.resolve(m_tunerList);
        return;
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
