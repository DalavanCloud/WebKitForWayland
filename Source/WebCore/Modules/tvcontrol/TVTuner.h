#ifndef TVTuner_h
#define TVTuner_h

#if ENABLE(TV_CONTROL)

#include "JSTVSource.h"
#include "TVSource.h"
#include "PlatformTVTuner.h"
#include "JSDOMPromise.h"
#include "ActiveDOMObject.h"
#include "TVCurrentSourceChangedEvent.h"
#include "EventNames.h"


namespace WebCore {
class Document;
class Frame;
class ScriptExecutionContext;

class TVTuner : public RefCounted<TVTuner>, public PlatformTVTunerClient, public ActiveDOMObject, public EventTargetWithInlineData {
public:
    static Ref<TVTuner> create (ScriptExecutionContext*,RefPtr<PlatformTVTuner> platformTVTuner);

    Document* document() const;
    WEBCORE_EXPORT Frame* frame() const;

    virtual ~TVTuner () { }

    enum class SourceType { DvbT, DvbT2, DvbC, DvbC2, DvbS, DvbS2, DvbH, DvbSh, Atsc, AtscMH, IsdbT, IsdbTb, IsdbS, IsdbC, _1seg, Dtmb, Cmmb, TDmb, SDmb };

    typedef DOMPromise<std::nullptr_t> TVPromise;
    typedef DOMPromise<TVSourceVector> TVSourcePromise;

    const Vector<SourceType>&   getSupportedSourceTypes ();
    void                        getSources(TVSourcePromise&&);
    void                        setCurrentSource (SourceType sourceType, TVPromise&&);
    void                        dispatchSourceChangedEvent();
    const String&       id () const { return m_platformTVTuner->id(); }
    RefPtr<TVSource>    currentSource() const { return m_currentSource; } //TODO check again
    //TVMediaStream*    stream() const { return nullptr; } //TODO enable if it is required for basic functionalities
    double              signalStrength() const { return m_platformTVTuner->signalStrength(); }

    using RefCounted<TVTuner>::ref;
    using RefCounted<TVTuner>::deref;

private:
    explicit TVTuner (ScriptExecutionContext*, RefPtr<PlatformTVTuner> platformTVTuner);
    RefPtr<PlatformTVTuner>   m_platformTVTuner;
    Vector<RefPtr<TVSource>>  m_sourceList;
    Vector<SourceType>        m_sourceTypeList;
    RefPtr<TVSource>          m_currentSource;

    void refEventTarget() override { ref(); }
    void derefEventTarget() override { deref(); }
    EventTargetInterface eventTargetInterface() const override { return TVTunerEventTargetInterfaceType; }
    virtual const char* activeDOMObjectName() const override  { return 0; }
    virtual bool canSuspendForDocumentSuspension() const override {return false;}
    ScriptExecutionContext* scriptExecutionContext() const override;
};

typedef Vector<RefPtr<TVTuner>> TVTunerVector;

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // TVTuner_h
