#include "config.h"
#include "TVRecordingChangedEvent.h"

#if ENABLE(TV_RECORDING_CHANGED)

namespace WebCore {
Ref<TVRecordingChangedEvent> TVRecordingChangedEvent::create(const AtomicString& type,const String& id,const String& errorName,bool isTimeChange,bool isStateChange)
{
        return adoptRef(*new TVRecordingChangedEvent(id,errorName,isTimeChange,isStateChange));
}
TVRecordingChangedEvent::TVRecordingChangedEvent(const AtomicString& type,const String& id,const String& errorName,bool isTimeChange,bool isStateChange)
	:Event(type,false,false)
	,m_id(id)
	,m_errorName(errorName)
	,m_isTimeChange(isTimeChange)
	,m_isStateChange(isStateChange)
{
}
}


#endif

