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

bool PlatformTVTuner::getSupportedSourceTypes(Vector<PlatformTVSource::Type>& sourceTypeVector)
{
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    struct wpe_tvcontrol_src_types_vector sourceTypeList;
    sourceTypeList.length = 0;
    wpe_tvcontrol_backend_get_supported_source_types_list(m_tvBackend->m_backend, m_tunerId.utf8().data(), &sourceTypeList);
    if (sourceTypeList.length) {
        for (uint64_t i = 0; i < sourceTypeList.length; i++) {
            sourceTypeVector.append(PlatformTVSource::Type(sourceTypeList.types[i]));
        }
        return true;
    }
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return false;
}

bool PlatformTVTuner::getSources(Vector<RefPtr<PlatformTVSource>>& sourceVector)
{
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    struct wpe_tvcontrol_src_types_vector sourceList;
    sourceList.length = 0;
    tvcontrol_return ret = TVControlFailed;
    ret = wpe_tvcontrol_backend_get_source_list(m_tvBackend->m_backend, m_tunerId.utf8().data(), &sourceList);
    if (ret == TVControlFailed || ret == TVControlNotImplemented)
        return false;
    if (sourceList.length) {
        for (uint64_t i = 0; i < sourceList.length; i++) {
            sourceVector.append(PlatformTVSource::create(m_tvBackend, m_tunerId.utf8().data(), PlatformTVSource::Type(sourceList.types[i])));
        }
        return true;
    }
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return false;
}

bool PlatformTVTuner::setCurrentSource(PlatformTVSource::Type sourceType) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    tvcontrol_return ret = TVControlFailed;
    ret = wpe_tvcontrol_backend_set_current_source(m_tvBackend->m_backend, m_tunerId.utf8().data(), (SourceType)sourceType);
    if (ret == TVControlFailed || ret == TVControlNotImplemented)
        return false;
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return true;
}

double PlatformTVTuner::signalStrength() {
    wpe_tvcontrol_backend_get_signal_strength(m_tvBackend->m_backend, m_tunerId.utf8().data(), &m_signalStrength);
    return m_signalStrength;
}

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
