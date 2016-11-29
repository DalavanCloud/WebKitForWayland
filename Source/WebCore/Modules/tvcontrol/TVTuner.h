#ifndef TVTuner_h
#define TVTuner_h

#if ENABLE(TV_CONTROL)

#include "TVSource.h"
#include "PlatformTVTuner.h"
//#include "TVMediaStream.h"

namespace WebCore {

class TVTuner : public RefCounted<TVTuner>, public PlatformTVTunerClient, public EventTargetWithInlineData {
public:
    static Ref<TVTuner> create (RefPtr<PlatformTVTuner> platformTVTuner);
    virtual ~TVTuner () { }

    const Vector<TVSourceType>&     getSupportedSourceTypes ();
    const Vector<RefPtr<TVSource>>& getSources ();
    void                            setCurrentSource (TVSourceType sourceType);
        
    const String&       id () const { return m_platformTVTuner->id(); }
    TVSource*           currentSource() const { return m_currentSource; } //TODO check again
    //TVMediaStream*    stream() const { return nullptr; } //TODO enable if it is required for basic functionalities
    double              signalStrength() const { return m_platformTVTuner->signalStrength(); }

    using RefCounted<TVTuner>::ref;
    using RefCounted<TVTuner>::deref;

private:
    explicit TVTuner (RefPtr<PlatformTVTuner> platformTVTuner);
    RefPtr<PlatformTVTuner>   m_platformTVTuner;
    Vector<RefPtr<TVSource>>  m_sourceList;
    Vector<TVSourceType>      m_sourceTypeList;
    TVSource*                 m_currentSource;

    EventTargetInterface eventTargetInterface() const override { return TVTunerEventTargetInterfaceType; }
    void refEventTarget() override { ref(); }
    void derefEventTarget() override { deref(); }
};

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // TVTuner_h
