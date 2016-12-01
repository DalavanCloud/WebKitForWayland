#include "config.h"
#include "TVManager.h"

#if ENABLE(TV_CONTROL)

namespace WebCore {

Ref<TVManager> TVManager::create(ScriptExecutionContext& context) {
    return adoptRef(*new TVManager(context));
}

TVManager::TVManager(ScriptExecutionContext& context)
   : ActiveDOMObject(&context)
   , m_platformTVManager(nullptr) {
}

TVManager::~TVManager() {

}

const Vector<RefPtr<TVTuner>>&  TVManager::getTuners() {
    if (m_tunerList.size())
        return m_tunerList;

    if (!m_platformTVManager)
        m_platformTVManager = std::make_unique<PlatformTVManager>(this);

    // If the voiceList is empty, that's the cue to get the voices from the platform again.
    for (auto& tuner : m_platformTVManager->getTuners())
        m_tunerList.append(TVTuner::create(tuner));

    return m_tunerList;
}

ScriptExecutionContext* TVManager::scriptExecutionContext() const
{
    return ActiveDOMObject::scriptExecutionContext();
}

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
