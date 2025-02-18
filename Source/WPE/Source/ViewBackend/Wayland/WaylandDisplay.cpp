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
#include "WaylandDisplay.h"

#if WPE_BACKEND(WAYLAND)

#include "ivi-application-client-protocol.h"
#include "xdg-shell-client-protocol.h"
#include "wayland-drm-client-protocol.h"
#include "nsc-client-protocol.h"
#include <WPE/Input/Handling.h>
#include <cassert>
#include <cstring>
#include <glib.h>
#include <linux/input.h>
#include <locale.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client.h>

namespace WPE {

namespace ViewBackend {

class EventSource {
public:
    static GSourceFuncs sourceFuncs;

    GSource source;
    GPollFD pfd;
    struct wl_display* display;
};

GSourceFuncs EventSource::sourceFuncs = {
    // prepare
    [](GSource* base, gint* timeout) -> gboolean
    {
        auto* source = reinterpret_cast<EventSource*>(base);
        struct wl_display* display = source->display;

        *timeout = -1;

        wl_display_flush(display);
        wl_display_dispatch_pending(display);

        return FALSE;
    },
    // check
    [](GSource* base) -> gboolean
    {
        auto* source = reinterpret_cast<EventSource*>(base);
        return !!source->pfd.revents;
    },
    // dispatch
    [](GSource* base, GSourceFunc, gpointer) -> gboolean
    {
        auto* source = reinterpret_cast<EventSource*>(base);
        struct wl_display* display = source->display;

        if (source->pfd.revents & G_IO_IN)
            wl_display_dispatch(display);

        if (source->pfd.revents & (G_IO_ERR | G_IO_HUP))
            return FALSE;

        source->pfd.revents = 0;
        return TRUE;
    },
    nullptr, // finalize
    nullptr, // closure_callback
    nullptr, // closure_marshall
};

const struct wl_registry_listener g_registryListener = {
    // global
    [](void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version)
    {
        auto& interfaces = *static_cast<WaylandDisplay::Interfaces*>(data);

        if (!std::strcmp(interface, "wl_compositor"))
            interfaces.compositor = static_cast<struct wl_compositor*>(wl_registry_bind(registry, name, &wl_compositor_interface, 1));

        if (!std::strcmp(interface, "wl_data_device_manager"))
            interfaces.data_device_manager = static_cast<struct wl_data_device_manager*>(wl_registry_bind(registry, name, &wl_data_device_manager_interface, 2));

        if (!std::strcmp(interface, "wl_drm"))
            interfaces.drm = static_cast<struct wl_drm*>(wl_registry_bind(registry, name, &wl_drm_interface, 2));

        if (!std::strcmp(interface, "wl_seat"))
            interfaces.seat = static_cast<struct wl_seat*>(wl_registry_bind(registry, name, &wl_seat_interface, 4));

        if (!std::strcmp(interface, "xdg_shell"))
            interfaces.xdg = static_cast<struct xdg_shell*>(wl_registry_bind(registry, name, &xdg_shell_interface, 1)); 

        if (!std::strcmp(interface, "ivi_application"))
            interfaces.ivi_application = static_cast<struct ivi_application*>(wl_registry_bind(registry, name, &ivi_application_interface, 1));

        if (!std::strcmp(interface, "wl_nsc"))
            interfaces.nsc = static_cast<struct wl_nsc*>(wl_registry_bind(registry, name, &wl_nsc_interface, version));
    },
    // global_remove
    [](void*, struct wl_registry*, uint32_t) { },
};

static const struct xdg_shell_listener g_xdgShellListener = {
    // ping
    [](void*, struct xdg_shell* shell, uint32_t serial)
    {
        xdg_shell_pong(shell, serial);
    },
};

static const struct wl_pointer_listener g_pointerListener = {
    // enter
    [](void* data, struct wl_pointer*, uint32_t serial, struct wl_surface* surface, wl_fixed_t, wl_fixed_t)
    {
        auto& seatData = *static_cast<WaylandDisplay::SeatData*>(data);
        seatData.serial = serial;
        auto it = seatData.inputClients.find(surface);
        if (it != seatData.inputClients.end())
            seatData.pointer.target = *it;
    },
    // leave
    [](void* data, struct wl_pointer*, uint32_t serial, struct wl_surface* surface)
    {
        auto& seatData = *static_cast<WaylandDisplay::SeatData*>(data);
        seatData.serial = serial;
        auto it = seatData.inputClients.find(surface);
        if (it != seatData.inputClients.end() && seatData.pointer.target.first == it->first)
            seatData.pointer.target = { };
    },
    // motion
    [](void* data, struct wl_pointer*, uint32_t time, wl_fixed_t fixedX, wl_fixed_t fixedY)
    {
        auto x = wl_fixed_to_int(fixedX);
        auto y = wl_fixed_to_int(fixedY);

        auto& pointer = static_cast<WaylandDisplay::SeatData*>(data)->pointer;
        pointer.coords = { x, y };
        if (pointer.target.first)
            pointer.target.second->handlePointerEvent({ Input::PointerEvent::Motion, time, x, y, 0, 0 });
    },
    // button
    [](void* data, struct wl_pointer*, uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
    {
        static_cast<WaylandDisplay::SeatData*>(data)->serial = serial;

        if (button >= BTN_MOUSE)
            button = button - BTN_MOUSE + 1;
        else
            button = 0;

        auto& pointer = static_cast<WaylandDisplay::SeatData*>(data)->pointer;
        auto& coords = pointer.coords;
        if (pointer.target.first)
            pointer.target.second->handlePointerEvent(
                { Input::PointerEvent::Button, time, coords.first, coords.second, button, state });
    },
    // axis
    [](void* data, struct wl_pointer*, uint32_t time, uint32_t axis, wl_fixed_t value)
    {
        auto& pointer = static_cast<WaylandDisplay::SeatData*>(data)->pointer;
        auto& coords = pointer.coords;
        if (pointer.target.first)
            pointer.target.second->handleAxisEvent(
                { Input::AxisEvent::Motion, time, coords.first, coords.second, axis, -wl_fixed_to_int(value) });
    },
};

static void
handleKeyEvent(WaylandDisplay::SeatData& seatData, uint32_t key, uint32_t state, uint32_t time)
{
    auto& xkb = seatData.xkb;
    uint32_t keysym = xkb_state_key_get_one_sym(xkb.state, key);
    uint32_t unicode = xkb_state_key_get_utf32(xkb.state, key);

    if (xkb.composeState
        && state == WL_KEYBOARD_KEY_STATE_PRESSED
        && xkb_compose_state_feed(xkb.composeState, keysym) == XKB_COMPOSE_FEED_ACCEPTED
        && xkb_compose_state_get_status(xkb.composeState) == XKB_COMPOSE_COMPOSED)
    {
        keysym = xkb_compose_state_get_one_sym(xkb.composeState);
        unicode = xkb_keysym_to_utf32(keysym);
    }

    if (seatData.keyboard.target.first)
        seatData.keyboard.target.second->handleKeyboardEvent({ time, keysym, unicode, !!state, xkb.modifiers });
}

static gboolean
repeatRateTimeout(void* data)
{
    auto& seatData = *static_cast<WaylandDisplay::SeatData*>(data);
    handleKeyEvent(seatData, seatData.repeatData.key, seatData.repeatData.state, seatData.repeatData.time);
    return G_SOURCE_CONTINUE;
}

static gboolean
repeatDelayTimeout(void* data)
{
    auto& seatData = *static_cast<WaylandDisplay::SeatData*>(data);
    handleKeyEvent(seatData, seatData.repeatData.key, seatData.repeatData.state, seatData.repeatData.time);
    seatData.repeatData.eventSource = g_timeout_add(seatData.repeatInfo.rate, static_cast<GSourceFunc>(repeatRateTimeout), data);
    return G_SOURCE_REMOVE;
}

static const struct wl_keyboard_listener g_keyboardListener = {
    // keymap
    [](void* data, struct wl_keyboard*, uint32_t format, int fd, uint32_t size)
    {
        if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
            close(fd);
            return;
        }

        void* mapping = mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0);
        if (mapping == MAP_FAILED) {
            close(fd);
            return;
        }

        auto& xkb = static_cast<WaylandDisplay::SeatData*>(data)->xkb;
        xkb.keymap = xkb_keymap_new_from_string(xkb.context, static_cast<char*>(mapping),
            XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
        munmap(mapping, size);
        close(fd);

        if (!xkb.keymap)
            return;

        xkb.state = xkb_state_new(xkb.keymap);
        if (!xkb.state)
            return;

        xkb.indexes.control = xkb_keymap_mod_get_index(xkb.keymap, XKB_MOD_NAME_CTRL);
        xkb.indexes.alt = xkb_keymap_mod_get_index(xkb.keymap, XKB_MOD_NAME_ALT);
        xkb.indexes.shift = xkb_keymap_mod_get_index(xkb.keymap, XKB_MOD_NAME_SHIFT);
    },
    // enter
    [](void* data, struct wl_keyboard*, uint32_t serial, struct wl_surface* surface, struct wl_array*)
    {
        auto& seatData = *static_cast<WaylandDisplay::SeatData*>(data);
        seatData.serial = serial;
        auto it = seatData.inputClients.find(surface);
        if (it != seatData.inputClients.end())
            seatData.keyboard.target = *it;
    },
    // leave
    [](void* data, struct wl_keyboard*, uint32_t serial, struct wl_surface* surface)
    {
        auto& seatData = *static_cast<WaylandDisplay::SeatData*>(data);
        seatData.serial = serial;
        auto it = seatData.inputClients.find(surface);
        if (it != seatData.inputClients.end() && seatData.keyboard.target.first == it->first)
            seatData.keyboard.target = { };
    },
    // key
    [](void* data, struct wl_keyboard*, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
    {
        // IDK.
        key += 8;

        auto& seatData = *static_cast<WaylandDisplay::SeatData*>(data);
        seatData.serial = serial;
        handleKeyEvent(seatData, key, state, time);

        if (!seatData.repeatInfo.rate)
            return;

        if (state == WL_KEYBOARD_KEY_STATE_RELEASED
            && seatData.repeatData.key == key) {
            if (seatData.repeatData.eventSource)
                g_source_remove(seatData.repeatData.eventSource);
            seatData.repeatData = { 0, 0, 0, 0 };
        } else if (state == WL_KEYBOARD_KEY_STATE_PRESSED
            && xkb_keymap_key_repeats(seatData.xkb.keymap, key)) {

            if (seatData.repeatData.eventSource)
                g_source_remove(seatData.repeatData.eventSource);

            seatData.repeatData = { key, time, state, g_timeout_add(seatData.repeatInfo.delay, static_cast<GSourceFunc>(repeatDelayTimeout), data) };
        }
    },
    // modifiers
    [](void* data, struct wl_keyboard*, uint32_t serial, uint32_t depressedMods, uint32_t latchedMods, uint32_t lockedMods, uint32_t group)
    {
        static_cast<WaylandDisplay::SeatData*>(data)->serial = serial;
        auto& xkb = static_cast<WaylandDisplay::SeatData*>(data)->xkb;
        xkb_state_update_mask(xkb.state, depressedMods, latchedMods, lockedMods, 0, 0, group);

        auto& modifiers = xkb.modifiers;
        modifiers = 0;
        auto component = static_cast<xkb_state_component>(XKB_STATE_MODS_DEPRESSED | XKB_STATE_MODS_LATCHED);
        if (xkb_state_mod_index_is_active(xkb.state, xkb.indexes.control, component))
            modifiers |= Input::KeyboardEvent::Control;
        if (xkb_state_mod_index_is_active(xkb.state, xkb.indexes.alt, component))
            modifiers |= Input::KeyboardEvent::Alt;
        if (xkb_state_mod_index_is_active(xkb.state, xkb.indexes.shift, component))
            modifiers |= Input::KeyboardEvent::Shift;
    },
    // repeat_info
    [](void* data, struct wl_keyboard*, int32_t rate, int32_t delay)
    {
        auto& repeatInfo = static_cast<WaylandDisplay::SeatData*>(data)->repeatInfo;
        repeatInfo = { rate, delay };

        // A rate of zero disables any repeating.
        if (!rate) {
            auto& repeatData = static_cast<WaylandDisplay::SeatData*>(data)->repeatData;
            if (repeatData.eventSource) {
                g_source_remove(repeatData.eventSource);
                repeatData = { 0, 0, 0, 0 };
            }
        }
    },
};

static const struct wl_touch_listener g_touchListener = {
    // down
    [](void* data, struct wl_touch*, uint32_t serial, uint32_t time, struct wl_surface* surface, int32_t id, wl_fixed_t x, wl_fixed_t y)
    {
        auto& seatData = *static_cast<WaylandDisplay::SeatData*>(data);
        seatData.serial = serial;

        int32_t arraySize = std::tuple_size<decltype(seatData.touch.targets)>::value;
        if (id < 0 || id >= arraySize)
            return;

        auto& target = seatData.touch.targets[id];
        assert(!target.first && !target.second);

        auto it = seatData.inputClients.find(surface);
        if (it == seatData.inputClients.end())
            return;

        target = { surface, it->second };

        auto& touchPoints = seatData.touch.touchPoints;
        touchPoints[id] = { Input::TouchEvent::Down, time, id, wl_fixed_to_int(x), wl_fixed_to_int(y) };
        target.second->handleTouchEvent({ touchPoints, Input::TouchEvent::Down, id, time });
    },
    // up
    [](void* data, struct wl_touch*, uint32_t serial, uint32_t time, int32_t id)
    {
        auto& seatData = *static_cast<WaylandDisplay::SeatData*>(data);
        seatData.serial = serial;

        int32_t arraySize = std::tuple_size<decltype(seatData.touch.targets)>::value;
        if (id < 0 || id >= arraySize)
            return;

        auto& target = seatData.touch.targets[id];
        assert(target.first && target.second);

        auto& touchPoints = seatData.touch.touchPoints;
        auto& point = touchPoints[id];
        point = { Input::TouchEvent::Up, time, id, point.x, point.y };
        target.second->handleTouchEvent({ touchPoints, Input::TouchEvent::Up, id, time });

        point = { Input::TouchEvent::Null, 0, 0, 0, 0 };
        target = { nullptr, nullptr };
    },
    // motion
    [](void* data, struct wl_touch*, uint32_t time, int32_t id, wl_fixed_t x, wl_fixed_t y)
    {
        auto& seatData = *static_cast<WaylandDisplay::SeatData*>(data);

        int32_t arraySize = std::tuple_size<decltype(seatData.touch.targets)>::value;
        if (id < 0 || id >= arraySize)
            return;

        auto& target = seatData.touch.targets[id];
        assert(target.first && target.second);

        auto& touchPoints = seatData.touch.touchPoints;
        touchPoints[id] = { Input::TouchEvent::Motion, time, id, wl_fixed_to_int(x), wl_fixed_to_int(y) };
        target.second->handleTouchEvent({ touchPoints, Input::TouchEvent::Motion, id, time });
    },
    // frame
    [](void*, struct wl_touch*)
    {
        // FIXME: Dispatching events via frame() would avoid dispatching events
        // for every single event that's encapsulated in a frame with multiple
        // other events.
    },
    // cancel
    [](void*, struct wl_touch*) { },
};

static const struct wl_seat_listener g_seatListener = {
    // capabilities
    [](void* data, struct wl_seat* seat, uint32_t capabilities)
    {
        auto& seatData = *static_cast<WaylandDisplay::SeatData*>(data);

        // WL_SEAT_CAPABILITY_POINTER
        const bool hasPointerCap = capabilities & WL_SEAT_CAPABILITY_POINTER;
        if (hasPointerCap && !seatData.pointer.object) {
            seatData.pointer.object = wl_seat_get_pointer(seat);
            wl_pointer_add_listener(seatData.pointer.object, &g_pointerListener, &seatData);
        }
        if (!hasPointerCap && seatData.pointer.object) {
            wl_pointer_destroy(seatData.pointer.object);
            seatData.pointer.object = nullptr;
        }

        // WL_SEAT_CAPABILITY_KEYBOARD
        const bool hasKeyboardCap = capabilities & WL_SEAT_CAPABILITY_KEYBOARD;
        if (hasKeyboardCap && !seatData.keyboard.object) {
            seatData.keyboard.object = wl_seat_get_keyboard(seat);
            wl_keyboard_add_listener(seatData.keyboard.object, &g_keyboardListener, &seatData);
        }
        if (!hasKeyboardCap && seatData.keyboard.object) {
            wl_keyboard_destroy(seatData.keyboard.object);
            seatData.keyboard.object = nullptr;
        }

        // WL_SEAT_CAPABILITY_TOUCH
        const bool hasTouchCap = capabilities & WL_SEAT_CAPABILITY_TOUCH;
        if (hasTouchCap && !seatData.touch.object) {
            seatData.touch.object = wl_seat_get_touch(seat);
            wl_touch_add_listener(seatData.touch.object, &g_touchListener, &seatData);
        }
        if (!hasTouchCap && seatData.touch.object) {
            wl_touch_destroy(seatData.touch.object);
            seatData.touch.object = nullptr;
        }
    },
    // name
    [](void*, struct wl_seat*, const char*) { }
};

WaylandDisplay& WaylandDisplay::singleton()
{
    static WaylandDisplay display;
    return display;
}

WaylandDisplay::WaylandDisplay()
{
    m_display = wl_display_connect(nullptr);
    m_registry = wl_display_get_registry(m_display);

    wl_registry_add_listener(m_registry, &g_registryListener, &m_interfaces);
    wl_display_roundtrip(m_display);

    m_eventSource = g_source_new(&EventSource::sourceFuncs, sizeof(EventSource));
    auto* source = reinterpret_cast<EventSource*>(m_eventSource);
    source->display = m_display;

    source->pfd.fd = wl_display_get_fd(m_display);
    source->pfd.events = G_IO_IN | G_IO_ERR | G_IO_HUP;
    source->pfd.revents = 0;
    g_source_add_poll(m_eventSource, &source->pfd);

    g_source_set_name(m_eventSource, "[WPE] WaylandDisplay");
    g_source_set_priority(m_eventSource, G_PRIORITY_HIGH + 30);
    g_source_set_can_recurse(m_eventSource, TRUE);
    g_source_attach(m_eventSource, g_main_context_get_thread_default());

    if (m_interfaces.xdg) {
        xdg_shell_add_listener(m_interfaces.xdg, &g_xdgShellListener, nullptr);
        xdg_shell_use_unstable_version(m_interfaces.xdg, 5);
    }

    wl_seat_add_listener(m_interfaces.seat, &g_seatListener, &m_seatData);

    m_seatData.xkb.context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    m_seatData.xkb.composeTable = xkb_compose_table_new_from_locale(m_seatData.xkb.context, setlocale(LC_CTYPE, nullptr), XKB_COMPOSE_COMPILE_NO_FLAGS);
    if (m_seatData.xkb.composeTable)
        m_seatData.xkb.composeState = xkb_compose_state_new(m_seatData.xkb.composeTable, XKB_COMPOSE_STATE_NO_FLAGS);
}

WaylandDisplay::~WaylandDisplay()
{
    if (m_eventSource)
        g_source_unref(m_eventSource);
    m_eventSource = nullptr;

    if (m_interfaces.compositor)
        wl_compositor_destroy(m_interfaces.compositor);
    if (m_interfaces.data_device_manager)
        wl_data_device_manager_destroy(m_interfaces.data_device_manager);
    if (m_interfaces.drm)
        wl_drm_destroy(m_interfaces.drm);
    if (m_interfaces.seat)
        wl_seat_destroy(m_interfaces.seat);
    if (m_interfaces.xdg)
        xdg_shell_destroy(m_interfaces.xdg);
    if (m_interfaces.ivi_application)
        ivi_application_destroy(m_interfaces.ivi_application);
    if (m_interfaces.nsc)
        wl_nsc_destroy(m_interfaces.nsc);
    m_interfaces = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

    if (m_registry)
        wl_registry_destroy(m_registry);
    m_registry = nullptr;
    if (m_display)
        wl_display_disconnect(m_display);
    m_display = nullptr;

    if (m_seatData.pointer.object)
        wl_pointer_destroy(m_seatData.pointer.object);
    if (m_seatData.keyboard.object)
        wl_keyboard_destroy(m_seatData.keyboard.object);
    if (m_seatData.touch.object)
        wl_touch_destroy(m_seatData.touch.object);
    if (m_seatData.xkb.context)
        xkb_context_unref(m_seatData.xkb.context);
    if (m_seatData.xkb.keymap)
        xkb_keymap_unref(m_seatData.xkb.keymap);
    if (m_seatData.xkb.state)
        xkb_state_unref(m_seatData.xkb.state);
    if (m_seatData.xkb.composeTable)
        xkb_compose_table_unref(m_seatData.xkb.composeTable);
    if (m_seatData.xkb.composeState)
        xkb_compose_state_unref(m_seatData.xkb.composeState);
    if (m_seatData.repeatData.eventSource)
        g_source_remove(m_seatData.repeatData.eventSource);
    m_seatData = SeatData{ };
}

void WaylandDisplay::registerInputClient(struct wl_surface* surface, Input::Client* client)
{
#ifndef NDEBUG
    auto result =
#endif
        m_seatData.inputClients.insert({ surface, client });
    assert(result.second);
}

void WaylandDisplay::unregisterInputClient(struct wl_surface* surface)
{
    auto it = m_seatData.inputClients.find(surface);
    assert(it != m_seatData.inputClients.end());

    if (m_seatData.pointer.target.first == it->first)
        m_seatData.pointer.target = { };
    if (m_seatData.keyboard.target.first == it->first)
        m_seatData.keyboard.target = { };
    m_seatData.inputClients.erase(it);
}

} // namespace ViewBackend

} // namespace WPE

#endif // WPE_BACKEND(WAYLAND)
