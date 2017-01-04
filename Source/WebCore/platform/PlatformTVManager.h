#ifndef PlatformTVManager_h
#define PlatformTVManager_h

#if ENABLE(TV_CONTROL)

#include "PlatformTVTuner.h"

namespace WebCore {

class PlatformTVControlBackend;


class PlatformTVManagerClient {
public:
    virtual void didTunerOperationChanged(String tunerId, uint16_t event) = 0;
    virtual void didCurrentSourceChanged(String tunerId, String sourceId) = 0;
    virtual void didCurrentChannelChanged(String tunerId, String sourceId, String channelId) = 0;
    virtual void didScanningStateChanged(String tunerId, String sourceId, String channelId, uint16_t state) = 0;
protected:
    virtual ~PlatformTVManagerClient() { }
};


class WEBCORE_EXPORT PlatformTVManager {
public:
    WEBCORE_EXPORT explicit PlatformTVManager(PlatformTVManagerClient*);
    WEBCORE_EXPORT virtual ~PlatformTVManager();

    void  getTuners(Vector<RefPtr<PlatformTVTuner>>&);
    void  updateTunerList(String, uint16_t);

    PlatformTVControlBackend *m_tvBackend;
private:
    bool m_tunerListIsInitialized;
    PlatformTVManagerClient* m_platformTVManagerClient;
};

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // PlatformTVManager_h
