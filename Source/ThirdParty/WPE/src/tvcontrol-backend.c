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
    struct wpe_tvcontrol_backend* backend = malloc(sizeof(struct wpe_tvcontrol_backend));
    if (!backend)
        return 0;

    backend->interface = wpe_load_object("_wpe_tvcontrol_backend_interface");
    backend->interface_data = backend->interface->create();

    return backend;
}

__attribute__((visibility("default")))
void
wpe_tvcontrol_backend_destroy(struct wpe_tvcontrol_backend* backend)
{
    backend->interface->destroy(backend->interface_data);
    backend->interface_data = 0;

    free(backend);
}

__attribute__((visibility("default")))
void
wpe_tvcontrol_backend_get_tuner_list(struct wpe_tvcontrol_backend* backend, struct wpe_tvcontrol_string_vector* out_tuner_list)
{
    backend->interface->get_tuner_list(backend->interface_data, out_tuner_list);
    return;
}

__attribute__((visibility("default")))
void
wpe_tvcontrol_backend_get_supported_source_types_list(struct wpe_tvcontrol_backend* backend, const char* tuner_id, struct wpe_tvcontrol_src_types_vector* out_source_types_list)
{
    backend->interface->get_supported_source_types_list(backend->interface_data, tuner_id, out_source_types_list);
    return;
}
