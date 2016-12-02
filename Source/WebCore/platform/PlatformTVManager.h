#ifndef PlatformTVManager_h
#define PlatformTVManager_h

#if ENABLE(TV_CONTROL)

#include "PlatformTVTuner.h"

struct wpe_tvcontrol_backend;

namespace WebCore {

class PlatformTVManagerClient {
public:
protected:
    virtual ~PlatformTVManagerClient() { }
};


class WEBCORE_EXPORT PlatformTVManager {

   enum REGION {
     US,
     EU
   };

public:
    WEBCORE_EXPORT explicit PlatformTVManager(PlatformTVManagerClient*);
    WEBCORE_EXPORT virtual ~PlatformTVManager();

    const Vector<RefPtr<PlatformTVTuner>>& getTuners();
private:
    Vector<RefPtr<PlatformTVTuner>> m_tunerList;
    bool m_tunerListIsInitialized;
    PlatformTVManagerClient* m_platformTVManagerClient;

    void CheckRegion();

    REGION m_region;
    struct wpe_tvcontrol_backend* m_backend;
};

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // PlatformTVManager_h
