#pragma once

#include "IXrContextARCore.h"

#if __has_include("jsi/jsi.h")
#include "jsi/jsi.h"

namespace BabylonReactNative
{
    bool TryGetNativeAnchor(facebook::jsi::Runtime& jsiRuntime, facebook::jsi::Value& jsAnchor, ArAnchor*& nativeAnchor)
    {
        nativeAnchor = nullptr;
        if (!jsAnchor.isObject())
        {
            return false;
        }

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
        if (!nativeXr.hasProperty(jsiRuntime, "getNativeAnchor"))
        {
            return false;
        }

        auto getNativeAnchor{nativeXr.getPropertyAsFunction(jsiRuntime, "getNativeAnchor")};
        auto nativeAnchorPtr{static_cast<uintptr_t>(getNativeAnchor.call(jsiRuntime, { jsAnchor.asObject(jsiRuntime) }).asNumber())};

        nativeAnchor = reinterpret_cast<ArAnchor*>(nativeAnchorPtr);
        return true;
    }
}
#endif

#if __has_include("napi/env.h")
#include "napi/env.h"

namespace BabylonReactNative
{
    bool TryGetNativeAnchor(Napi::Env env, Napi::Value anchor, ArAnchor*& nativeAnchor)
    {
        nativeAnchor = nullptr;
        if (!anchor.IsObject())
        {
            return false;
        }

        if (!env.Global().Has("navigator"))
        {
            return false;
        }

        auto navigator{ env.Global().Get("navigator").ToObject() };
        if (!navigator.Has("xr"))
        {
            return false;
        }

        auto nativeXr{ navigator.Get("xr").ToObject() };
        if (!nativeXr.Has("getNativeAnchor"))
        {
            return false;
        }

        auto getNativeAnchor{nativeXr.Get("getNativeAnchor").As<Napi::Function>()};
        auto nativeAnchorPtr{static_cast<uintptr_t>(getNativeAnchor.Call({ anchor }).As<Napi::Number>().DoubleValue())};
        nativeAnchor = reinterpret_cast<ArAnchor*>(nativeAnchorPtr);
        return true;
    }
}
#endif