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

#ifndef TVScanningStateChangedEvent_h
#define TVScanningStateChangedEvent_h

#if ENABLE(TV_CONTROL)

#include "Event.h"
#include "JSTVChannel.h"

namespace WebCore {

class TVChannel;

class TVScanningStateChangedEvent : public Event {
public:
    enum class State {
        Cleared,
        Scanned,
        Completed,
        Stopped
    };

    static Ref<TVScanningStateChangedEvent> create(const AtomicString&, State, RefPtr<TVChannel>);
    ~TVScanningStateChangedEvent();

    State state() const { return m_state; }
    RefPtr<TVChannel> channel() const { return m_channel; }
    virtual EventInterface eventInterface() const { return TVScanningStateChangedEventInterfaceType; }

private:
    TVScanningStateChangedEvent(const AtomicString&, State, RefPtr<TVChannel>);

    State m_state;
    RefPtr<TVChannel> m_channel;
};

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // TVScanningStateChangedEvent_h