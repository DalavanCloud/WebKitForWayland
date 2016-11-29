#include "config.h"
#include "TVManager.h"

#if ENABLE(TV_CONTROL)

namespace WebCore {

Ref<TVManager> TVManager::create() {
    return adoptRef(*new TVManager);
}

TVManager::TVManager() 
   : m_platformTVManager(nullptr){

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

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
