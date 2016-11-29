#ifndef TVManager_h
#define TVManager_h

#if ENABLE(TV_CONTROL)

#include "TVTuner.h"
#include "PlatformTVManager.h"

namespace WebCore {

class TVManager : public RefCounted<TVManager>, public EventTargetWithInlineData, public EventTargetWithInlineData {
public:
    static Ref<TVManager> create ();
    ~TVManager ();
    const Vector<RefPtr<TVTuner>>& getTuners();

    using RefCounted<TVManager>::ref;
    using RefCounted<TVManager>::deref;

private:
    TVManager ();
    std::unique_ptr<PlatformTVManager> m_platformTVManager;
    Vector<RefPtr<TVTuner>> m_tunerList;

    EventTargetInterface eventTargetInterface() const override { return TVManagerEventTargetInterfaceType; }
    void refEventTarget() override { ref(); }
    void derefEventTarget() override { deref(); }
};

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // TVManager_h
