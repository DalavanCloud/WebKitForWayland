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
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    NavigatorTVManager* supplement = static_cast<NavigatorTVManager*>(Supplement<Navigator>::from(navigator, supplementName()));
    if (!supplement) {
        auto newSupplement = std::make_unique<NavigatorTVManager>(navigator->frame());
        supplement = newSupplement.get();
        provideTo(navigator, supplementName(), WTFMove(newSupplement));
    }
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return supplement;
}

TVManager* NavigatorTVManager::tv(Navigator& navigator)
{
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return NavigatorTVManager::from(&navigator)->tv();
}

TVManager* NavigatorTVManager::tv() const
{
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (!m_tvManager && frame()) {
        printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        m_tvManager = TVManager::create(frame()->document());
    }
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return m_tvManager.get();
}


} // namespace WebCore

#endif // ENABLE(TV_CONTROL)
