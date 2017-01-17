
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

#ifndef TVTuner_h
#define TVTuner_h

#if ENABLE(TV_CONTROL)

#include "ActiveDOMObject.h"
#include "EventNames.h"
#include "JSDOMPromise.h"
#include "JSTVSource.h"
#include "PlatformTVTuner.h"
#include "ScriptExecutionContext.h"
#include "TVCurrentSourceChangedEvent.h"
#include "TVSource.h"

namespace WebCore {

class TVTuner : public RefCounted<TVTuner>, public PlatformTVTunerClient, public ContextDestructionObserver, public EventTargetWithInlineData {
public:
    static Ref<TVTuner> create(ScriptExecutionContext*, RefPtr<PlatformTVTuner> platformTVTuner);
    virtual ~TVTuner();

    enum class SourceType {
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

    typedef DOMPromise<std::nullptr_t> TVPromise;
    typedef DOMPromise<TVSourceVector> TVSourcePromise;

    const Vector<SourceType>& getSupportedSourceTypes();
    void getSources(TVSourcePromise&&);
    void setCurrentSource(SourceType sourceType, TVPromise&&);

    const String& id() const { return m_platformTVTuner->id(); }
    RefPtr<TVSource> currentSource() const { return m_currentSource; }
    double signalStrength() const { return m_platformTVTuner->signalStrength(); }
    void dispatchSourceChangedEvent();

    using RefCounted<TVTuner>::ref;
    using RefCounted<TVTuner>::deref;

private:
    TVTuner(ScriptExecutionContext*, RefPtr<PlatformTVTuner> platformTVTuner);
    RefPtr<PlatformTVTuner> m_platformTVTuner;
    Vector<RefPtr<TVSource> > m_sourceList;
    Vector<SourceType> m_sourceTypeList;
    RefPtr<TVSource> m_currentSource;

    void refEventTarget() override { ref(); }
    void derefEventTarget() override { deref(); }

    void contextDestroyed() final;
    EventTargetInterface eventTargetInterface() const override { return TVTunerEventTargetInterfaceType; }
    ScriptExecutionContext* scriptExecutionContext() const override;
};
typedef Vector<RefPtr<TVTuner> > TVTunerVector;

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // TVTuner_h
