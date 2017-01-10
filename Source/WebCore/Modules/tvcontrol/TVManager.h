#ifndef TVManager_h
#define TVManager_h

#if ENABLE(TV_CONTROL)

#include "ActiveDOMObject.h"
#include "ScriptExecutionContext.h"
#include "PlatformTVManager.h"
#include "JSDOMPromise.h"
#include "JSTVTuner.h"
#include "TVTuner.h"

namespace WebCore {

class Document;
class Frame;
class ScriptExecutionContext;

class TVManager : public RefCounted<TVManager>, public PlatformTVManagerClient, public ActiveDOMObject, public EventTargetWithInlineData {
public:
    static Ref<TVManager> create (ScriptExecutionContext*);
    WEBCORE_EXPORT ~TVManager ();

    Document* document() const;

    typedef DOMPromise<TVTunerVector> TVTunerPromise;
    void getTuners(TVTunerPromise&& );

    using RefCounted<TVManager>::ref;
    using RefCounted<TVManager>::deref;

    void didTunerOperationChanged(String tunerId, uint16_t event) override;
    void didCurrentSourceChanged(String tunerId) override;
    void didCurrentChannelChanged(String tunerId) override;
    void didScanningStateChanged(String tunerId, RefPtr<PlatformTVChannel>, uint16_t state) override;

private:
    TVManager (ScriptExecutionContext*);
    std::unique_ptr<PlatformTVManager> m_platformTVManager;
    Vector<RefPtr<TVTuner>> m_tunerList;

    void refEventTarget() override { ref(); }
    void derefEventTarget() override { deref(); }
    virtual const char* activeDOMObjectName() const override  {return "TVManager";}
    virtual bool canSuspendForDocumentSuspension() const override {return false;}
    ScriptExecutionContext* scriptExecutionContext() const override;
    EventTargetInterface eventTargetInterface() const override { return TVManagerEventTargetInterfaceType; }
};

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // TVManager_h
