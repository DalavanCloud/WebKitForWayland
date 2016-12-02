#include "config.h"
#include "NavigatorTVManager.h"

#if ENABLE(TV_CONTROL)

#include "TVManager.h"
#include "Navigator.h"

namespace WebCore {

NavigatorTVManager::NavigatorTVManager()
{
}

NavigatorTVManager::~NavigatorTVManager()
{
}

TVManager* NavigatorTVManager::tv(Navigator& navigator)
{
    if (!navigator.frame())
        return nullptr;

    NavigatorTVManager* navigatorTVManager = NavigatorTVManager::from(&navigator);
    if (!navigatorTVManager->m_tvManager)
        navigatorTVManager->m_tvManager = TVManager::create(&navigator);
    return navigatorTVManager->m_tvManager.get();
}

const char* NavigatorTVManager::supplementName()
{
    return "NavigatorTVManager";
}

NavigatorTVManager* NavigatorTVManager::from(Navigator* navigator)
{
    NavigatorTVManager* supplement = static_cast<NavigatorTVManager*>(Supplement<Navigator>::from(navigator, supplementName()));
    if (!supplement) {
        auto newSupplement = std::make_unique<NavigatorTVManager>();
        supplement = newSupplement.get();
        provideTo(navigator, supplementName(), WTFMove(newSupplement));
    }
    return supplement;
}

} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
