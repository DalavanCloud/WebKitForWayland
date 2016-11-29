#ifndef TVChannel_h
#define TVChannel_h

#if ENABLE(TV_CONTROL)

#include "EventTarget.h"
#include "PlatformTVChannel.h"

namespace WebCore {

class TVSource;

enum TVChannelType : short {
    TV,
    RADIO,
    DATA
};

 
class TVChannel : public RefCounted<TVChannel>, public PlatformTVChannel, public EventTargetWithInlineData {
public:
    static Ref<TVChannel> create (RefPtr<PlatformTVChannel>);
    virtual ~TVChannel () { }

    const String&                   networkId () const { return m_platformTVChannel->networkId(); }
    const String&                   transportStreamId () const { return m_platformTVChannel->transportStreamId(); }
    const String&                   serviceId () const { return m_platformTVChannel->serviceId(); }
//    TVSource*                       source () const { return m_source; }
    TVChannelType                   type () const { return m_platformTVChannel->type(); }
    const String&                   name () const { return m_platformTVChannel->name(); }
    const String&                   number () const { return m_platformTVChannel->number(); }
    bool                            isEmergency () const { return m_platformTVChannel->isEmergency(); }

    using RefCounted<TVChannel>::ref;
    using RefCounted<TVChannel>::deref;

private:
    explicit TVChannel (RefPtr<PlatformTVChannel>);
    
    RefPtr<PlatformTVChannel>   m_platformTVChannel;
//    TVSource*                   m_source;

    EventTargetInterface eventTargetInterface() const override { return TVChannelEventTargetInterfaceType; }
    void refEventTarget() override { ref(); }
    void derefEventTarget() override { deref(); }
};

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // TVChannel_h
