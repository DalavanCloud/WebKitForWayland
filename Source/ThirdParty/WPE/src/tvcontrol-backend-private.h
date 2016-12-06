#ifndef wpe_tvcontrol_backend_private_h
#define wpe_tvcontrol_backend_private_h

#ifdef __cplusplus
extern "C" {
#endif

struct wpe_tvcontrol_backend {
    const struct wpe_tvcontrol_backend_interface* interface;
    void* interface_data;

    struct wpe_tvcontrol_backend_manager_event_client* event_client;
    void* event_client_data;
};

#ifdef __cplusplus
}
#endif

#endif // wpe_tvcontrol_backend_private_h
