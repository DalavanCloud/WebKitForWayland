#ifndef PlatformTVTuner_h
#define PlatformTVTuner_h

#if ENABLE(TV_CONTROL)

#include "PlatformTVSource.h"

namespace WebCore {

class PlatformTVControlBackend;

class PlatformTVTunerClient {
public:
protected:
    virtual ~PlatformTVTunerClient() { }
};

class PlatformTVTuner : public RefCounted<PlatformTVTuner> {
public:
    static RefPtr<PlatformTVTuner> create(String, PlatformTVControlBackend*);

    virtual ~PlatformTVTuner();

    bool getSupportedSourceTypes(Vector<PlatformTVSource::Type>& sourceTypeVector);
    bool getSources (Vector<RefPtr<PlatformTVSource>>& sourceVector);
    bool setCurrentSource(PlatformTVSource::Type sourceType);
    void setTunerClient (PlatformTVTunerClient* client);

    const String&             id () const { return m_tunerId; }
    double                    signalStrength();
private:
    PlatformTVTuner(String, PlatformTVControlBackend*);
    String                     m_tunerId;
    double                     m_signalStrength;
    PlatformTVTunerClient*     m_platformTVTunerClient;
    PlatformTVControlBackend*  m_tvBackend;

    Vector<PlatformTVSource::Type> m_sourceTypeList;
};

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // PlatformTVTuner_h
