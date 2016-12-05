#ifndef PlatformTVSource_h
#define PlatformTVSource_h

#if ENABLE(TV_CONTROL)

#include "PlatformTVChannel.h"

namespace WebCore {

class PlatformTVControlBackend;

class PlatformTVSourceClient {
public:
protected:
    virtual ~PlatformTVSourceClient() { }
};


class PlatformTVSource : public RefCounted<PlatformTVSource> {
public:
    static RefPtr<PlatformTVSource> create(PlatformTVControlBackend*, String);

    virtual ~PlatformTVSource();

    enum class Type { DvbT, DvbT2, DvbC, DvbC2, DvbS, DvbS2, DvbH, DvbSh, Atsc, AtscMH, IsdbT, IsdbTb, IsdbS, IsdbC, _1seg, Dtmb, Cmmb, TDmb, SDmb };

    const Vector<RefPtr<PlatformTVChannel>>&   getChannels ();
    PlatformTVChannel*                         setCurrentChannel (const String& channelNumber);
    void                                       startScanning ();
    void                                       stopScanning ();
    void                                       setSourceClient (PlatformTVSourceClient* client);

    //PlatformTVTuner*                         tuner () const { return m_tuner; } //need to check the tuner part again
    Type                                       type () const { return m_type; }
    bool                                       isScanning () const { return m_isScanning; }

private:
    PlatformTVSource (PlatformTVControlBackend*, String);
    String                      m_tunerId;
    Type                        m_type;
    bool                        m_isScanning;
    PlatformTVChannel*          m_currentChannel;
    PlatformTVSourceClient*     m_platformTVSourceClient;
    PlatformTVControlBackend*   m_tvBackend;

    Vector<RefPtr<PlatformTVChannel>> m_channelList;
    bool m_channelListIsInitialized;
};

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // PlatformTVSource_h
