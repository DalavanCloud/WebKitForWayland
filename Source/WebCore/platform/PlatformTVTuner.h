#ifndef PlatformTVTuner_h
#define PlatformTVTuner_h

#if ENABLE(TV_CONTROL)

#include "PlatformTVSource.h"

namespace WebCore {

class PlatformTVTunerClient {
public:
protected:
    virtual ~PlatformTVTunerClient() { }
};

class PlatformTVTuner : public RefCounted<PlatformTVTuner> {
public:
public:
    static RefPtr<PlatformTVTuner> create(String id);
    
    virtual ~PlatformTVTuner();
    
    const Vector<TVSourceType>&              getSupportedSourceTypes();
    const Vector<RefPtr<PlatformTVSource>>&  getSources ();
    void                                     setCurrentSource (TVSourceType sourceType);
    void                                     setTunerClient (PlatformTVTunerClient* client);
        
    const String&      id () const { return m_tunerId; }
    PlatformTVSource*  currentSource() const { return m_currentSource; } //TODO check again
    //TVMediaStream*   stream() const { return nullptr; } //TODO enable if it is required for basic functionalities
    double             signalStrength() const { return m_signalStrength; }
private:
    PlatformTVTuner(String id);
    String                 m_tuneId;
    PlatformTVSource*      m_currentSource;
    double                 m_signalStrength;
    TVSourceType           m_currentSourceType;
    PlatformTVTunerClient* m_platformTVTunerClient;
    
    Vector<TVSourceType> m_sourceTypeList;
    bool m_sourceTypeListIsInitialized;
    Vector<RefPtr<PlatformTVSource>> m_sourceList;
    bool m_sourceListIsInitialized;
};

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // PlatformTVTuner_h
