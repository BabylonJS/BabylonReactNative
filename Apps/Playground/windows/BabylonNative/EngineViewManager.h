#pragma once
#include "winrt/Microsoft.ReactNative.h"
#include "NativeModules.h"

namespace winrt::BabylonNative::implementation {
    REACT_MODULE(EngineViewManager, L"EngineViewManager");
    struct EngineViewManager
    {
        REACT_INIT(Initialize);
        void Initialize(winrt::Microsoft::ReactNative::ReactContext const& /*reactContext*/) noexcept
        {
        }

        REACT_METHOD(SetJSThread, L"setJSThread");
        void SetJSThread() noexcept
        {
            // native modules for react-native-windows should use the javascript thread by default on 0.64+
        }

        ~EngineViewManager() {}
    };

} // namespace winrt::BabylonNative::implementation
