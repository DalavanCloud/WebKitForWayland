#ifndef TVChannel_h
#define TVChannel_h

#if ENABLE(TV_CONTROL)

#include "EventTarget.h"
#include "PlatformTVChannel.h"

namespace WebCore {

class TVSource;

class TVChannel final : public RefCounted<TVChannel>, public PlatformTVChannelClient, public EventTargetWithInlineData {
public:
    static Ref<TVChannel> create (RefPtr<PlatformTVChannel>, TVSource*);
    ~TVChannel () { }

    enum class Type {
       Tv,
       Radio,
       Data
    };

    const String                    networkId () const { return m_platformTVChannel->networkId(); }
    const String                    transportStreamId () const { return m_platformTVChannel->transportStreamId(); }
    const String                    serviceId () const { return m_platformTVChannel->serviceId(); }
    TVSource*                       source () const { return m_parentTVSource; }
    Type                            type () const {  return ((Type)m_platformTVChannel->type()); }
    const String                    name () const { return m_platformTVChannel->name(); }
    const String                    number () const { return m_platformTVChannel->number(); }
    bool                            isEmergency () const { return m_platformTVChannel->isEmergency(); }
    bool                            isFree () const { return true; }

    using RefCounted<TVChannel>::ref;
    using RefCounted<TVChannel>::deref;

private:
    explicit TVChannel (RefPtr<PlatformTVChannel>, TVSource*);

    RefPtr<PlatformTVChannel>   m_platformTVChannel;
    TVSource*                   m_parentTVSource;

    void refEventTarget() override { ref(); }
    void derefEventTarget() override { deref(); }

    EventTargetInterface eventTargetInterface() const override { return TVChannelEventTargetInterfaceType; }
    ScriptExecutionContext* scriptExecutionContext() const override { return nullptr; }
};

typedef Vector<RefPtr<TVChannel>> TVChannelVector;

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // TVChannel_h
