#ifndef wpe_tvcontrol_backend_h
#define wpe_tvcontrol_backend_h

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __GBM__
#define __GBM__
#endif

#include <stdint.h>

typedef enum { Added, Removed } tuner_changed_operation;
typedef enum { Cleared, Scanned, Completed, Stopped } scanning_state;

struct wpe_tvcontrol_string {
    char* data;
    uint64_t length;
};

struct wpe_tvcontrol_string_vector {
    struct wpe_tvcontrol_string* strings;
    uint64_t length;
};

struct wpe_tvcontrol_tuner_event {
    struct wpe_tvcontrol_string tuner_id;
    tuner_changed_operation operation;
};

struct wpe_tvcontrol_source_event {
    struct wpe_tvcontrol_string tuner_id;
    struct wpe_tvcontrol_string source_id;
};

struct wpe_tvcontrol_channel_event {
    struct wpe_tvcontrol_string tuner_id;
    struct wpe_tvcontrol_string source_id;
    struct wpe_tvcontrol_string channel_id;
    scanning_state              state;
};

struct wpe_tvcontrol_backend_manager_event_client {
    void (*handle_tuner_event)(void*, struct wpe_tvcontrol_tuner_event);
    void (*handle_source_changed_event)(void*, struct wpe_tvcontrol_source_event);
    void (*handle_channel_changed_event)(void*, struct wpe_tvcontrol_channel_event);
    void (*handle_scanning_state_changed_event)(void*, struct wpe_tvcontrol_channel_event);
};

typedef enum { DvbT, DvbT2, DvbC, DvbC2, DvbS, DvbS2, DvbH, DvbSh, Atsc, AtscMH, IsdbT, IsdbTb, IsdbS, IsdbC, _1seg, Dtmb, Cmmb, TDmb, SDmb, Undifined } Type;
struct wpe_tvcontrol_src_types_vector {
    Type*    types;
    uint64_t length;
};

struct wpe_tvcontrol_backend_interface {
    void* (*create)(struct  wpe_tvcontrol_backend*);
    void (*destroy)(void*);
    void (*get_tuner_list)(void*, struct wpe_tvcontrol_string_vector*);
    void (*get_supported_source_types_list)(void*, const char*, struct wpe_tvcontrol_src_types_vector*);
    void (*get_signal_strength)(void*, const char*, double*);
};

struct wpe_tvcontrol_backend*
wpe_tvcontrol_backend_create();

void
wpe_tvcontrol_backend_destroy(struct wpe_tvcontrol_backend*);

void
wpe_tvcontrol_backend_set_manager_event_client(struct wpe_tvcontrol_backend*, struct wpe_tvcontrol_backend_manager_event_client*, void*);

void
wpe_tvcontrol_backend_dispatch_tuner_event(struct wpe_tvcontrol_backend*, struct wpe_tvcontrol_tuner_event);

void
wpe_tvcontrol_backend_dispatch_source_event(struct wpe_tvcontrol_backend*, struct wpe_tvcontrol_source_event);

void
wpe_tvcontrol_backend_dispatch_channel_event(struct wpe_tvcontrol_backend*, struct wpe_tvcontrol_channel_event);

void
wpe_tvcontrol_backend_dispatch_scanning_state_event(struct wpe_tvcontrol_backend*, struct wpe_tvcontrol_channel_event);

void
wpe_tvcontrol_backend_get_tuner_list(struct wpe_tvcontrol_backend* backend, struct wpe_tvcontrol_string_vector* out_tuner_list);

void
wpe_tvcontrol_backend_get_supported_source_types_list(struct wpe_tvcontrol_backend* backend, const char* tuner_id, struct wpe_tvcontrol_src_types_vector* out_source_types_list);

void
wpe_tvcontrol_backend_get_signal_strength(struct wpe_tvcontrol_backend* backend, const char* tuner_id, double* signal_strength);

#ifdef __cplusplus
}
#endif

#endif // wpe_tvcontrol_backend_h
