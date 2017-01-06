#include "config.h"
#include "PlatformTVSource.h"

#if ENABLE(TV_CONTROL)

#include "PlatformTVControl.h"
#include <wpe/tvcontrol-backend.h>

namespace WebCore {

RefPtr<PlatformTVSource> PlatformTVSource::create(PlatformTVControlBackend* tvBackend, String tunerId, Type type)
{
    return adoptRef(new PlatformTVSource(tvBackend, tunerId, type));
}

PlatformTVSource::PlatformTVSource(PlatformTVControlBackend* tvBackend, String tunerId, Type type)
    : m_tunerId(tunerId)
    , m_type(type)
    , m_tvBackend(tvBackend)
    , m_platformTVSourceClient(nullptr)
{
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
}

PlatformTVSource::~PlatformTVSource()
{
}

void PlatformTVSource::setSourceClient(PlatformTVSourceClient* client)
{
    m_platformTVSourceClient = client;
}

bool PlatformTVSource::setCurrentChannel (const String& channelNumber) {
    tvcontrol_return ret = TVControlFailed;
    ret = wpe_tvcontrol_backend_set_current_channel(m_tvBackend->m_backend, m_tunerId.utf8().data(), (SourceType)m_type, atoi(channelNumber.utf8().data()));
    if (ret == TVControlFailed || ret == TVControlNotImplemented)
        return false;
    return true;
}

bool PlatformTVSource::startScanning (bool scanningOption) {
    tvcontrol_return ret = TVControlFailed;
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    ret = wpe_tvcontrol_backend_start_scanning(m_tvBackend->m_backend, m_tunerId.utf8().data(), (SourceType)m_type, scanningOption);
    if (ret == TVControlFailed || ret == TVControlNotImplemented)
        return false;
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return true;
}

bool PlatformTVSource::stopScanning () {
    tvcontrol_return ret = TVControlFailed;
    wpe_tvcontrol_backend_stop_scanning(m_tvBackend->m_backend, m_tunerId.utf8().data());
    if (ret == TVControlFailed || ret == TVControlNotImplemented)
        return false;
    return true;
}

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
