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
#include "TVManager.h"

#if ENABLE(TV_CONTROL)

#include "TVTunerChangedEvent.h"

#include "Document.h"
#include "Frame.h"
#include "Navigator.h"
#include "EventNames.h"

namespace WebCore {

Ref<TVManager> TVManager::create(ScriptExecutionContext* context)
{
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    Ref<TVManager> tvManager(adoptRef(*new TVManager(context)));
    tvManager->suspendIfNeeded();
    return tvManager;
}

TVManager::TVManager(ScriptExecutionContext* context)
    : ActiveDOMObject(context)
    , m_platformTVManager(nullptr)
{
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
}

TVManager::~TVManager()
{
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (m_tunerList.size())
        m_tunerList.clear();
}

Document* TVManager::document() const
{
    return downcast<Document>(scriptExecutionContext());
}

void TVManager::didTunerOperationChanged(String tunerId, uint16_t event)
{
    int position;
    if ((TVTunerChangedEvent::Operation)event == TVTunerChangedEvent::Operation::Added) { // Case when DVB adapter is added.
        m_tunerList.append(TVTuner::create(scriptExecutionContext(), PlatformTVTuner::create(tunerId.utf8().data(), m_platformTVManager->m_tvBackend)));
        printf("Found and Added the Tuner");
    } else { // Case when DVB Adapter is closed.
        position = 0;
        //Iterate  private tuner list and get the particular tuner info
        for (auto& element : m_tunerList) {
            printf("Id of this tuner %s\n", (element->id()).utf8().data());
            if (strncmp((element->id()).utf8().data(), tunerId.utf8().data(), 3) == 0) {
                m_tunerList.remove(position);
                printf("Found and Deleted the Tuner");
                break;
            }
            position++;
        }
    }
    scriptExecutionContext()->postTask([=](ScriptExecutionContext&) {
        dispatchEvent(TVTunerChangedEvent::create(eventNames().tunerchangedEvent, tunerId, (TVTunerChangedEvent::Operation)event));
    });
}

void TVManager::didCurrentSourceChanged(String tunerId)
{
    printf("\n%s:%s:%d\n TUNER ID = %s", __FILE__, __func__, __LINE__, tunerId.utf8().data());

    /*Identify tuner */
    for (auto& tuner : m_tunerList) {
        if (equalIgnoringASCIICase(tunerId, tuner->id()) == 1) {
            tuner->dispatchSourceChangedEvent();
            break;
        }
    }
}

void TVManager::didCurrentChannelChanged(String tunerId)
{

    printf("\n%s:%s:%d\n TUNER ID = %s", __FILE__, __func__, __LINE__, tunerId.utf8().data());
    for (auto& tuner : m_tunerList) {
        if (equalIgnoringASCIICase(tunerId, tuner->id()) == 1) {
            printf("\n%s:%s:%d\n TUNER ID = %s", __FILE__, __func__, __LINE__, tunerId.utf8().data());
            tuner->currentSource()->dispatchChannelChangedEvent();
            break;
        }
    }

    //Implement logic to identify corresponding tuner instance, source instance and channel instance
    //Create event using idenified instance details
}

void TVManager::didScanningStateChanged(String tunerId, RefPtr<PlatformTVChannel> platformTVChannel, uint16_t state)
{

    for (auto& tuner : m_tunerList) {
        if (equalIgnoringASCIICase(tunerId, tuner->id()) == 1) {
            printf("Scanning state  dispatched from Mgr = ");
            printf("%" PRIu16 "\n", state);

            tuner->currentSource()->dispatchScanningStateChangedEvent(platformTVChannel, state);
            break;
        }
    }
}

void TVManager::getTuners(TVTunerPromise&& promise)
{
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (m_tunerList.size()) {
        promise.resolve(m_tunerList);
        return;
    }
    if (!m_platformTVManager)
        m_platformTVManager = std::make_unique<PlatformTVManager>(this);

    if (m_platformTVManager) {
        Vector<RefPtr<PlatformTVTuner>> platformTunerList;
        if (!m_platformTVManager->getTuners(platformTunerList)) {
            promise.reject(nullptr);
            return;
        }
        if (platformTunerList.size()) {
            printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
            for (auto& tuner : platformTunerList) {
                printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
                m_tunerList.append(TVTuner::create(document(), tuner));
            }
            platformTunerList.clear();
        }
        if (m_tunerList.size()) {
            promise.resolve(m_tunerList);
            return;
        }
    }
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    promise.reject(nullptr);
}

ScriptExecutionContext* TVManager::scriptExecutionContext() const
{
    return ActiveDOMObject::scriptExecutionContext();
}

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
