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

    const Vector<PlatformTVSource::Type>&    getSupportedSourceTypes();
    const Vector<RefPtr<PlatformTVSource>>&  getSources ();
    void                                     setCurrentSource (PlatformTVSource::Type sourceType);
    void                                     setTunerClient (PlatformTVTunerClient* client);

    const String&      id () const { return m_tunerId; }
    PlatformTVSource*  currentSource() const { return m_currentSource; } //TODO check again
    //TVMediaStream*   stream() const { return nullptr; } //TODO enable if it is required for basic functionalities
    double             signalStrength();
private:
    PlatformTVTuner(String, PlatformTVControlBackend*);
    String                     m_tunerId;
    PlatformTVSource*          m_currentSource;
    double                     m_signalStrength;
    PlatformTVSource::Type     m_currentSourceType;
    PlatformTVTunerClient*     m_platformTVTunerClient;
    PlatformTVControlBackend*  m_tvBackend;

    Vector<PlatformTVSource::Type> m_sourceTypeList;
    bool m_sourceTypeListIsInitialized;
    Vector<RefPtr<PlatformTVSource>> m_sourceList;
    bool m_sourceListIsInitialized;
};

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // PlatformTVTuner_h
