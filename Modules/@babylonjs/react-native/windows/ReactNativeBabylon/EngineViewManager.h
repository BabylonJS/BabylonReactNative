#pragma once
#include "winrt/Microsoft.ReactNative.h"
#include "NativeModules.h"

namespace winrt::ReactNativeBabylon::implementation {
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
            // Native Modules for react-native-windows should use the javascript thread by default on 0.64+
            // To access the JSThread, use winrt::Microsoft::ReactNative::ExecuteJsi
        }

        ~EngineViewManager() {}
    };

} // namespace winrt::ReactNativeBabylon::implementation
