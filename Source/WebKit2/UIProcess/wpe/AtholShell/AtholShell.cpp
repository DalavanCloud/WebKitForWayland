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

#include "config.h"
#include "AtholShell.h"

#include <WPE/Input/Handling.h>
#include <WebKit/WKContextConfigurationRef.h>
#include <WebKit/WKContext.h>
#include <WebKit/WKFramePolicyListener.h>
#include <WebKit/WKPageGroup.h>
#include <WebKit/WKPage.h>
#include <WebKit/WKString.h>
#include <WebKit/WKURL.h>
#include <WebKit/WKCookieManagerSoup.h>
#include <cstdio>
#include <wtf/text/CString.h>
#include <wtf/text/WTFString.h>

namespace WPE {

class InputClient final : public API::InputClient {
public:
    InputClient(AtholShell&);

private:
    virtual void handleKeyboardEvent(uint32_t, uint32_t, uint32_t) override;
    virtual void handlePointerMotion(uint32_t, double, double) override;
    virtual void handlePointerButton(uint32_t, uint32_t, uint32_t) override;
    virtual void handlePointerAxis(uint32_t, API::InputClient::PointerAxis, double, double) override;

    AtholShell& m_shell;
    std::pair<uint32_t, uint32_t> m_pointer = { 0, 0 };
};

InputClient::InputClient(AtholShell& shell)
    : m_shell(shell)
{
}

void InputClient::handleKeyboardEvent(uint32_t time, uint32_t key, uint32_t state)
{
    WPE::Input::Server::singleton().serveKeyboardEvent({ time, key, state });
}

void InputClient::handlePointerMotion(uint32_t time, double dx, double dy)
{
    m_pointer = {
        std::max<uint32_t>(0, std::min<uint32_t>(m_shell.width() - 1, m_pointer.first + dx)),
        std::max<uint32_t>(0, std::min<uint32_t>(m_shell.height() - 1, m_pointer.second + dy))
    };

    WPE::Input::Server::singleton().servePointerEvent({
        WPE::Input::PointerEvent::Motion,
        time, static_cast<int>(m_pointer.first), static_cast<int>(m_pointer.second), 0, 0
    });
}

void InputClient::handlePointerButton(uint32_t time, uint32_t button, uint32_t state)
{
    WPE::Input::Server::singleton().servePointerEvent({
        WPE::Input::PointerEvent::Button,
        time, 0, 0, button, state
    });
}

void InputClient::handlePointerAxis(uint32_t time, API::InputClient::PointerAxis axis, double dx, double dy)
{
    UNUSED_PARAM(dy);

    int axisStep = 0;

    switch (axis) {
    case API::InputClient::PointerAxis::Wheel:
        if (!dx)
            return;
        axisStep = -2 * dx;
        break;
    default:
        return;
    }

    WPE::Input::Server::singleton().serveAxisEvent({
        WPE::Input::AxisEvent::Motion,
        time, static_cast<uint32_t>(axis), axisStep
    });
}

AtholShell::AtholShell(API::Compositor* compositor)
    : m_compositor(compositor)
{
    wl_global_create(m_compositor->display(),
        &wl_wpe_interface, wl_wpe_interface.version, this,
        [](struct wl_client* client, void* data, uint32_t version, uint32_t id) {
            ASSERT(version == wl_wpe_interface.version);
            auto* shell = static_cast<AtholShell*>(data);
            struct wl_resource* resource = wl_resource_create(client, &wl_wpe_interface, version, id);
            wl_resource_set_implementation(resource, &m_wpeInterface, shell, nullptr);
        });

    m_compositor->initializeInput(std::make_unique<InputClient>(*this));
}

const struct wl_wpe_interface AtholShell::m_wpeInterface = {
    // register_surface
    [](struct wl_client*, struct wl_resource* shellResource, struct wl_resource* surfaceResource) { }
};

// Taken from WebKitTestRunner.
static WTF::String getWTFString(WKStringRef string) {
    size_t bufferSize = WKStringGetMaximumUTF8CStringSize(string);
    auto buffer = std::make_unique<char[]>(bufferSize);
    size_t stringLength = WKStringGetUTF8CString(string, buffer.get(), bufferSize);
    return WTF::String::fromUTF8WithLatin1Fallback(buffer.get(), stringLength - 1);
}

gpointer AtholShell::launchWPE(gpointer data)
{
    auto& shell = *static_cast<AtholShell*>(data);

    shell.m_threadContext = g_main_context_new();
    GMainLoop* threadLoop = g_main_loop_new(shell.m_threadContext, FALSE);

    g_main_context_push_thread_default(shell.m_threadContext);

    shell.m_dialServer = std::make_unique<DIAL::Server>(shell);

    auto pageGroupIdentifier = adoptWK(WKStringCreateWithUTF8CString("WPEPageGroup"));
    auto pageGroup = adoptWK(WKPageGroupCreateWithIdentifier(pageGroupIdentifier.get()));

    auto context = adoptWK(WKContextCreate());

    if (!!g_getenv("WPE_SHELL_COOKIE_STORAGE")) {
        gchar *cookieDatabasePath = g_build_filename(g_get_user_cache_dir(), "cookies.db", nullptr);
        auto path = adoptWK(WKStringCreateWithUTF8CString(cookieDatabasePath));
        g_free(cookieDatabasePath);
        auto cookieManager = adoptWK(WKContextGetCookieManager(context.get()));
        WKCookieManagerSetCookiePersistentStorage(cookieManager.get(), path.get(), kWKCookieStorageTypeSQLite);
    }

    shell.m_view = adoptWK(WKViewCreate(context.get(), pageGroup.get()));
    auto* view = shell.m_view.get();
    WKViewResize(view, WKSizeMake(shell.width(), shell.height()));
    WKViewMakeWPEInputTarget(view);

    WKPageNavigationClientV0 pageNavigationClient = {
        { 0, nullptr },
        // decidePolicyForNavigationAction
        [](WKPageRef, WKNavigationActionRef, WKFramePolicyListenerRef listener, WKTypeRef, const void*) {
            WKFramePolicyListenerUse(listener);
        },
        nullptr, // decidePolicyForNavigationResponse
        nullptr, // decidePolicyForPluginLoad
        nullptr, // didStartProvisionalNavigation
        nullptr, // didReceiveServerRedirectForProvisionalNavigation
        nullptr, // didFailProvisionalNavigation
        nullptr, // didCommitNavigation
        nullptr, // didFinishNavigation
        nullptr, // didFailNavigation
        nullptr, // didFailProvisionalLoadInSubframe
        nullptr, // didFinishDocumentLoad
        nullptr, // didSameDocumentNavigation
        nullptr, // renderingProgressDidChange
        nullptr, // canAuthenticateAgainstProtectionSpace
        nullptr, // didReceiveAuthenticationChallenge
        // webProcessDidCrash
        [](WKPageRef page, const void*) {
            if (g_getenv("WPE_DISABLE_CRASH_RECOVERY"))
                return;
            auto url = adoptWK(WKPageCopyActiveURL(page));
            auto urlString = adoptWK(WKURLCopyString(url.get()));
            auto wtfString = getWTFString(urlString.get());

            std::fprintf(stderr, "[AtholShell] Crashed on '%s', reloading.\n", wtfString.utf8().data());
            WKPageReload(page);
        },
        nullptr, // copyWebCryptoMasterKey
    };
    WKPageSetPageNavigationClient(WKViewGetPage(view), &pageNavigationClient.base);

    const char* url = g_getenv("WPE_SHELL_URL");
    if (!url)
        url = "https://www.youtube.com/tv/";
    auto shellURL = adoptWK(WKURLCreateWithUTF8CString(url));
    WKPageLoadURL(WKViewGetPage(view), shellURL.get());

    g_main_loop_run(threadLoop);

    g_main_context_pop_thread_default(shell.m_threadContext);
    g_main_context_unref(shell.m_threadContext);
    return nullptr;
}

void AtholShell::startApp(unsigned, const char* appURL)
{
    m_DIALAppURL = g_strdup(appURL);
    scheduleDIALAppLoad();
}

void AtholShell::stopApp(unsigned)
{
    m_DIALAppURL = g_strdup("http://www.youtube.com/tv/");
    scheduleDIALAppLoad();
}

void AtholShell::scheduleDIALAppLoad()
{
    if (m_DIALAppSource)
        return;

    m_DIALAppSource = g_idle_source_new();
    g_source_set_callback(m_DIALAppSource, reinterpret_cast<GSourceFunc>(&AtholShell::loadDIALApp), this, nullptr);
    g_source_set_ready_time(m_DIALAppSource, 0);
    g_source_attach(m_DIALAppSource, m_threadContext);
}

gboolean AtholShell::loadDIALApp(gpointer data)
{
    auto& shell = *reinterpret_cast<AtholShell*>(data);
    g_source_unref(shell.m_DIALAppSource);
    shell.m_DIALAppSource = nullptr;

    if (!shell.m_DIALAppURL)
        return FALSE;

    std::fprintf(stderr, "[AtholShell] Loading '%s' through DIAL\n", shell.m_DIALAppURL);
    auto shellURL = adoptWK(WKURLCreateWithUTF8CString(shell.m_DIALAppURL));
    WKPageLoadURL(WKViewGetPage(shell.m_view.get()), shellURL.get());

    g_free(shell.m_DIALAppURL);
    shell.m_DIALAppURL = nullptr;

    return FALSE;
}

} // namespace WPE
