#ifndef PlatformTVChannel_h
#define PlatformTVChannel_h

#if ENABLE(TV_CONTROL)

#include <wtf/text/WTFString.h>
#include <wtf/RefCounted.h>
#include <wtf/RefPtr.h>
#include <wtf/Vector.h>

namespace WebCore {

enum TVChannelType : short;
struct TVSource;

class PlatformTVChannelClient {
public:
protected:
    virtual ~PlatformTVChannelClient() { }
};

class PlatformTVChannel : public RefCounted<PlatformTVChannel> {
public:
public:
    static RefPtr<PlatformTVChannel> create ();
    
    virtual ~PlatformTVChannel ();
    
    const String                   networkId () const { return m_networkId; }
    const String                   transportStreamId () const { return m_transportStreamId; }
    const String                   serviceId () const { return m_serviceId; }
    //TVSource*                       source () const { return m_source; } //Check how to get source details
    TVChannelType                  type () const { return m_type; }
    const String                   name () const { return m_name; }
    const String                   number () const { return m_number; }
    bool                           isEmergency () const { return m_isEmergency; }
    void                           setChannelClient (PlatformTVChannelClient* client);
    
private:
    PlatformTVChannel ();

    String            m_networkId; 
    String            m_transportStreamId;
    String            m_serviceId;
    TVSource*         m_source;
    TVChannelType     m_type;
    String            m_name;
    String            m_number;
    bool                     m_isEmergency;
    PlatformTVChannelClient* m_PlatformTVChannelClient;
};

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // PlatformTVChannel_h
