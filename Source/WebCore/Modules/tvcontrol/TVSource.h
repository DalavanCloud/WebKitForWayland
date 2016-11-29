#ifndef TVSource_h
#define TVSource_h

#if ENABLE(TV_CONTROL)

#include "TVChannel.h"
#include "PlatformTVSource.h"

namespace WebCore {

class TVTuner;

enum TVSourceType : short {
    DVB_T,
    DVB_T2,
    DVB_C,
    DVB_C2,
    DVB_S,
    DVB_S2,
    DVB_H,
    DVB_SH,
    ATSC,
    ATSC_M_H,
    ISDB_T,
    ISDB_TB,
    ISDB_S,
    ISDB_C,
    ISEG,
    DTMB,
    CMMB,
    T_DMB,
    S_DMB
};

class TVSource : public RefCounted<TVSource>, public PlatformTVSourceClient, public EventTargetWithInlineData {
public:
    static Ref<TVSource> create (RefPtr<PlatformTVSource>);
    virtual ~TVSource () { }
    
    struct TVStartScanningOptions {
        bool isRescanned;
    };

    const Vector<RefPtr<TVChannel>>&   getChannels ();
    void                            setCurrentChannel (String& channelNumber);
    void                            startScanning (TVStartScanningOptions options);
    void                            stopScanning ();

    TVTuner*                        tuner () const { return m_tuner; }
    TVSourceType                    type () const { return m_platformTVSource->type(); }
    bool                            isScanning () const { return m_platformTVSource->isScanning(); }
    TVChannel*                      currentChannel () const { return m_channel; }
   
    using RefCounted<TVSource>::ref;
    using RefCounted<TVSource>::deref;

private:
    explicit TVSource (RefPtr<PlatformTVSource>);
    RefPtr<PlatformTVSource>   m_platformTVSource;
    
    Vector<RefPtr<TVChannel>>  m_channelList;
    TVTuner*                   m_tuner;
    TVChannel*                 m_channel;

    EventTargetInterface eventTargetInterface() const override { return TVSourceEventTargetInterfaceType; }
    void refEventTarget() override { ref(); }
    void derefEventTarget() override { deref(); } 
};

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // TVSource_h
