#ifndef PlatformTVControl_h
#define PlatformTVControl_h

#if ENABLE(TV_CONTROL)

struct wpe_tvcontrol_backend;
struct wpe_tvcontrol_channel;

namespace WebCore {

class PlatformTVControlBackend {
public: 
    struct wpe_tvcontrol_backend* m_backend;
    struct wpe_tvcontrol_channel* m_channel;
};

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // PlatformTVControl_h
