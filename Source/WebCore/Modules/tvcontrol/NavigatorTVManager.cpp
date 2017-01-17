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
#include "NavigatorTVManager.h"

#if ENABLE(TV_CONTROL)

#include "Document.h"
#include "Frame.h"
#include "Navigator.h"
#include "TVManager.h"

namespace WebCore {

NavigatorTVManager::NavigatorTVManager(Frame* frame)
    : DOMWindowProperty(frame)
{
}

NavigatorTVManager::~NavigatorTVManager()
{
}

const char* NavigatorTVManager::supplementName()
{
    return "NavigatorTVManager";
}

NavigatorTVManager* NavigatorTVManager::from(Navigator* navigator)
{
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    NavigatorTVManager* supplement = static_cast<NavigatorTVManager*>(Supplement<Navigator>::from(navigator, supplementName()));
    if (!supplement) {
        auto newSupplement = std::make_unique<NavigatorTVManager>(navigator->frame());
        supplement = newSupplement.get();
        provideTo(navigator, supplementName(), WTFMove(newSupplement));
    }
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return supplement;
}

TVManager* NavigatorTVManager::tv(Navigator& navigator)
{
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return NavigatorTVManager::from(&navigator)->tv();
}

TVManager* NavigatorTVManager::tv() const
{
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (!m_tvManager && frame()) {
        printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        m_tvManager = TVManager::create(frame()->document());
    }
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return m_tvManager.get();
}

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
