/*
 *  This file is part of the WebKit open source project.
 *  This file has been generated by generate-bindings.pl. DO NOT MODIFY!
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include "WebKitDOMTestGlobalObject.h"

#include "CSSImportRule.h"
#include "DOMObjectCache.h"
#include "Document.h"
#include "ExceptionCode.h"
#include "ExceptionCodeDescription.h"
#include "JSMainThreadExecState.h"
#include "WebKitDOMPrivate.h"
#include "WebKitDOMTestGlobalObjectPrivate.h"
#include "gobject/ConvertToUTF8String.h"
#include <wtf/GetPtr.h>
#include <wtf/RefPtr.h>

#define WEBKIT_DOM_TEST_GLOBAL_OBJECT_GET_PRIVATE(obj) G_TYPE_INSTANCE_GET_PRIVATE(obj, WEBKIT_DOM_TYPE_TEST_GLOBAL_OBJECT, WebKitDOMTestGlobalObjectPrivate)

typedef struct _WebKitDOMTestGlobalObjectPrivate {
    RefPtr<WebCore::TestGlobalObject> coreObject;
} WebKitDOMTestGlobalObjectPrivate;

namespace WebKit {

WebKitDOMTestGlobalObject* kit(WebCore::TestGlobalObject* obj)
{
    if (!obj)
        return 0;

    if (gpointer ret = DOMObjectCache::get(obj))
        return WEBKIT_DOM_TEST_GLOBAL_OBJECT(ret);

    return wrapTestGlobalObject(obj);
}

WebCore::TestGlobalObject* core(WebKitDOMTestGlobalObject* request)
{
    return request ? static_cast<WebCore::TestGlobalObject*>(WEBKIT_DOM_OBJECT(request)->coreObject) : 0;
}

WebKitDOMTestGlobalObject* wrapTestGlobalObject(WebCore::TestGlobalObject* coreObject)
{
    ASSERT(coreObject);
    return WEBKIT_DOM_TEST_GLOBAL_OBJECT(g_object_new(WEBKIT_DOM_TYPE_TEST_GLOBAL_OBJECT, "core-object", coreObject, nullptr));
}

} // namespace WebKit

G_DEFINE_TYPE(WebKitDOMTestGlobalObject, webkit_dom_test_global_object, WEBKIT_DOM_TYPE_OBJECT)

enum {
    PROP_0,
    PROP_REGULAR_ATTRIBUTE,
    PROP_ENABLED_AT_RUNTIME_ATTRIBUTE,
};

static void webkit_dom_test_global_object_finalize(GObject* object)
{
    WebKitDOMTestGlobalObjectPrivate* priv = WEBKIT_DOM_TEST_GLOBAL_OBJECT_GET_PRIVATE(object);

    WebKit::DOMObjectCache::forget(priv->coreObject.get());

    priv->~WebKitDOMTestGlobalObjectPrivate();
    G_OBJECT_CLASS(webkit_dom_test_global_object_parent_class)->finalize(object);
}

static void webkit_dom_test_global_object_set_property(GObject* object, guint propertyId, const GValue* value, GParamSpec* pspec)
{
    WebKitDOMTestGlobalObject* self = WEBKIT_DOM_TEST_GLOBAL_OBJECT(object);

    switch (propertyId) {
    case PROP_REGULAR_ATTRIBUTE:
        webkit_dom_test_global_object_set_regular_attribute(self, g_value_get_string(value));
        break;
    case PROP_ENABLED_AT_RUNTIME_ATTRIBUTE:
        webkit_dom_test_global_object_set_enabled_at_runtime_attribute(self, g_value_get_string(value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, propertyId, pspec);
        break;
    }
}

static void webkit_dom_test_global_object_get_property(GObject* object, guint propertyId, GValue* value, GParamSpec* pspec)
{
    WebKitDOMTestGlobalObject* self = WEBKIT_DOM_TEST_GLOBAL_OBJECT(object);

    switch (propertyId) {
    case PROP_REGULAR_ATTRIBUTE:
        g_value_take_string(value, webkit_dom_test_global_object_get_regular_attribute(self));
        break;
    case PROP_ENABLED_AT_RUNTIME_ATTRIBUTE:
        g_value_take_string(value, webkit_dom_test_global_object_get_enabled_at_runtime_attribute(self));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, propertyId, pspec);
        break;
    }
}

static GObject* webkit_dom_test_global_object_constructor(GType type, guint constructPropertiesCount, GObjectConstructParam* constructProperties)
{
    GObject* object = G_OBJECT_CLASS(webkit_dom_test_global_object_parent_class)->constructor(type, constructPropertiesCount, constructProperties);

    WebKitDOMTestGlobalObjectPrivate* priv = WEBKIT_DOM_TEST_GLOBAL_OBJECT_GET_PRIVATE(object);
    priv->coreObject = static_cast<WebCore::TestGlobalObject*>(WEBKIT_DOM_OBJECT(object)->coreObject);
    WebKit::DOMObjectCache::put(priv->coreObject.get(), object);

    return object;
}

static void webkit_dom_test_global_object_class_init(WebKitDOMTestGlobalObjectClass* requestClass)
{
    GObjectClass* gobjectClass = G_OBJECT_CLASS(requestClass);
    g_type_class_add_private(gobjectClass, sizeof(WebKitDOMTestGlobalObjectPrivate));
    gobjectClass->constructor = webkit_dom_test_global_object_constructor;
    gobjectClass->finalize = webkit_dom_test_global_object_finalize;
    gobjectClass->set_property = webkit_dom_test_global_object_set_property;
    gobjectClass->get_property = webkit_dom_test_global_object_get_property;

    g_object_class_install_property(
        gobjectClass,
        PROP_REGULAR_ATTRIBUTE,
        g_param_spec_string(
            "regular-attribute",
            "TestGlobalObject:regular-attribute",
            "read-write gchar* TestGlobalObject:regular-attribute",
            "",
            WEBKIT_PARAM_READWRITE));

    g_object_class_install_property(
        gobjectClass,
        PROP_ENABLED_AT_RUNTIME_ATTRIBUTE,
        g_param_spec_string(
            "enabled-at-runtime-attribute",
            "TestGlobalObject:enabled-at-runtime-attribute",
            "read-write gchar* TestGlobalObject:enabled-at-runtime-attribute",
            "",
            WEBKIT_PARAM_READWRITE));

}

static void webkit_dom_test_global_object_init(WebKitDOMTestGlobalObject* request)
{
    WebKitDOMTestGlobalObjectPrivate* priv = WEBKIT_DOM_TEST_GLOBAL_OBJECT_GET_PRIVATE(request);
    new (priv) WebKitDOMTestGlobalObjectPrivate();
}

void webkit_dom_test_global_object_regular_operation(WebKitDOMTestGlobalObject* self, const gchar* testParam)
{
    WebCore::JSMainThreadNullState state;
    g_return_if_fail(WEBKIT_DOM_IS_TEST_GLOBAL_OBJECT(self));
    g_return_if_fail(testParam);
    WebCore::TestGlobalObject* item = WebKit::core(self);
    WTF::String convertedTestParam = WTF::String::fromUTF8(testParam);
    item->regularOperation(convertedTestParam);
}

void webkit_dom_test_global_object_enabled_at_runtime_operation(WebKitDOMTestGlobalObject* self, const gchar* testParam)
{
#if ENABLE(TEST_FEATURE)
    WebCore::JSMainThreadNullState state;
    g_return_if_fail(WEBKIT_DOM_IS_TEST_GLOBAL_OBJECT(self));
    g_return_if_fail(testParam);
    WebCore::TestGlobalObject* item = WebKit::core(self);
    WTF::String convertedTestParam = WTF::String::fromUTF8(testParam);
    item->enabledAtRuntimeOperation(convertedTestParam);
#else
    UNUSED_PARAM(self);
    UNUSED_PARAM(testParam);
    WEBKIT_WARN_FEATURE_NOT_PRESENT("Test Feature")
#endif /* ENABLE(TEST_FEATURE) */
}

void webkit_dom_test_global_object_enabled_at_runtime_operation(WebKitDOMTestGlobalObject* self, glong testParam)
{
#if ENABLE(TEST_FEATURE)
    WebCore::JSMainThreadNullState state;
    g_return_if_fail(WEBKIT_DOM_IS_TEST_GLOBAL_OBJECT(self));
    WebCore::TestGlobalObject* item = WebKit::core(self);
    item->enabledAtRuntimeOperation(testParam);
#else
    UNUSED_PARAM(self);
    UNUSED_PARAM(testParam);
    WEBKIT_WARN_FEATURE_NOT_PRESENT("Test Feature")
#endif /* ENABLE(TEST_FEATURE) */
}

gchar* webkit_dom_test_global_object_get_regular_attribute(WebKitDOMTestGlobalObject* self)
{
    WebCore::JSMainThreadNullState state;
    g_return_val_if_fail(WEBKIT_DOM_IS_TEST_GLOBAL_OBJECT(self), 0);
    WebCore::TestGlobalObject* item = WebKit::core(self);
    gchar* result = convertToUTF8String(item->regularAttribute());
    return result;
}

void webkit_dom_test_global_object_set_regular_attribute(WebKitDOMTestGlobalObject* self, const gchar* value)
{
    WebCore::JSMainThreadNullState state;
    g_return_if_fail(WEBKIT_DOM_IS_TEST_GLOBAL_OBJECT(self));
    g_return_if_fail(value);
    WebCore::TestGlobalObject* item = WebKit::core(self);
    WTF::String convertedValue = WTF::String::fromUTF8(value);
    item->setRegularAttribute(convertedValue);
}

gchar* webkit_dom_test_global_object_get_enabled_at_runtime_attribute(WebKitDOMTestGlobalObject* self)
{
#if ENABLE(TEST_FEATURE)
    WebCore::JSMainThreadNullState state;
    g_return_val_if_fail(WEBKIT_DOM_IS_TEST_GLOBAL_OBJECT(self), 0);
    WebCore::TestGlobalObject* item = WebKit::core(self);
    gchar* result = convertToUTF8String(item->enabledAtRuntimeAttribute());
    return result;
#else
    UNUSED_PARAM(self);
    WEBKIT_WARN_FEATURE_NOT_PRESENT("Test Feature")
    return 0;
#endif /* ENABLE(TEST_FEATURE) */
}

void webkit_dom_test_global_object_set_enabled_at_runtime_attribute(WebKitDOMTestGlobalObject* self, const gchar* value)
{
#if ENABLE(TEST_FEATURE)
    WebCore::JSMainThreadNullState state;
    g_return_if_fail(WEBKIT_DOM_IS_TEST_GLOBAL_OBJECT(self));
    g_return_if_fail(value);
    WebCore::TestGlobalObject* item = WebKit::core(self);
    WTF::String convertedValue = WTF::String::fromUTF8(value);
    item->setEnabledAtRuntimeAttribute(convertedValue);
#else
    UNUSED_PARAM(self);
    UNUSED_PARAM(value);
    WEBKIT_WARN_FEATURE_NOT_PRESENT("Test Feature")
#endif /* ENABLE(TEST_FEATURE) */
}

