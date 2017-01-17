/*
 * Copyright (C) 2017 TATA ELXSI
 * Copyright (C) 2017 Metrological
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "TVTuner.h"

#if ENABLE(TV_CONTROL)

namespace WebCore {

Ref<TVTuner> TVTuner::create(ScriptExecutionContext* context, RefPtr<PlatformTVTuner> platformTVTuner)
{
    return adoptRef(*new TVTuner(context, platformTVTuner));
}

TVTuner::TVTuner(ScriptExecutionContext* context, RefPtr<PlatformTVTuner> platformTVTuner)
    : ContextDestructionObserver(context)
    , m_platformTVTuner(platformTVTuner)
{
}

TVTuner::~TVTuner()
{
    if (m_sourceTypeList.size())
        m_sourceTypeList.clear();
    if (m_sourceList.size())
        m_sourceList.clear();
}

const Vector<TVTuner::SourceType>& TVTuner::getSupportedSourceTypes()
{
    // Use platform logic to get supported Source Types and map.
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

void TVTuner::getSources(TVSourcePromise&& promise)
{
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (m_sourceList.size()) {
        promise.resolve(m_sourceList);
        return;
    }
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (m_platformTVTuner) {
        Vector<RefPtr<PlatformTVSource> > platformSourceList;
        if (!m_platformTVTuner->getSources(platformSourceList)) {
            promise.reject(nullptr);
            return;
        }
        if (platformSourceList.size()) {
            for (auto& source : platformSourceList) {
                m_sourceList.append(TVSource::create(scriptExecutionContext(), source, this));
            }
            promise.resolve(m_sourceList);
            return;
        }
    }
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    promise.reject(nullptr);
}

void TVTuner::setCurrentSource(TVTuner::SourceType sourceType, TVPromise&& promise)
{
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (m_platformTVTuner) {
        m_currentSource = nullptr;
        if (m_sourceList.isEmpty()) {
            /* Get the list first */
            Vector<RefPtr<PlatformTVSource> > platformSourceList;
            if (!m_platformTVTuner->getSources(platformSourceList)) {
                promise.reject(nullptr);
                return;
            }
            for (auto& source : platformSourceList)
                m_sourceList.append(TVSource::create(scriptExecutionContext(), source, this));
        }
        /* Parse the source list and set current source */
        for (auto& src : m_sourceList) {
            if ((SourceType)src->type() == sourceType) {
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

void TVTuner::dispatchSourceChangedEvent()
{

    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    scriptExecutionContext()->postTask([=](ScriptExecutionContext&) {
        dispatchEvent(TVCurrentSourceChangedEvent::create(eventNames().currentsourcechangedEvent, currentSource()));
    });
}

ScriptExecutionContext* TVTuner::scriptExecutionContext() const
{
    return ContextDestructionObserver::scriptExecutionContext();
}

void TVTuner::contextDestroyed()
{
    ContextDestructionObserver::contextDestroyed();
}

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
