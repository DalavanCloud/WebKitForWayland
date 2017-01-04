/*
 * Copyright (C) 2015, 2016 Igalia S.L.
 * Copyright (C) 2015, 2016 Metrological
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

#include <wpe/renderer-host.h>

#include "renderer-host.h"
#include <cstdio>
#include <cstdlib>

namespace Westeros {

RendererHost::RendererHost()
{
    m_compositor = WstCompositorCreate();
    if (!m_compositor)
        return;

    const char* nestedTargetDisplay = std::getenv("WAYLAND_DISPLAY");
    if (nestedTargetDisplay) {
        fprintf(stderr, "RendererHostWesteros: running as the nested compositor\n");
        WstCompositorSetIsNested(m_compositor, true);
        WstCompositorSetIsRepeater(m_compositor, true);
        WstCompositorSetNestedDisplayName(m_compositor, nestedTargetDisplay);
    }

    if (!WstCompositorStart(m_compositor))
    {
        fprintf(stderr, "RendererHostWesteros: failed to start the compositor: %s\n",
            WstCompositorGetLastErrorDetail(m_compositor));
        WstCompositorDestroy(m_compositor);
        m_compositor = nullptr;
    }
}

RendererHost::~RendererHost()
{
    if (m_compositor) {
        WstCompositorStop(m_compositor);
        WstCompositorDestroy(m_compositor);
        m_compositor = nullptr;
    }
}

RendererHost& RendererHost::singleton()
{
    static RendererHost s_host;
    return s_host;
}

int RendererHost::createClient()
{
    if (!m_compositor)
        return -1;

    int clientFd = -1;
    bool result = WstCompositorCreateClient(m_compositor, &clientFd);
    if (!result)
        return -1;

    return clientFd;
}

} // namespace Westeros

extern "C" {

struct wpe_renderer_host_interface westeros_renderer_host_interface = {
    // create
    []() -> void*
    {
        return nullptr;
    },
    // destroy
    [](void* data)
    {
    },
    // create_client
    [](void* data) -> int
    {
        int fd = Westeros::RendererHost::singleton().createClient();
        return fd;
    }
};

}
