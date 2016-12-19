#include "config.h"
#include "PlatformTVTuner.h"

#if ENABLE(TV_CONTROL)

#include "PlatformTVControl.h"
#include <wpe/tvcontrol-backend.h>

namespace WebCore {

RefPtr<PlatformTVTuner> PlatformTVTuner::create(String Id, PlatformTVControlBackend* tvBackend)
{
    return adoptRef(new PlatformTVTuner(Id, tvBackend));
}

PlatformTVTuner::PlatformTVTuner(String Id, PlatformTVControlBackend* tvBackend)
    : m_tunerId(Id)
    , m_platformTVTunerClient(nullptr)
    , m_tvBackend(tvBackend)
    , m_sourceTypeListIsInitialized(false)
    , m_sourceListIsInitialized(false)
{
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
}

PlatformTVTuner::~PlatformTVTuner()
{
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
}

void PlatformTVTuner::setTunerClient(PlatformTVTunerClient* client)
{
    m_platformTVTunerClient = client;
}

const Vector<PlatformTVSource::Type>& PlatformTVTuner::getSupportedSourceTypes()
{
    if (!m_sourceTypeListIsInitialized) {
        ASSERT(m_sourceListType.isEmpty());
        struct wpe_tvcontrol_src_types_vector sourceTypeList;
        wpe_tvcontrol_backend_get_supported_source_types_list(m_tvBackend->m_backend, m_tunerId.utf8().data(), &sourceTypeList);
        if (sourceTypeList.length) {
            for(uint64_t i = 0; i < sourceTypeList.length; i++) {
                m_sourceTypeList.append(PlatformTVSource::Type(sourceTypeList.types[i]));
            }
            m_sourceTypeListIsInitialized = true;
        }
    }
    return m_sourceTypeList;
}

const Vector<RefPtr<PlatformTVSource>>& PlatformTVTuner::getSources()
{
    if (!m_sourceListIsInitialized) {
        ASSERT(m_sourceList.isEmpty());

        /*Get available source list*/
        struct wpe_tvcontrol_src_types_vector sourceList;
        sourceList.length = 0;
        wpe_tvcontrol_backend_get_source_list(m_tvBackend->m_backend, m_tunerId.utf8().data(), &sourceList);
        if (sourceList.length) {
            for (uint64_t i = 0; i < sourceList.length; i++) {
                m_sourceList.append(PlatformTVSource::create(m_tvBackend, m_tunerId.utf8().data(), PlatformTVSource::Type(sourceList.types[i])));
            }
            m_sourceListIsInitialized = true;
        }
    }
    return m_sourceList;
}

RefPtr<PlatformTVSource> PlatformTVTuner::setCurrentSource(PlatformTVSource::Type sourceType) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    m_currentSourceType = sourceType;
    m_currentSource = nullptr;
    wpe_tvcontrol_backend_set_current_source(m_tvBackend->m_backend, m_tunerId.utf8().data(), (SourceType)sourceType);
    /* Parse the source list and set current source */
    for(auto& src : m_sourceList){
        printf("Type of this src = %d, given type  = %d", src->type(), sourceType);
        if(src->type()  == sourceType ){
            printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
            m_currentSource = src;
        }
    }
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return m_currentSource;
}

double PlatformTVTuner::signalStrength() {
    wpe_tvcontrol_backend_get_signal_strength(m_tvBackend->m_backend, m_tunerId.utf8().data(), &m_signalStrength);
    return m_signalStrength;
}

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
