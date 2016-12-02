#ifndef TVManager_h
#define TVManager_h

#if ENABLE(TV_CONTROL)

#include "ActiveDOMObject.h"
#include "TVTuner.h"
#include "PlatformTVManager.h"

namespace WebCore {

class Navigator;
class ScriptExecutionContext;

class TVManager : public RefCounted<TVManager>, public PlatformTVManagerClient, public ActiveDOMObject, public EventTargetWithInlineData {
public:
    static Ref<TVManager> create (Navigator*);
    ~TVManager ();
    const Vector<RefPtr<TVTuner>>& getTuners();

    using RefCounted<TVManager>::ref;
    using RefCounted<TVManager>::deref;

private:
    TVManager (Navigator*);
    std::unique_ptr<PlatformTVManager> m_platformTVManager;
    Vector<RefPtr<TVTuner>> m_tunerList;

    void refEventTarget() override { ref(); }
    void derefEventTarget() override { deref(); }
    virtual const char* activeDOMObjectName() const override  { return 0; }
    virtual bool canSuspendForDocumentSuspension() const override {return false;}
    ScriptExecutionContext* scriptExecutionContext() const override;
    EventTargetInterface eventTargetInterface() const override { return TVManagerEventTargetInterfaceType; }
};

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // TVManager_h
