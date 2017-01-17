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

bool PlatformTVTuner::setCurrentSource(PlatformTVSource::Type sourceType)
{
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    tvcontrol_return ret = TVControlFailed;
    ret = wpe_tvcontrol_backend_set_current_source(m_tvBackend->m_backend, m_tunerId.utf8().data(), (SourceType)sourceType);
    if (ret == TVControlFailed || ret == TVControlNotImplemented)
        return false;
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return true;
}

double PlatformTVTuner::signalStrength()
{
    wpe_tvcontrol_backend_get_signal_strength(m_tvBackend->m_backend, m_tunerId.utf8().data(), &m_signalStrength);
    return m_signalStrength;
}

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
