#ifndef PlatformTVChannel_h
#define PlatformTVChannel_h

#if ENABLE(TV_CONTROL)

#include <wtf/text/WTFString.h>
#include <wtf/RefCounted.h>
#include <wtf/RefPtr.h>
#include <wtf/Vector.h>

namespace WebCore {

class PlatformTVControlBackend;

class PlatformTVChannelClient {
public:
protected:
    virtual ~PlatformTVChannelClient() { }
};

class PlatformTVChannel : public RefCounted<PlatformTVChannel> {
public:
    static RefPtr<PlatformTVChannel> create (PlatformTVControlBackend*, String);

    /*virtual*/ ~PlatformTVChannel ();

    enum class Type { Tv, Radio, Data };

    const String&                   networkId () const { return m_networkId; }
    const String                   transportStreamId () const { return m_transportStreamId; }
    const String                   name () const { return m_name; }
    const String                   number () const { return m_number; }
    const String                   serviceId () const { return m_serviceId; }

    Type                           type () const { return m_type; }
    bool                           isEmergency () const { return m_isEmergency; }
    void                           setChannelClient (PlatformTVChannelClient* client);

private:
    PlatformTVChannel (PlatformTVControlBackend*, String);

    String                     m_tunerId;
    String                     m_networkId;
    String                     m_transportStreamId;
    String                     m_serviceId;
    Type                       m_type;
    String                     m_name;
    String                     m_number;
    bool                       m_isEmergency;
    PlatformTVChannelClient*   m_PlatformTVChannelClient;
    PlatformTVControlBackend*  m_tvBackend;
};

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // PlatformTVChannel_h
