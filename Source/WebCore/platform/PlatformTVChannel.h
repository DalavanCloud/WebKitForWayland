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

#ifndef PlatformTVChannel_h
#define PlatformTVChannel_h

#if ENABLE(TV_CONTROL)

#include <wtf/text/WTFString.h>
#include <wtf/RefCounted.h>
#include <wtf/RefPtr.h>
#include <wtf/Vector.h>

namespace WebCore {

class PlatformTVControlBackend;
class PlatformTVSource;

class PlatformTVChannelClient {
public:
protected:
    virtual ~PlatformTVChannelClient() {}
};

class PlatformTVChannel : public RefCounted<PlatformTVChannel> {
public:
    static RefPtr<PlatformTVChannel> create(PlatformTVControlBackend*, String);

    /*virtual*/ ~PlatformTVChannel();

    enum class Type {
	    Tv,
        Radio,
        Data };

    const String& networkId() const { return m_networkId; }
    const String transportStreamId() const { return m_transportStreamId; }
    const String name() const { return m_name; }
    const String number() const { return m_number; }
    const String serviceId() const { return m_serviceId; }

    Type type() const { return m_type; }
    bool isEmergency() const { return m_isEmergency; }
    void setChannelClient(PlatformTVChannelClient* client);

private:
    PlatformTVChannel(PlatformTVControlBackend*, String);

    String m_tunerId;
    String m_networkId;
    String m_transportStreamId;
    String m_serviceId;
    Type m_type;
    String m_name;
    String m_number;
    bool m_isEmergency;
    PlatformTVChannelClient* m_PlatformTVChannelClient;
    PlatformTVControlBackend* m_tvBackend;
};

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // PlatformTVChannel_h