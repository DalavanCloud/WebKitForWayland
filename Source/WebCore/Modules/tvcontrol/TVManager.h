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

#ifndef TVManager_h
#define TVManager_h

#if ENABLE(TV_CONTROL)

#include "ActiveDOMObject.h"
#include "JSDOMPromise.h"
#include "JSTVTuner.h"
#include "PlatformTVManager.h"
#include "ScriptExecutionContext.h"
#include "TVTuner.h"

namespace WebCore {

class Document;
class Frame;
class ScriptExecutionContext;

class TVManager : public RefCounted<TVManager>, public PlatformTVManagerClient, public ActiveDOMObject, public EventTargetWithInlineData {
public:
    static Ref<TVManager> create(ScriptExecutionContext*);
    WEBCORE_EXPORT ~TVManager();

    Document* document() const;

    typedef DOMPromise<TVTunerVector> TVTunerPromise;
    void getTuners(TVTunerPromise&&);

    using RefCounted<TVManager>::ref;
    using RefCounted<TVManager>::deref;

    void didTunerOperationChanged(String, uint16_t) override;
    void didCurrentSourceChanged(String) override;
    void didCurrentChannelChanged(String) override;
    void didScanningStateChanged(String, RefPtr<PlatformTVChannel>, uint16_t) override;

private:
    TVManager(ScriptExecutionContext*);
    std::unique_ptr<PlatformTVManager> m_platformTVManager;
    Vector<RefPtr<TVTuner> > m_tunerList;

    void refEventTarget() override { ref(); }
    void derefEventTarget() override { deref(); }
    virtual const char* activeDOMObjectName() const override { return "TVManager"; }
    virtual bool canSuspendForDocumentSuspension() const override { return false; }
    ScriptExecutionContext* scriptExecutionContext() const override;
    EventTargetInterface eventTargetInterface() const override { return TVManagerEventTargetInterfaceType; }
};

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // TVManager_h
