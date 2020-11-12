#pragma once
#include "NativeModules.h"

namespace winrt::BabylonNative::implementation {
    REACT_MODULE(BabylonModule, L"BabylonModule");
    struct BabylonModule
    {
        REACT_INIT(Initialize);
        void Initialize(winrt::Microsoft::ReactNative::ReactContext const& /*reactContext*/) noexcept
        {
        }

        REACT_METHOD(CustomInitialize, L"initialize");
        void CustomInitialize(const winrt::Microsoft::ReactNative::ReactPromise<bool>& result) noexcept
        {
            result.Resolve(false);
        }

        REACT_METHOD(WhenInitialized, L"whenInitialized");
        void WhenInitialized(const winrt::Microsoft::ReactNative::ReactPromise<bool>& result) noexcept
        {
            result.Resolve(false);
        }

        ~BabylonModule() {}
    };
} // namespace winrt::BabylonNative::implementation
