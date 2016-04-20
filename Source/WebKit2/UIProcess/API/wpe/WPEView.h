/*
 * Copyright (C) 2014 Igalia S.L.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WPEView_h
#define WPEView_h

#include "APIObject.h"
#include "PageClientImpl.h"
#include "WebPageProxy.h"
#include "WPEViewClient.h"
#include <WPE/Input/Handling.h>
#include <WPE/ViewBackend/ViewBackend.h>
#include <WebCore/ViewState.h>
#include <memory>
#include <wtf/RefPtr.h>

typedef struct WKViewClientBase WKViewClientBase;

namespace WebKit {
class CompositingManagerProxy;
class WebPageGroup;
class WebProcessPool;
}

namespace WKWPE {

class View : public API::ObjectImpl<API::Object::Type::View>, public WPE::Input::Client {
public:
    static View* create(const API::PageConfiguration& configuration)
    {
        return new View(configuration);
    }

    // Client methods
    void initializeClient(const WKViewClientBase*);
    void frameDisplayed();

    WebKit::WebPageProxy& page() { return *m_pageProxy; }

    WPE::ViewBackend::ViewBackend& viewBackend() { return *m_viewBackend; }

    const WebCore::IntSize& size() const { return m_size; }
    void setSize(const WebCore::IntSize& size);

    WebCore::ViewState::Flags viewState() const { return m_viewStateFlags; }
    void setViewState(WebCore::ViewState::Flags);

    // WPE::Input::Client
    void handleKeyboardEvent(WPE::Input::KeyboardEvent&&) override;
    void handlePointerEvent(WPE::Input::PointerEvent&&) override;
    void handleAxisEvent(WPE::Input::AxisEvent&&) override;
    void handleTouchEvent(WPE::Input::TouchEvent&&) override;

private:
    View(const API::PageConfiguration&);
    virtual ~View();

    ViewClient m_client;

    std::unique_ptr<WebKit::PageClientImpl> m_pageClient;
    RefPtr<WebKit::WebPageProxy> m_pageProxy;
    std::unique_ptr<WPE::ViewBackend::ViewBackend> m_viewBackend;
    std::unique_ptr<WebKit::CompositingManagerProxy> m_compositingManagerProxy;

    WebCore::IntSize m_size;
    WebCore::ViewState::Flags m_viewStateFlags;
};

} // namespace WKWPE

#endif // WPEView_h
