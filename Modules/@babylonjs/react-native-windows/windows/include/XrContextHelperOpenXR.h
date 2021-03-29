#pragma once

#if __has_include("jsi/jsi.h")
#include "jsi/jsi.h"
#include "IXrContextOpenXR.h"

namespace BabylonReactNative
{
    bool TryGetXrContext(facebook::jsi::Runtime& jsiRuntime, IXrContextOpenXR*& xrContext)
    {
        xrContext = nullptr;
        if (jsiRuntime.global().hasProperty(jsiRuntime, "navigator") &&
            jsiRuntime.global().getProperty(jsiRuntime, "navigator").asObject(jsiRuntime).hasProperty(jsiRuntime, "xr"))
        {
            auto nativeXr = jsiRuntime.global().getProperty(jsiRuntime, "navigator").asObject(jsiRuntime).getProperty(jsiRuntime, "xr").asObject(jsiRuntime);
            if (nativeXr.hasProperty(jsiRuntime, "nativeXrContext") &&
                nativeXr.hasProperty(jsiRuntime, "nativeXrContextType") &&
                nativeXr.getProperty(jsiRuntime, "nativeXrContextType").asString(jsiRuntime).utf8(jsiRuntime) == "OpenXR")
            {
                auto nativeExtensionPtr = static_cast<uintptr_t>(nativeXr.getProperty(jsiRuntime, "nativeXrContext").asNumber());
                xrContext = reinterpret_cast<IXrContextOpenXR*>(nativeExtensionPtr);
                return true;
            }
        }

        return false;
    }
}
#endif