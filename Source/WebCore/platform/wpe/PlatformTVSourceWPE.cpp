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

bool PlatformTVSource::getChannels(Vector<RefPtr<PlatformTVChannel>>& channelVector) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    struct wpe_tvcontrol_channel_vector *channelList;
    tvcontrol_return ret = TVControlFailed;
    ret = wpe_tvcontrol_backend_get_channel_list(m_tvBackend->m_backend, m_tunerId.utf8().data(), (SourceType)m_type, &channelList);
    if (ret == TVControlFailed || ret == TVControlNotImplemented)
        return false;
    if (channelList) {
        if (channelList->length) {
            for (uint64_t i = 0; i < channelList->length; i++) {
                m_tvBackend->m_channel = &channelList->channels[i];
                channelVector.append(PlatformTVChannel::create(m_tvBackend, m_tunerId.utf8().data()));
            }
            return true;
        }
    }
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return false;
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
