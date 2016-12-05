#ifndef NavigatorTVManager_h
#define NavigatorTVManager_h

#if ENABLE(TV_CONTROL)

#include "Supplementable.h"
#include <wtf/Vector.h>

namespace WebCore {


class TVManager;
class Navigator;

class NavigatorTVManager : public Supplement<Navigator> {
public:
    NavigatorTVManager();
    virtual ~NavigatorTVManager();

    static NavigatorTVManager* from(Navigator*);
    static TVManager* tv(Navigator&);

private:
    static const char* supplementName();

    RefPtr<TVManager> m_tvManager;
};

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // NavigatorTVManager_h
