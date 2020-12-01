#pragma once

#include "NativeModules.h"
#include "JSI/JsiApi.h"

#include "winrt/Windows.UI.Core.h"
#include "winrt/Windows.Graphics.Display.h"

namespace winrt::BabylonNative::implementation {
    REACT_MODULE(BabylonModule, L"BabylonModule");
    struct BabylonModule
    {
        REACT_INIT(Initialize);
        void Initialize(winrt::Microsoft::ReactNative::ReactContext const& reactContext) noexcept
        {
            auto dispatcher = reactContext.JSDispatcher();
            winrt::Microsoft::ReactNative::ExecuteJsi(reactContext, [&](facebook::jsi::Runtime& jsiRuntime) {
                auto env = Napi::Attach<facebook::jsi::Runtime&>(jsiRuntime);

                bool isShuttingDown = false;
                auto runtime = &Babylon::JsRuntime::CreateForJavaScript(env, CreateJsRuntimeDispatcher(env, jsiRuntime, dispatcher, isShuttingDown));

                auto coreWindow = winrt::Windows::UI::Core::CoreWindow::GetForCurrentThread();
                void* windowPtr;
                copy_to_abi(coreWindow, windowPtr);

                auto displayScale = static_cast<float>(winrt::Windows::Graphics::Display::DisplayInformation::GetForCurrentView().RawPixelsPerViewPixel());
                auto width = static_cast<size_t>(coreWindow.Bounds().Width * displayScale);
                auto height = static_cast<size_t>(coreWindow.Bounds().Height * displayScale);

                auto graphics = Babylon::Graphics::CreateGraphics(windowPtr, width, height);
                graphics->AddToJavaScript(env);

                // Populate polyfills
                Babylon::Polyfills::Window::Initialize(env);
                Babylon::Polyfills::XMLHttpRequest::Initialize(env);
                Babylon::Polyfills::Console::Initialize(env, [](const char* message, auto)
                {
                    OutputDebugStringA(message);
                });

                // Populate plugins
                Babylon::Plugins::NativeEngine::Initialize(env, true);
                Babylon::Plugins::NativeXr::Initialize(env);

                // TODO hook up NativeInput
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

        ~BabylonModule()
        {
        }

    private:
        static Babylon::JsRuntime::DispatchFunctionT CreateJsRuntimeDispatcher(
            Napi::Env env,
            facebook::jsi::Runtime& jsiRuntime,
            const winrt::Microsoft::ReactNative::ReactDispatcher& dispatcher,
            const bool& isShuttingDown)
        {
            return [env, &jsiRuntime, &dispatcher, &isShuttingDown](std::function<void(Napi::Env)> func)
            {
                // Ideally we would just use CallInvoker::invokeAsync directly, but currently it does not seem to integrate well with the React Native logbox.
                // To work around this, we wrap all functions in a try/catch, and when there is an exception, we do the following:
                // 1. Call the JavaScript setImmediate function.
                // 2. Have the setImmediate callback call back into native code (throwFunc).
                // 3. Re-throw the exception from throwFunc.
                // This works because:
                // 1. setImmediate queues the callback, and that queue is drained immediately following the invocation of the function passed to CallInvoker::invokeAsync.
                // 2. The immediates queue is drained as part of the class bridge, which knows how to display the logbox for unhandled exceptions.
                // In the future, CallInvoker::invokeAsync likely will properly integrate with logbox, at which point we can remove the try/catch and just call func directly.
                dispatcher.Post([env, &jsiRuntime, func{ std::move(func) }, &isShuttingDown]
                    {
                        try
                        {
                            // If JS engine shutdown is in progress, don't dispatch any new work.
                            if (!isShuttingDown)
                            {
                                func(env);
                            }
                        }
                        catch (...)
                        {
                            auto ex{std::current_exception()};
                            auto setImmediate{jsiRuntime.global().getPropertyAsFunction(jsiRuntime, "setImmediate")};
                            auto throwFunc{facebook::jsi::Function::createFromHostFunction(jsiRuntime, facebook::jsi::PropNameID::forAscii(jsiRuntime, "throwFunc"), 0,
                                [ex](facebook::jsi::Runtime&, const facebook::jsi::Value&, const facebook::jsi::Value*, size_t) -> facebook::jsi::Value
                                {
                                    std::rethrow_exception(ex);
                                })};
                            setImmediate.call(jsiRuntime, {std::move(throwFunc)});
                        }
                    });
            };
        }
    };
} // namespace winrt::BabylonNative::implementation
