#ifndef TVSource_h
#define TVSource_h

#if ENABLE(TV_CONTROL)

#include "PlatformTVSource.h"
#include "JSDOMPromise.h"
#include "JSTVChannel.h"
#include "TVChannel.h"
#include "TVScanningStateChangedEvent.h"
#include "EventNames.h"

namespace WebCore {

class TVTuner;

class TVSource : public RefCounted<TVSource>, public PlatformTVSourceClient, public EventTargetWithInlineData {
public:

    struct StartScanningOptions {
        bool isRescanned;
    };

    static Ref<TVSource> create (RefPtr<PlatformTVSource>, TVTuner*);
    virtual ~TVSource () { }

    enum class Type { DvbT, DvbT2, DvbC, DvbC2, DvbS, DvbS2, DvbH, DvbSh, Atsc, AtscMH, IsdbT, IsdbTb, IsdbS, IsdbC, _1seg, Dtmb, Cmmb, TDmb, SDmb };

    typedef DOMPromise<TVChannelVector> TVChannelPromise;
    typedef DOMPromise<std::nullptr_t> TVPromise;

    void getChannels(TVChannelPromise&&);
    void setCurrentChannel (const String& channelNumber, TVPromise&&);
    void startScanning (const StartScanningOptions&, TVPromise&&);
    void stopScanning(TVPromise&&);
    void dispatchScanningStateChangedEvent(RefPtr<PlatformTVChannel> platformTVChannel, uint16_t state);

    TVTuner*                        tuner () const { return m_parentTVTuner; }
    Type                            type () const { return ((Type)m_platformTVSource->type()); }
    bool                            isScanning () const { return m_isScanning; }
    TVChannel*                      currentChannel () const { return m_channel; }

    using RefCounted<TVSource>::ref;
    using RefCounted<TVSource>::deref;

private:
    explicit TVSource (RefPtr<PlatformTVSource>,  TVTuner*);
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
    ScriptExecutionContext* scriptExecutionContext() const override { return nullptr; }
};
typedef Vector<RefPtr<TVSource>> TVSourceVector;
} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // TVSource_h
