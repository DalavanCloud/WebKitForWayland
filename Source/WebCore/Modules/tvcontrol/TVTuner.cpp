#include "config.h"
#include "TVTuner.h"

#if ENABLE(TV_CONTROL)

namespace WebCore {

Ref<TVTuner> TVTuner::create (RefPtr<PlatformTVTuner> platformTVTuner) {
    return adoptRef(*new TVTuner (platformTVTuner));
}

TVTuner::TVTuner (RefPtr<PlatformTVTuner> platformTVTuner)
    : m_platformTVTuner(platformTVTuner) {
}

const Vector<TVTuner::SourceType>&  TVTuner::getSupportedSourceTypes () {
    //Use platform logic to get supported Source Types and map.
    if (m_sourceTypeList.size())
        return m_sourceTypeList;

    if (m_platformTVTuner) {
        Vector<PlatformTVSource::Type> platformSourceTypeList;
        if (!m_platformTVTuner->getSupportedSourceTypes(platformSourceTypeList)) {
            return m_sourceTypeList;
        }
        if (platformSourceTypeList.size()) {
            printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
            for (auto& type : platformSourceTypeList) {
                printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
                m_sourceTypeList.append((SourceType)(type));
            }
        }
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
        Vector<RefPtr<PlatformTVSource>> platformSourceList;
        if (!m_platformTVTuner->getSources(platformSourceList)) {
            promise.reject(nullptr);
            return;
        }
        if (platformSourceList.size()) {
            for (auto& source : platformSourceList) {
                m_sourceList.append(TVSource::create(source, this));
            }
            promise.resolve(m_sourceList);
            return;
        }
    }
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    promise.reject(nullptr);
}

void  TVTuner::setCurrentSource (TVTuner::SourceType sourceType, TVPromise&& promise) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (m_platformTVTuner) {
        m_currentSource = nullptr;
        if (m_sourceList.isEmpty()) {
            /* Get the list first */
            Vector<RefPtr<PlatformTVSource>> platformSourceList;
            if (!m_platformTVTuner->getSources(platformSourceList)) {
                promise.reject(nullptr);
                return;
            }
            for (auto& source : platformSourceList)
                m_sourceList.append(TVSource::create(source, this));
        } 
        /* Parse the source list and set current source */
        for (auto& src : m_sourceList) {
            if ((SourceType)src->type()  == sourceType ) {
                m_currentSource = src;
                if (m_platformTVTuner->setCurrentSource((PlatformTVSource::Type)sourceType)) {
                    promise.resolve(nullptr);
                    return;
                }
            }
        }
        printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    }
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    promise.reject(nullptr);
}

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
