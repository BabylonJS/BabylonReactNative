#pragma once

#include "IXrContextARCore.h"
#include <android/log.h>

#if __has_include("jsi/jsi.h")
#include "jsi/jsi.h"

namespace BabylonReactNative
{
    bool TryGetXrContext(facebook::jsi::Runtime& jsiRuntime, IXrContextARCore*& xrContext)
    {
        xrContext = nullptr;
        if (!jsiRuntime.global().hasProperty(jsiRuntime, "navigator"))
        {
            return false;
        }

        auto navigator{ jsiRuntime.global().getProperty(jsiRuntime, "navigator").asObject(jsiRuntime) };
        if (!navigator.hasProperty(jsiRuntime, "xr"))
        {
            return false;
        }

        auto nativeXr{ navigator.getProperty(jsiRuntime, "xr").asObject(jsiRuntime) };
        if (!nativeXr.hasProperty(jsiRuntime, "nativeXrContext") ||
            !nativeXr.hasProperty(jsiRuntime, "nativeXrContextType") ||
            nativeXr.getProperty(jsiRuntime, "nativeXrContextType").asString(jsiRuntime).utf8(jsiRuntime) != "ARCore")
        {
            return false;
        }

        auto nativeExtensionPtr = static_cast<uintptr_t>(nativeXr.getProperty(jsiRuntime, "nativeXrContext").asNumber());
        xrContext = reinterpret_cast<IXrContextARCore*>(nativeExtensionPtr);
        return true;
    }
}
#endif

#if __has_include("napi/env.h")
#include "napi/env.h"

namespace BabylonReactNative
{
    bool TryGetXrContext(Napi::Env env, IXrContextARCore*& xrContext)
    {
        xrContext = nullptr;
        if (!env.Global().Has("navigator"))
        {
            return false;
        }

        auto navigator{ env.Global().Get("navigator").As<Napi::Object>() };
        if (!navigator.Has("xr"))
        {
            return false;
        }

        auto nativeXr{ navigator.Get("xr").As<Napi::Object>() };
        if (!nativeXr.Has("nativeXrContext") ||
            !nativeXr.Has("nativeXrContextType") ||
            nativeXr.Get("nativeXrContextType").As<Napi::String>().Utf8Value() != "ARCore")
        {
            return false;
        }

        auto nativeExtensionPtr = static_cast<uintptr_t>(nativeXr.Get("nativeXrContext").As<Napi::Number>().DoubleValue());
        xrContext = reinterpret_cast<IXrContextARCore*>(nativeExtensionPtr);
        return true;
    }
}
#endif