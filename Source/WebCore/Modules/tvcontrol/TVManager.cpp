#include "config.h"
#include "TVManager.h"

#if ENABLE(TV_CONTROL)

#include "Document.h"
#include "Frame.h"
#include "Navigator.h"

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

const Vector<RefPtr<TVTuner>>&  TVManager::getTuners() {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (m_tunerList.size())
        return m_tunerList;

    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (!m_platformTVManager)
        m_platformTVManager = std::make_unique<PlatformTVManager>(this);

    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    // If the voiceList is empty, that's the cue to get the voices from the platform again.
    for (auto& tuner : m_platformTVManager->getTuners()) {
        printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        m_tunerList.append(TVTuner::create(tuner));
    }

    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return m_tunerList;
}

ScriptExecutionContext* TVManager::scriptExecutionContext() const
{
    return ActiveDOMObject::scriptExecutionContext();
}

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
