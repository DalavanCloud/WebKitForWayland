#ifndef NavigatorTVManager_h
#define NavigatorTVManager_h

#if ENABLE(TV_CONTROL)

#include "DOMWindowProperty.h"
#include "Supplementable.h"
#include <wtf/Vector.h>

namespace WebCore {

class Frame;
class TVManager;
class Navigator;

class NavigatorTVManager : public Supplement<Navigator>, public DOMWindowProperty {
public:
    explicit NavigatorTVManager(Frame*);
    virtual ~NavigatorTVManager();

    static NavigatorTVManager* from(Navigator*);
    static TVManager* tv(Navigator&);
    TVManager* tv() const;

private:
    static const char* supplementName();

    mutable RefPtr<TVManager> m_tvManager;
};

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)

#endif // NavigatorTVManager_h
