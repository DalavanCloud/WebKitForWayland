#include "config.h"
#include "NavigatorTVManager.h"

#if ENABLE(TV_CONTROL)

#include "TVManager.h"
#include "Document.h"
#include "Frame.h"
#include "Navigator.h"

namespace WebCore {

NavigatorTVManager::NavigatorTVManager(Frame* frame)
    : DOMWindowProperty(frame)
{
}

NavigatorTVManager::~NavigatorTVManager()
{
}

const char* NavigatorTVManager::supplementName()
{
    return "NavigatorTVManager";
}

NavigatorTVManager* NavigatorTVManager::from(Navigator* navigator)
{
    NavigatorTVManager* supplement = static_cast<NavigatorTVManager*>(Supplement<Navigator>::from(navigator, supplementName()));
    if (!supplement) {
        auto newSupplement = std::make_unique<NavigatorTVManager>(navigator->frame());
        supplement = newSupplement.get();
        provideTo(navigator, supplementName(), WTFMove(newSupplement));
    }
    return supplement;
}

TVManager* NavigatorTVManager::tv(Navigator& navigator)
{
    return NavigatorTVManager::from(&navigator)->tv();
}

TVManager* NavigatorTVManager::tv() const
{
    if (!m_tvManager && frame())
        m_tvManager = TVManager::create(frame()->document());
    return m_tvManager.get();
}


} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
