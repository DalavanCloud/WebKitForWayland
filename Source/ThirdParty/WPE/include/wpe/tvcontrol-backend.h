#ifndef wpe_tvcontrol_backend_h
#define wpe_tvcontrol_backend_h

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __GBM__
#define __GBM__
#endif

#include <stdint.h>

struct wpe_tvcontrol_string {
    char* data;
    uint64_t length;
};

struct wpe_tvcontrol_string_vector {
    struct wpe_tvcontrol_string* strings;
    uint64_t length;
};
     

struct wpe_tvcontrol_backend_interface {
    void* (*create)();
    void (*destroy)(void*);
    void (*get_tuner_list)(void*, struct wpe_tvcontrol_string_vector*);
    void (*get_supported_source_types_list)(void*, const char*, struct wpe_tvcontrol_string_vector*);
};

struct wpe_tvcontrol_backend*
wpe_tvcontrol_backend_create();

void
wpe_tvcontrol_backend_destroy(struct wpe_tvcontrol_backend*);

void
wpe_tvcontrol_backend_get_tuner_list(struct wpe_tvcontrol_backend* backend, struct wpe_tvcontrol_string_vector* out_tuner_list);

void
wpe_tvcontrol_backend_get_supported_source_types_list(struct wpe_tvcontrol_backend* backend, const char* tuner_id, struct wpe_tvcontrol_string_vector* out_source_types_list);

#ifdef __cplusplus
}
#endif

#endif // wpe_renderer_h
