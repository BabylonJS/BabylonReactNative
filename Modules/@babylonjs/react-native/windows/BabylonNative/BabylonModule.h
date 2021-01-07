#pragma once

#include "NativeModules.h"

namespace winrt::BabylonNative::implementation {
    REACT_MODULE(BabylonModule, L"BabylonModule");
    struct BabylonModule
    {
        REACT_INIT(Initialize);
        void Initialize(const winrt::Microsoft::ReactNative::ReactContext& reactContext) noexcept;

        REACT_METHOD(CustomInitialize, L"initialize");
        void CustomInitialize(const winrt::Microsoft::ReactNative::ReactPromise<bool>& result) noexcept;

        REACT_METHOD(WhenInitialized, L"whenInitialized");
        void WhenInitialized(const winrt::Microsoft::ReactNative::ReactPromise<bool>& result) noexcept;

        REACT_METHOD(Reset, L"reset");
        void Reset(const winrt::Microsoft::ReactNative::ReactPromise<bool>& result) noexcept;

        BabylonModule();
        ~BabylonModule();

    private:
        winrt::Microsoft::ReactNative::IReactContext _reactContext;
    };
} // namespace winrt::BabylonNative::implementation
