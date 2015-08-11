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

#include <Athol/Interfaces.h>
#include <DerivedSources/WebCore/WaylandWPEProtocolServer.h>
#include <DIAL/Server.h>
#include <WebKit/WKRetainPtr.h>
#include <WebKit/WKView.h>
#include <glib.h>
#include <wayland-server.h>

namespace WPE {

class InputClient;
class View;

class AtholShell : public DIAL::Server::Client {
public:
    AtholShell(API::Compositor*);
    static gpointer launchWPE(gpointer);

private:
    friend class InputClient;

    static const struct wl_wpe_interface m_wpeInterface;

    API::Compositor* m_compositor;

    uint32_t width() { return m_compositor->width(); }
    uint32_t height() { return m_compositor->height(); }

    WKRetainPtr<WKViewRef> m_view;

    // DIAL::Server::Client
    virtual void startApp(unsigned, const char*) override;
    virtual void stopApp(unsigned) override;

    std::unique_ptr<DIAL::Server> m_dialServer;
    GMainContext* m_threadContext;
    GSource* m_DIALAppSource { nullptr };
    char* m_DIALAppURL;
    void scheduleDIALAppLoad();
    static gboolean loadDIALApp(gpointer);
};

} // namespace WPE
