#ifndef TVSource_h
#define TVSource_h

#if ENABLE(TV_CONTROL)

#include "TVChannel.h"
#include "PlatformTVSource.h"

namespace WebCore {

class TVTuner;

class TVSource : public RefCounted<TVSource>, public PlatformTVSourceClient, public EventTargetWithInlineData {
public:
    static Ref<TVSource> create (RefPtr<PlatformTVSource>, TVTuner*);
    virtual ~TVSource () { }

    enum class Type { DvbT, DvbT2, DvbC, DvbC2, DvbS, DvbS2, DvbH, DvbSh, Atsc, AtscMH, IsdbT, IsdbTb, IsdbS, IsdbC, _1seg, Dtmb, Cmmb, TDmb, SDmb };

    struct Options {
        bool isRescanned;
    };

    const Vector<RefPtr<TVChannel>>&   getChannels ();
    void                            setCurrentChannel (const String& channelNumber);
    void                            startScanning (/*const Options&*/);
    void                            stopScanning ();

    TVTuner*                        tuner () const { return m_parentTVTuner; }
    Type                            type () const { return ((Type)m_platformTVSource->type()); }
    bool                            isScanning () const { return m_platformTVSource->isScanning(); }
    TVChannel*                      currentChannel () const { return m_channel; }

    using RefCounted<TVSource>::ref;
    using RefCounted<TVSource>::deref;

private:
    explicit TVSource (RefPtr<PlatformTVSource>,  TVTuner*);
    RefPtr<PlatformTVSource>   m_platformTVSource;
    TVTuner*                   m_parentTVTuner;

    Vector<RefPtr<TVChannel>>  m_channelList;
    TVChannel*                 m_channel;

    void refEventTarget() override { ref(); }
    void derefEventTarget() override { deref(); }

    EventTargetInterface eventTargetInterface() const override { return TVSourceEventTargetInterfaceType; }
    ScriptExecutionContext* scriptExecutionContext() const override { return nullptr; }
};

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // TVSource_h
