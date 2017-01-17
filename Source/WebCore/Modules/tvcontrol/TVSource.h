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

#ifndef TVSource_h
#define TVSource_h

#if ENABLE(TV_CONTROL)

#include "ActiveDOMObject.h"
#include "EventNames.h"
#include "JSDOMPromise.h"
#include "JSTVChannel.h"
#include "PlatformTVSource.h"
#include "ScriptExecutionContext.h"
#include "TVChannel.h"
#include "TVScanningStateChangedEvent.h"
#include "TVCurrentChannelChangedEvent.h"

namespace WebCore {

class TVTuner;

class TVSource : public RefCounted<TVSource>, public PlatformTVSourceClient, public ContextDestructionObserver, public EventTargetWithInlineData {
public:
    struct StartScanningOptions {
        bool isRescanned;
    };

    static Ref<TVSource> create(ScriptExecutionContext*, RefPtr<PlatformTVSource>, TVTuner*);
    virtual ~TVSource();

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
        SDmb
    };

    typedef DOMPromise<TVChannelVector> TVChannelPromise;
    typedef DOMPromise<std::nullptr_t> TVPromise;

    void getChannels(TVChannelPromise&&);
    void setCurrentChannel(const String& channelNumber, TVPromise&&);
    RefPtr<TVChannel> currentChannel() const { return m_currentChannel; }
    void startScanning(const StartScanningOptions&, TVPromise&&);
    void stopScanning(TVPromise&&);
    void dispatchScanningStateChangedEvent(RefPtr<PlatformTVChannel> platformTVChannel, uint16_t state);
    void dispatchChannelChangedEvent();
    TVTuner* tuner() const { return m_parentTVTuner; }
    Type type() const { return ((Type)m_platformTVSource->type()); }
    bool isScanning() const { return m_isScanning; }

    using RefCounted<TVSource>::ref;
    using RefCounted<TVSource>::deref;

private:
    TVSource(ScriptExecutionContext*, RefPtr<PlatformTVSource>, TVTuner*);
    RefPtr<PlatformTVSource> m_platformTVSource;
    TVTuner* m_parentTVTuner;

    Vector<RefPtr<TVChannel> > m_channelList;
    RefPtr<TVChannel> m_currentChannel;

    enum ScanningState {
        SCANNING_NOT_INITIALISED,
        SCANNING_STARTED,
        SCANNING_COMPLETED
    };
    ScanningState m_scanState;
    bool m_isScanning;

    void refEventTarget() override { ref(); }
    void derefEventTarget() override { deref(); }

    void contextDestroyed() final;
    EventTargetInterface eventTargetInterface() const override { return TVSourceEventTargetInterfaceType; }
    ScriptExecutionContext* scriptExecutionContext() const override;
};
typedef Vector<RefPtr<TVSource> > TVSourceVector;

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // TVSource_h
