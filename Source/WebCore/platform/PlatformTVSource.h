#ifndef PlatformTVSource_h
#define PlatformTVSource_h

#if ENABLE(TV_CONTROL)

#include "PlatformTVChannel.h"

namespace WebCore {

enum TVSourceType : short;

struct PlatformTVTuner;
    
class PlatformTVSourceClient {
public:
protected:
    virtual ~PlatformTVSourceClient() { }
};


class PlatformTVSource : public RefCounted<PlatformTVSource> {
public:
    static RefPtr<PlatformTVSource> create();
    
    virtual ~PlatformTVSource();
    
    const Vector<RefPtr<PlatformTVChannel>>&   getChannels ();
    PlatformTVChannel*                         setCurrentChannel (String& channelNumber);
    void                                       startScanning ();
    void                                       stopScanning ();
    void                                       setSourceClient (PlatformTVSourceClient* client);
    
    //PlatformTVTuner*                         tuner () const { return m_tuner; } //need to check the tuner part again
    TVSourceType                               type () const { return m_type; }
    bool                                       isScanning () const { return m_isScanning; }
   
private:
    PlatformTVSource ();
    PlatformTVTuner*       m_tuner;
    TVSourceType           m_type;
    bool                   m_isScanning;
    PlatformTVChannel*     m_currentChannel;

    PlatformTVSourceClient* m_PlatformTVSourceClient;

    Vector<RefPtr<PlatformTVChannel>> m_channelList;
    bool m_channelListIsInitialized;
};

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // PlatformTVSource_h
