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

#ifndef PlatformTVSource_h
#define PlatformTVSource_h

#if ENABLE(TV_CONTROL)

#include "PlatformTVChannel.h"

namespace WebCore {

class PlatformTVControlBackend;

class PlatformTVSourceClient {
public:
protected:
    virtual ~PlatformTVSourceClient() {}
};

class PlatformTVSource : public RefCounted<PlatformTVSource> {
public:
    enum class Type {
        DvbT,
        DvbT2,
        DvbC,
        DvbC2,
        DvbS,
        DvbS2,
        DvbH,
        DvbSh,
        Atsc,
        AtscMH,
        IsdbT,
        IsdbTb,
        IsdbS,
        IsdbC,
        _1seg,
        Dtmb,
        Cmmb,
        TDmb,
        SDmb,
        Undefined };

    static RefPtr<PlatformTVSource> create(PlatformTVControlBackend*, String, Type);
    virtual ~PlatformTVSource();

    bool setCurrentChannel(const String& channelNumber);
    bool startScanning(bool scanningOption);
    bool stopScanning();
    void setSourceClient(PlatformTVSourceClient* client);
    bool getChannels(Vector<RefPtr<PlatformTVChannel>>& channelVector);

    Type type() const { return m_type; }

private:
    PlatformTVSource(PlatformTVControlBackend*, String, Type);
    String m_tunerId;
    Type m_type;
    PlatformTVControlBackend* m_tvBackend;
    PlatformTVSourceClient* m_platformTVSourceClient;
};

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // PlatformTVSource_h
