/*
 * Copyright (C) 2017 TATA ELXSI
 * Copyright (C) 2017 Metrological
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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

bool PlatformTVSource::getChannels(Vector<RefPtr<PlatformTVChannel>>& channelVector)
{
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    struct wpe_tvcontrol_channel_vector* channelList;
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

bool PlatformTVSource::setCurrentChannel(const String& channelNumber)
{
    tvcontrol_return ret = TVControlFailed;
    ret = wpe_tvcontrol_backend_set_current_channel(m_tvBackend->m_backend, m_tunerId.utf8().data(), (SourceType)m_type, atoi(channelNumber.utf8().data()));
    if (ret == TVControlFailed || ret == TVControlNotImplemented)
        return false;
    return true;
}

bool PlatformTVSource::startScanning(bool scanningOption)
{
    tvcontrol_return ret = TVControlFailed;
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    ret = wpe_tvcontrol_backend_start_scanning(m_tvBackend->m_backend, m_tunerId.utf8().data(), (SourceType)m_type, scanningOption);
    if (ret == TVControlFailed || ret == TVControlNotImplemented)
        return false;
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return true;
}

bool PlatformTVSource::stopScanning()
{
    tvcontrol_return ret = TVControlFailed;
    wpe_tvcontrol_backend_stop_scanning(m_tvBackend->m_backend, m_tunerId.utf8().data());
    if (ret == TVControlFailed || ret == TVControlNotImplemented)
        return false;
    return true;
}

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
