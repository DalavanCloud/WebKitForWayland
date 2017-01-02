#ifndef TVSource_h
#define TVSource_h

#if ENABLE(TV_CONTROL)

#include "PlatformTVSource.h"
#include "JSDOMPromise.h"
#include "JSTVChannel.h"
#include "TVChannel.h"
#include "ActiveDOMObject.h"
#include "TVScanningStateChangedEvent.h"
#include "EventNames.h"

namespace WebCore {

class TVTuner;
class Document;
class Frame;
class ScriptExecutionContext;

class TVSource : public RefCounted<TVSource>, public PlatformTVSourceClient, public ActiveDOMObject, public EventTargetWithInlineData {
public:

    struct StartScanningOptions {
        bool isRescanned;
    };

    static Ref<TVSource> create (ScriptExecutionContext*, RefPtr<PlatformTVSource>, TVTuner*);
    Document* document() const;
    WEBCORE_EXPORT Frame* frame() const;
    virtual ~TVSource () { }

    enum class Type { DvbT, DvbT2, DvbC, DvbC2, DvbS, DvbS2, DvbH, DvbSh, Atsc, AtscMH, IsdbT, IsdbTb, IsdbS, IsdbC, _1seg, Dtmb, Cmmb, TDmb, SDmb };

    typedef DOMPromise<TVChannelVector> TVChannelPromise;
    typedef DOMPromise<std::nullptr_t> TVPromise;

    void getChannels(TVChannelPromise&&);
    void setCurrentChannel (const String& channelNumber, TVPromise&&);
    void startScanning (const StartScanningOptions&, TVPromise&&);
    void stopScanning(TVPromise&&);
    void dispatchScanningStateChangedEvent(uint16_t state);

    TVTuner*                        tuner () const { return m_parentTVTuner; }
    Type                            type () const { return ((Type)m_platformTVSource->type()); }
    bool                            isScanning () const { return m_isScanning; }
    TVChannel*                      currentChannel () const { return m_channel; }

    using RefCounted<TVSource>::ref;
    using RefCounted<TVSource>::deref;

private:
    explicit TVSource (ScriptExecutionContext*, RefPtr<PlatformTVSource>,  TVTuner*);
    RefPtr<PlatformTVSource>   m_platformTVSource;
    TVTuner*                   m_parentTVTuner;

    Vector<RefPtr<TVChannel>>  m_channelList;
    TVChannel*                 m_channel;

    enum ScanningState { SCANNING_NOT_INITIALISED, SCANNING_STARTED, SCANNING_COMPLETED };
    ScanningState              m_scanState;
    bool                       m_isScanning;

    void refEventTarget() override { ref(); }
    void derefEventTarget() override { deref(); }

    EventTargetInterface eventTargetInterface() const override { return TVSourceEventTargetInterfaceType; }
    virtual const char* activeDOMObjectName() const override  { return 0; }
    virtual bool canSuspendForDocumentSuspension() const override {return false;}
    ScriptExecutionContext* scriptExecutionContext() const override;
};
typedef Vector<RefPtr<TVSource>> TVSourceVector;
} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // TVSource_h
