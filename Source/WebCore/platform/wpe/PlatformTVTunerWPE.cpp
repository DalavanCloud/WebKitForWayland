#include "config.h"
#include "PlatformTVTuner.h"

#if ENABLE(TV_CONTROL)

namespace WebCore {

RefPtr<PlatformTVTuner> PlatformTVTuner::create(String Id)
{
    return adoptRef(new PlatformTVTuner(Id));
}

PlatformTVTuner::PlatformTVTuner(String Id)
    : m_tunerId(Id)
    , m_platformTVTunerClient(nullptr)
    , m_sourceTypeListIsInitialized(false)
    , m_sourceListIsInitialized(false)
{
}

PlatformTVTuner::~PlatformTVTuner()
{
}

void PlatformTVTuner::setTunerClient(PlatformTVTunerClient* client)
{
    m_platformTVTunerClient = client;
}

const Vector<PlatformTVSource::Type>& PlatformTVTuner::getSupportedSourceTypes()
{
    if (!m_sourceTypeListIsInitialized) {
        ASSERT(m_sourceListType.isEmpty());
        //Do steps to identify tuners;
        m_sourceTypeListIsInitialized = true;
    }
    return m_sourceTypeList;
}

const Vector<RefPtr<PlatformTVSource>>& PlatformTVTuner::getSources()
{
    if (!m_sourceListIsInitialized) {
        ASSERT(m_sourceList.isEmpty());
        //Do steps to identify tuners;
        m_sourceListIsInitialized = true;
    }
    return m_sourceList;
}

void  PlatformTVTuner::setCurrentSource (PlatformTVSource::Type sourceType) {
    m_currentSourceType = sourceType;
}

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
