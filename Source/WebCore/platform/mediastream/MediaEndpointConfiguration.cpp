/*
 * Copyright (C) 2015 Ericsson AB. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of Ericsson nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "MediaEndpointConfiguration.h"

#if ENABLE(WEB_RTC)

namespace WebCore {

IceServerInfo::IceServerInfo(const Vector<String>& urls, const String& credential, const String& username)
    : m_credential(credential)
    , m_username(username)
{
    m_urls.reserveCapacity(urls.size());
    for (auto& url : urls)
        m_urls.append(URL(URL(), url));
}

MediaEndpointConfiguration::MediaEndpointConfiguration(Vector<RefPtr<IceServerInfo>>& iceServers, const String& iceTransportPolicy, const String& bundlePolicy)
    : m_iceServers(iceServers)
{
    if (iceTransportPolicy == "none")
        m_iceTransportPolicy = IceTransportPolicy::None;
    else if (iceTransportPolicy == "relay")
        m_iceTransportPolicy = IceTransportPolicy::Relay;
    else if (iceTransportPolicy == "all")
        m_iceTransportPolicy = IceTransportPolicy::All;
    else
        ASSERT_NOT_REACHED();

    if (bundlePolicy == "balanced")
        m_bundlePolicy = BundlePolicy::Balanced;
    else if (bundlePolicy == "max-compat")
        m_bundlePolicy = BundlePolicy::MaxCompat;
    else if (bundlePolicy == "max-bundle")
        m_bundlePolicy = BundlePolicy::MaxBundle;
    else
        ASSERT_NOT_REACHED();
}

} // namespace WebCore

#endif // ENABLE(WEB_RTC)
