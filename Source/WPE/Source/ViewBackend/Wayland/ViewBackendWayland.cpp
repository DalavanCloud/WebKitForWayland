/*
 * Copyright (C) 2015 Igalia S.L.
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

#include "Config.h"
#include "ViewBackendWayland.h"

#if WPE_BACKEND(WAYLAND)

#if WPE_BUFFER_MANAGEMENT(GBM)
#include "BufferDataGBM.h"
#endif
#include "WaylandDisplay.h"
#include "ivi-application-client-protocol.h"
#include "xdg-shell-client-protocol.h"
#include "wayland-drm-client-protocol.h"
#include "nsc-client-protocol.h"
#include <WPE/Pasteboard/Pasteboard.h>
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <glib.h>
#include <unistd.h>
#include <string.h>

#include <refsw/nexus_config.h>
#include <refsw/nexus_platform.h>
#include <refsw/nexus_display.h>
#include <refsw/nexus_core_utils.h>
#include <refsw/default_nexus.h>
#include <refsw/nxclient.h>

namespace WPE {

namespace ViewBackend {

static const struct xdg_surface_listener g_xdgSurfaceListener = {
    // configure
    [](void*, struct xdg_surface* surface, int32_t, int32_t, struct wl_array*, uint32_t serial)
    {
        xdg_surface_ack_configure(surface, serial);
    },
    // delete
    [](void*, struct xdg_surface*) { },
};

static const struct ivi_surface_listener g_iviSurfaceListener = {
    // configure
    [](void* data, struct ivi_surface*, int32_t width, int32_t height)
    {
        auto& resizingData = *static_cast<ViewBackendWayland::ResizingData*>(data);
        if (resizingData.client)
            resizingData.client->setSize(std::max(0, width), std::max(0, height));
    },
};

const struct wl_buffer_listener g_bufferListener = {
    // release
    [](void* data, struct wl_buffer* buffer)
    {
        auto& bufferData = *static_cast<ViewBackendWayland::BufferListenerData*>(data);
        auto& bufferMap = bufferData.map;

        auto it = std::find_if(bufferMap.begin(), bufferMap.end(),
            [buffer] (const std::pair<uint32_t, struct wl_buffer*>& entry) { return entry.second == buffer; });

        if (it == bufferMap.end())
            return;

        if (bufferData.client)
            bufferData.client->releaseBuffer(it->first);
    },
};

const struct wl_callback_listener g_callbackListener = {
    // frame
    [](void* data, struct wl_callback* callback, uint32_t)
    {
        auto& callbackData = *static_cast<ViewBackendWayland::CallbackListenerData*>(data);
        if (callbackData.client)
            callbackData.client->frameComplete();
        callbackData.frameCallback = nullptr;
        wl_callback_destroy(callback);
    },
};

static const struct wl_nsc_listener nsc_listener = {
    // handle_standby_status
    [](void*, struct wl_nsc*, struct wl_array*) { },
    // connectID
    [](void*, struct wl_nsc*, uint32_t) { },
    // composition
    [](void*, struct wl_nsc*, struct wl_array*) { },
    // authenticated
    [](void*, struct wl_nsc*, const char* certData, uint32_t certLength)
    {
        NEXUS_Certificate certificate;
        memcpy(certificate.data, certData, certLength);
        certificate.length = certLength;

        NEXUS_ClientAuthenticationSettings authSettings;
        NEXUS_Platform_GetDefaultClientAuthenticationSettings(&authSettings);
        authSettings.certificate = certificate;

        if(NEXUS_Platform_AuthenticatedJoin( &authSettings))
            printf("### Failed to join platform\n");
        else
            printf("### authenticated\n");
    },
    // clientID_created
    [](void* data, struct wl_nsc*, struct wl_array* clientIDArray)
    {
        auto& nscData = *static_cast<ViewBackendWayland::NscData*>(data);
        auto pClientID = static_cast<uint*>(clientIDArray->data);

        nscData.clientID = pClientID[0];
        printf("client received is %d\n", nscData.clientID);
    },
    // display_geometry
    [](void* data, struct wl_nsc*, uint32_t width, uint32_t height)
    {
        printf("display geometry %dx%d\n", width, height);
        auto& nscData = *static_cast<ViewBackendWayland::NscData*>(data);
        nscData.width = width;
        nscData.height = height;
    },
    // audiosettings
    [](void*, struct wl_nsc*, struct wl_array*) { },
    // displaystatus
    [](void*, struct wl_nsc*, struct wl_array*) { },
    // picturequalitysettings
    [](void*, struct wl_nsc*, struct wl_array*) { },
    // displaysettings
    [](void*, struct wl_nsc*, struct wl_array*) { },
};

ViewBackendWayland::ViewBackendWayland()
    : m_display(WaylandDisplay::singleton())
{
    m_surface = wl_compositor_create_surface(m_display.interfaces().compositor);

    if (m_display.interfaces().xdg) {
        m_xdgSurface = xdg_shell_get_xdg_surface(m_display.interfaces().xdg, m_surface);
        xdg_surface_add_listener(m_xdgSurface, &g_xdgSurfaceListener, nullptr);
        xdg_surface_set_title(m_xdgSurface, "WPE");
    }

    if (m_display.interfaces().ivi_application) {
        m_iviSurface = ivi_application_surface_create(m_display.interfaces().ivi_application,
            4200 + getpid(), // a unique identifier
            m_surface);
        ivi_surface_add_listener(m_iviSurface, &g_iviSurfaceListener, &m_resizingData);
    }

    if (m_display.interfaces().nsc) {
        m_nscData.display = m_display.display();
        m_nscData.nsc = m_display.interfaces().nsc;
        wl_nsc_add_listener(m_nscData.nsc, &nsc_listener, &m_nscData);
        wl_nsc_request_clientID(m_nscData.nsc, WL_NSC_CLIENT_SURFACE);
        wl_display_roundtrip(m_nscData.display);
        wl_nsc_authenticate(m_nscData.nsc);
        wl_display_roundtrip(m_nscData.display);
        wl_nsc_get_display_geometry(m_nscData.nsc);
        wl_display_roundtrip(m_nscData.display);
    }

    m_bufferFactory = Graphics::BufferFactory::create();

    // Ensure the Pasteboard singleton is constructed early.
    Pasteboard::Pasteboard::singleton();
}

ViewBackendWayland::~ViewBackendWayland()
{
    m_display.unregisterInputClient(m_surface);

    m_bufferData = { nullptr, decltype(m_bufferData.map){ } };

    if (m_callbackData.frameCallback)
        wl_callback_destroy(m_callbackData.frameCallback);
    m_callbackData = { nullptr, nullptr };

    m_resizingData = { nullptr, 0, 0 };

    if (m_iviSurface)
        ivi_surface_destroy(m_iviSurface);
    m_iviSurface = nullptr;
    if (m_xdgSurface)
        xdg_surface_destroy(m_xdgSurface);
    m_xdgSurface = nullptr;
    if (m_surface)
        wl_surface_destroy(m_surface);
    m_surface = nullptr;
}

void ViewBackendWayland::setClient(Client* client)
{
    m_bufferData.client = client;
    m_callbackData.client = client;
    m_resizingData.client = client;
    m_nscData.client = client;
    m_nscData.client->setSize(m_nscData.width, m_nscData.height);
}

std::pair<const uint8_t*, size_t> ViewBackendWayland::authenticate()
{
    return m_bufferFactory->authenticate();
}

uint32_t ViewBackendWayland::constructRenderingTarget(uint32_t width, uint32_t height)
{
    return m_bufferFactory->constructRenderingTarget(width, height);
}

void ViewBackendWayland::commitBuffer(int fd, const uint8_t* data, size_t size)
{
    std::pair<bool, std::pair<uint32_t, struct wl_buffer*>> result = m_bufferFactory->createBuffer(fd, data, size);
    if (!result.first) {
        m_nscData.client->frameComplete();
        return;
    }

    auto buffer = result.second;

    auto& bufferMap = m_bufferData.map;
    auto it = bufferMap.find(buffer.first);

    if (buffer.second) {
        wl_buffer_add_listener(buffer.second, &g_bufferListener, &m_bufferData);

        if (it != bufferMap.end()) {
            wl_buffer_destroy(it->second);
            it->second = buffer.second;
        } else
            bufferMap.insert(buffer);
    } else {
        assert(it != bufferMap.end());
        buffer.second = it->second;
    }

    if (!buffer.second) {
        fprintf(stderr, "ViewBackendWayland: failed to process the committed buffer\n");
        return;
    }

    m_callbackData.frameCallback = wl_surface_frame(m_surface);
    wl_callback_add_listener(m_callbackData.frameCallback, &g_callbackListener, &m_callbackData);

    wl_surface_attach(m_surface, buffer.second, 0, 0);
    wl_surface_damage(m_surface, 0, 0, INT32_MAX, INT32_MAX);
    wl_surface_commit(m_surface);
    wl_display_flush(m_display.display());
}

void ViewBackendWayland::destroyBuffer(uint32_t handle)
{
    auto& bufferMap = m_bufferData.map;
    auto it = bufferMap.find(handle);
    assert(it != bufferMap.end());

    m_bufferFactory->destroyBuffer(handle, it->second);

    wl_buffer_destroy(it->second);
    bufferMap.erase(it);
}

void ViewBackendWayland::setInputClient(Input::Client* client)
{
    m_display.registerInputClient(m_surface, client);
}

} // namespace ViewBackend

} // namespace WPE

#endif // WPE_BACKEND(WAYLAND)
