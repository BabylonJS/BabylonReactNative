#pragma once
#include "NativeModules.h"

#include <winrt/Windows.ApplicationModel.h>
#include <windows.ui.core.h>

namespace winrt::BabylonNative::implementation {
    REACT_MODULE(BabylonModule, L"BabylonModule");
    struct BabylonModule
    {
        REACT_INIT(Initialize);
        void Initialize(winrt::Microsoft::ReactNative::ReactContext const& reactContext) noexcept
		{
			reactContext.ExecuteJsi([&](facebook::jsi::Runtime& /*jsiRuntime*/) {
                // TODO, this needs to be updated to use jsi compared to chakra
				auto env = Napi::Attach<>();

                // Populate polyfills
                Babylon::Polyfills::Window::Initialize(env);
                Babylon::Polyfills::XMLHttpRequest::Initialize(env);
                Babylon::Polyfills::Console::Initialize(env, [](const char* message, auto)
				{
					OutputDebugStringA(message);
				});


			});
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
