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

#ifndef TVChannel_h
#define TVChannel_h

#if ENABLE(TV_CONTROL)

#include "EventTarget.h"
#include "PlatformTVChannel.h"

namespace WebCore {

class TVSource;

class TVChannel final : public RefCounted<TVChannel>, public PlatformTVChannelClient, public EventTargetWithInlineData {
public:
    static Ref<TVChannel> create(RefPtr<PlatformTVChannel>, TVSource*);
    ~TVChannel() {}

    enum class Type {
        Tv,
        Radio,
        Data
    };

    const String networkId() const { return m_platformTVChannel->networkId(); }
    const String transportStreamId() const { return m_platformTVChannel->transportStreamId(); }
    const String serviceId() const { return m_platformTVChannel->serviceId(); }
    TVSource* source() const { return m_parentTVSource; }
    Type type() const { return ((Type)m_platformTVChannel->type()); }
    const String name() const { return m_platformTVChannel->name(); }
    const String number() const { return m_platformTVChannel->number(); }
    bool isEmergency() const { return m_platformTVChannel->isEmergency(); }
    bool isFree() const { return true; }

    using RefCounted<TVChannel>::ref;
    using RefCounted<TVChannel>::deref;

private:
    explicit TVChannel(RefPtr<PlatformTVChannel>, TVSource*);

    RefPtr<PlatformTVChannel> m_platformTVChannel;
    TVSource* m_parentTVSource;

    void refEventTarget() override { ref(); }
    void derefEventTarget() override { deref(); }

    EventTargetInterface eventTargetInterface() const override { return TVChannelEventTargetInterfaceType; }
    ScriptExecutionContext* scriptExecutionContext() const override { return nullptr; }
};

typedef Vector<RefPtr<TVChannel> > TVChannelVector;

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // TVChannel_h
