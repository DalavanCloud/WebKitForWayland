#include "config.h"
#include "TVTuner.h"

#if ENABLE(TV_CONTROL)

namespace WebCore {

Ref<TVTuner> TVTuner::create (ScriptExecutionContext* context, RefPtr<PlatformTVTuner> platformTVTuner) {
    return adoptRef(*new TVTuner (context, platformTVTuner));
}

TVTuner::TVTuner (ScriptExecutionContext* context, RefPtr<PlatformTVTuner> platformTVTuner)
    : ActiveDOMObject(context)
    , m_platformTVTuner(platformTVTuner) {
}

const Vector<TVTuner::SourceType>&  TVTuner::getSupportedSourceTypes () {
    //Use platform logic to get supported Source Types and map.
    if (m_sourceTypeList.size())
        return m_sourceTypeList;

    if (m_platformTVTuner) {
        for (auto& type : m_platformTVTuner->getSupportedSourceTypes())
            m_sourceTypeList.append((SourceType)(type)); // Add logic to convert this into TVSourceType
    }

    return m_sourceTypeList;
}

void  TVTuner::getSources (TVSourcePromise&& promise) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (m_sourceList.size()){
       promise.resolve(m_sourceList);
       return;
    }
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (m_platformTVTuner) {
        for (auto& source : m_platformTVTuner->getSources())
            m_sourceList.append(TVSource::create(scriptExecutionContext(), source, this));
        promise.resolve(m_sourceList);
    }
    else{
        printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        promise.reject(nullptr);
    }
}

void  TVTuner::dispatchSourceChangedEvent() {

    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    dispatchEvent(TVCurrentSourceChangedEvent::create(eventNames().currentsourcechangedEvent,
                                                      currentSource()));
}

void  TVTuner::setCurrentSource (TVTuner::SourceType sourceType, TVPromise&& promise) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (m_platformTVTuner) {
        m_currentSource = nullptr;
        if (m_sourceList.isEmpty()) {
            /* Get the list first */
            for (auto& source : m_platformTVTuner->getSources())
                m_sourceList.append(TVSource::create(scriptExecutionContext(), source, this));
        } 
        /* Parse the source list and set current source */
        for (auto& src : m_sourceList) {
            if ((SourceType)src->type()  == sourceType ) {
                m_currentSource = src;
                m_platformTVTuner->setCurrentSource((PlatformTVSource::Type)sourceType);
                promise.resolve(nullptr);
                return;
            }
        }
        printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    }
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    promise.reject(nullptr);
}

ScriptExecutionContext* TVTuner::scriptExecutionContext() const
{
    return ActiveDOMObject::scriptExecutionContext();
}


} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
