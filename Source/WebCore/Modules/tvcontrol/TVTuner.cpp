#include "config.h"
#include "TVTuner.h"

#if ENABLE(TV_CONTROL)

namespace WebCore {

Ref<TVTuner> TVTuner::create (RefPtr<PlatformTVTuner> platformTVTuner) {
    return adoptRef(*new TVTuner (platformTVTuner));
}

TVTuner::TVTuner (RefPtr<PlatformTVTuner> platformTVTuner)
    : m_platformTVTuner(nullptr) {
}

const Vector<TVTuner::SourceType>&  TVTuner::getSupportedSourceTypes () {
    //Use platform logic to get supported Source Types and map.
    if (m_sourceTypeList.size())
        return m_sourceTypeList;

    if (m_platformTVTuner) {
        // If the voiceList is empty, that's the cue to get the voices from the platform again.
        for (auto& type : m_platformTVTuner->getSupportedSourceTypes())
            m_sourceTypeList.append((SourceType)(type)); // Add logic to convert this into TVSourceType
    }

    return m_sourceTypeList;
}

const Vector<RefPtr<TVSource>>&  TVTuner::getSources () {
    if (m_sourceList.size())
        return m_sourceList;

    if (m_platformTVTuner) {
        // If the voiceList is empty, that's the cue to get the voices from the platform again.
        for (auto& tuner : m_platformTVTuner->getSources())
            m_sourceList.append(TVSource::create(tuner));
    }
    return m_sourceList;
}

void  TVTuner::setCurrentSource (TVTuner::SourceType sourceType) {
    //m_currentSourceType = sourceType
    if (m_platformTVTuner) {
        // If the voiceList is empty, that's the cue to get the voices from the platform again.
        m_platformTVTuner->setCurrentSource((PlatformTVSource::Type)sourceType);
    }
}

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
