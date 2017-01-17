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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <wpe/tvcontrol-backend.h>

#include "loader-private.h"
#include "tvcontrol-backend-private.h"
#include <stdlib.h>

__attribute__((visibility("default")))
struct wpe_tvcontrol_backend*
wpe_tvcontrol_backend_create()
{
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    struct wpe_tvcontrol_backend* backend = malloc(sizeof(struct wpe_tvcontrol_backend));
    if (!backend)
        return 0;
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    backend->interface = wpe_load_object("_wpe_tvcontrol_backend_interface");
    backend->interface_data = backend->interface->create(backend);

    return backend;
}

__attribute__((visibility("default")))
void
wpe_tvcontrol_backend_destroy(struct wpe_tvcontrol_backend* backend)
{
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    backend->interface->destroy(backend->interface_data);
    backend->interface_data = 0;

    free(backend);
}

__attribute__((visibility("default")))
void
wpe_tvcontrol_backend_set_manager_event_client(struct wpe_tvcontrol_backend* backend, struct wpe_tvcontrol_backend_manager_event_client* client, void* client_data)
{
    backend->event_client = client;
    backend->event_client_data = client_data;
}

__attribute__((visibility("default")))
void
wpe_tvcontrol_backend_dispatch_tuner_event(struct wpe_tvcontrol_backend* backend, struct wpe_tvcontrol_event* event)
{
    if (backend->event_client)
        backend->event_client->handle_tuner_event(backend->event_client_data, event);
}

__attribute__((visibility("default")))
void
wpe_tvcontrol_backend_dispatch_source_event(struct wpe_tvcontrol_backend* backend, struct wpe_tvcontrol_event* event)
{
    if (backend->event_client)
        backend->event_client->handle_source_changed_event(backend->event_client_data, event);
}

__attribute__((visibility("default")))
void
wpe_tvcontrol_backend_dispatch_channel_event(struct wpe_tvcontrol_backend* backend, struct wpe_tvcontrol_event* event)
{
    if (backend->event_client)
        backend->event_client->handle_channel_changed_event(backend->event_client_data, event);
}

__attribute__((visibility("default")))
void
wpe_tvcontrol_backend_dispatch_scanning_state_event(struct wpe_tvcontrol_backend* backend, struct wpe_tvcontrol_event* event)
{
    if (backend->event_client)
        backend->event_client->handle_scanning_state_changed_event(backend->event_client_data, event);
}

__attribute__((visibility("default")))
tvcontrol_return
wpe_tvcontrol_backend_get_tuner_list(struct wpe_tvcontrol_backend* backend, struct wpe_tvcontrol_string_vector* out_tuner_list)
{
    tvcontrol_return ret = TVControlFailed;
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    ret = backend->interface->get_tuner_list(backend->interface_data, out_tuner_list);
    return ret;
}

__attribute__((visibility("default")))
void
wpe_tvcontrol_backend_get_supported_source_types_list(struct wpe_tvcontrol_backend* backend, const char* tuner_id, struct wpe_tvcontrol_src_types_vector* out_source_types_list)
{
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    backend->interface->get_supported_source_types_list(backend->interface_data, tuner_id, out_source_types_list);
    return;
}

__attribute__((visibility("default")))
tvcontrol_return
wpe_tvcontrol_backend_get_source_list(struct wpe_tvcontrol_backend* backend, const char* tuner_id, struct wpe_tvcontrol_src_types_vector* out_source_list)
{
    tvcontrol_return ret = TVControlFailed;
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    ret = backend->interface->get_source_list(backend->interface_data, tuner_id, out_source_list);
    return ret;
}

__attribute__((visibility("default")))
tvcontrol_return
wpe_tvcontrol_backend_set_current_source(struct wpe_tvcontrol_backend* backend, const char* tuner_id, SourceType sType)
{
    tvcontrol_return ret = TVControlFailed;
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    ret = backend->interface->set_current_source(backend->interface_data, tuner_id, sType);
    return ret;
}
__attribute__((visibility("default")))
void
wpe_tvcontrol_backend_get_signal_strength(struct wpe_tvcontrol_backend* backend, const char* tuner_id, double* out_signal_strength)
{
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    backend->interface->get_signal_strength(backend->interface_data, tuner_id, out_signal_strength);
    return;
}

__attribute__((visibility("default")))
tvcontrol_return
wpe_tvcontrol_backend_start_scanning(struct wpe_tvcontrol_backend* backend, const char* tuner_id, SourceType type, bool isRescanned)
{
    tvcontrol_return ret = TVControlFailed;
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    ret = backend->interface->start_scanning(backend->interface_data, tuner_id, type, isRescanned);
    return ret;
}

__attribute__((visibility("default")))
tvcontrol_return
wpe_tvcontrol_backend_stop_scanning(struct wpe_tvcontrol_backend* backend, const char* tuner_id)
{
    tvcontrol_return ret = TVControlFailed;
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    ret = backend->interface->stop_scanning(backend->interface_data, tuner_id);
    return ret;
}

__attribute__((visibility("default")))
tvcontrol_return
wpe_tvcontrol_backend_get_channel_list(struct wpe_tvcontrol_backend* backend, const char* tuner_id, SourceType type, struct wpe_tvcontrol_channel_vector** out_channel_list)
{
    tvcontrol_return ret = TVControlFailed;
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    ret = backend->interface->get_channel_list(backend->interface_data, tuner_id, type, out_channel_list);
    return ret;
}

__attribute__((visibility("default")))
tvcontrol_return
wpe_tvcontrol_backend_set_current_channel(struct wpe_tvcontrol_backend* backend, const char* tuner_id, SourceType type, uint64_t channel_number)
{
    tvcontrol_return ret = TVControlFailed;
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    ret = backend->interface->set_current_channel(backend->interface_data, tuner_id, type, channel_number);
    return ret;
}
