#ifndef SOURCE_BACKEND_H_
#define SOURCE_BACKEND_H_

#include <wpe/tvcontrol-backend.h>

using namespace std;
#define TV_DEBUG 1 //TODO remove

namespace BCMRPi {

class SourceBackend {

public:
   SourceBackend(SourceType);
   virtual ~SourceBackend() {}

private:
    SourceType m_sType;
};


} // namespace BCMRPi
#endif //SOURCE_BACKEND_H_
