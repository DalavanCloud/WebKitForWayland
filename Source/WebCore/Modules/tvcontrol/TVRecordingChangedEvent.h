#ifndef TVRecordingChangedEvent_h
#define TVRecordingChangedEvent_h

#if ENABLE(TV_RECORDING_CHANGED)

#include "Event.h"

namespace Webcore {
class TVRecordingChangedEvent:public Event {
	Ref<TVRecordingChangedEvent> TVRecordingChangedEvent::create(const AtomicString& type,const String& id,const String& errorName,bool isTimeChange,bool isStateChange)
	const String& id() const {return m_id;}
	const String& errorName() const {return m_errorName;}
	bool isTimeChange() const {return m_isTimeChange;}
        bool isStateChange() const {return m_isStateChange;}
	virtual EventInterface eventInterface() const { return TVRecordingChangedEventInterfaceType; }
private:
	TVRecordingChangedEvent(const AtomicString& type,const String& id,const String& errorName,bool isTimeChange,bool isStateChange);
	String m_id;
	String m_errorName;
	bool m_isTimeChange;
	bool m_isStateChange;

};
}


#endif

#endif
