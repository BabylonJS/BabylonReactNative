#pragma once

#include "NativeModules.h"
#include "JSI/JsiApi.h"

#include "winrt/Windows.UI.Core.h"
#include "winrt/Windows.Graphics.Display.h"
#include "winrt/Windows.Foundation.Numerics.h"

namespace winrt::BabylonNative::implementation {
    REACT_MODULE(BabylonModule, L"BabylonModule");
    struct BabylonModule : std::enable_shared_from_this<BabylonModule>
    {
        REACT_INIT(Initialize);
        void Initialize(winrt::Microsoft::ReactNative::ReactContext const& reactContext) noexcept
        {
            _reactContext = reactContext;
            _jsDispatcher = reactContext.JSDispatcher();
            _uiDispatcher = reactContext.UIDispatcher();

            // TODO use shared_ptr/weak_ptr to this module compared to handing around references
            // modules are kept around by react native, this module can be killed
            _uiDispatcher.Post([weakThis{ this->weak_from_this() }]{
                if (auto trueThis = weakThis.lock())
                {
                    trueThis->_coreWindow = winrt::Windows::UI::Core::CoreWindow::GetForCurrentThread();
                    auto displayInformation = winrt::Windows::Graphics::Display::DisplayInformation::GetForCurrentView();
                    trueThis->_windowSize.x = static_cast<float>(trueThis->_coreWindow.Bounds().Width * displayInformation.RawPixelsPerViewPixel());
                    trueThis->_windowSize.y = static_cast<float>(trueThis->_coreWindow.Bounds().Height * displayInformation.RawPixelsPerViewPixel());

                    winrt::Microsoft::ReactNative::ExecuteJsi(trueThis->_reactContext, [weakThis](facebook::jsi::Runtime& jsiRuntime) {
                        if (auto trueThis = weakThis.lock())
                        {
                            trueThis->SetupBabylonNative(jsiRuntime);
                        }
                    });
                }
            });
        }

        REACT_METHOD(CustomInitialize, L"initialize");
        void CustomInitialize(const winrt::Microsoft::ReactNative::ReactPromise<bool>& result) noexcept
        {
            if (_initialized)
            {
                result.Resolve(_initializationSucceeded);
                return;
            }

            {
                std::lock_guard<std::mutex> lock(_initializedPromiseLock);
                if (_initialized)
                {
                    result.Resolve(_initializationSucceeded);
                    return;
                }

                _initializedPromises.push_back(result);
            }
        }

        REACT_METHOD(WhenInitialized, L"whenInitialized");
        void WhenInitialized(const winrt::Microsoft::ReactNative::ReactPromise<bool>& result) noexcept
        {
            if (_initialized)
            {
                result.Resolve(_initializationSucceeded);
                return;
            }

            {
                std::lock_guard<std::mutex> lock(_initializedPromiseLock);
                if (_initialized)
                {
                    result.Resolve(_initializationSucceeded);
                    return;
                }

                _initializedPromises.push_back(result);
            }
        }

        BabylonModule()
        {
        }

        ~BabylonModule()
        {
        }

    private:
        winrt::Microsoft::ReactNative::ReactContext _reactContext;
        winrt::Microsoft::ReactNative::ReactDispatcher _jsDispatcher;
        winrt::Microsoft::ReactNative::ReactDispatcher _uiDispatcher;

        Napi::Env _env{ nullptr };
        winrt::Windows::UI::Core::CoreWindow _coreWindow{ nullptr };
        winrt::Windows::Foundation::Numerics::float2 _windowSize;
        std::atomic<bool> _isShuttingDown{ false };
        Babylon::JsRuntime* _jsRuntime{ nullptr };
        std::unique_ptr<Babylon::Graphics> _graphics{ nullptr };
        Babylon::Plugins::NativeInput* _nativeInput{ nullptr };

        std::atomic<bool> _initialized{ false };
        std::atomic<bool> _initializationSucceeded{ true };
        std::mutex _initializedPromiseLock;
        std::vector<winrt::Microsoft::ReactNative::ReactPromise<bool>> _initializedPromises{};

        Babylon::JsRuntime::DispatchFunctionT CreateJsRuntimeDispatcher()
        {
            return [weakThis{ this->weak_from_this() }](std::function<void(Napi::Env)> func)
            {
                if (auto trueThis = weakThis.lock())
                {
                    // We need to avoid synchronous execution or things will start to break down
                    // So we Post to the dispatcher instead of using ExcecuteJSI, which supports synchronous execution
                    trueThis->_jsDispatcher.Post([weakThis, func{ std::move(func) }]() {
                        if (auto trueThis = weakThis.lock())
                        {
                            auto jsiRuntime = *JsiAbiRuntime::GetOrCreate(trueThis->_reactContext.Handle().JSRuntime().as<JsiRuntime>());

                            try
                            {
                                // If JS engine shutdown is in progress, don't dispatch any new work.
                                if (!trueThis->_isShuttingDown)
                                {
                                    func(trueThis->_env);
                                }
                            }
                            catch (...)
                            {
                                auto ex{ std::current_exception() };
                                auto setImmediate{ jsiRuntime.global().getPropertyAsFunction(jsiRuntime, "setImmediate") };
                                auto throwFunc{ facebook::jsi::Function::createFromHostFunction(jsiRuntime, facebook::jsi::PropNameID::forAscii(jsiRuntime, "throwFunc"), 0,
                                    [ex](facebook::jsi::Runtime&, const facebook::jsi::Value&, const facebook::jsi::Value*, size_t) -> facebook::jsi::Value
                                    {
                                       std::rethrow_exception(ex);
                                    }) };
                                setImmediate.call(jsiRuntime, { std::move(throwFunc) });
                            }
                        }
                    });
                }
            };
        }

        void SetupBabylonNative(facebook::jsi::Runtime& jsiRuntime)
        {
            _env = Napi::Attach<facebook::jsi::Runtime&>(jsiRuntime);

            _jsRuntime = &Babylon::JsRuntime::CreateForJavaScript(_env, CreateJsRuntimeDispatcher());

            void* windowPtr{ nullptr };
            copy_to_abi(_coreWindow, windowPtr);
            auto width = static_cast<size_t>(_windowSize.x);
            auto height = static_cast<size_t>(_windowSize.y);
            _graphics = Babylon::Graphics::CreateGraphics(windowPtr, width, height);
            _graphics->AddToJavaScript(_env);

            // Populate polyfills
            Babylon::Polyfills::Window::Initialize(_env);
            Babylon::Polyfills::XMLHttpRequest::Initialize(_env);
            Babylon::Polyfills::Console::Initialize(_env, [](const char* message, auto)
            {
                OutputDebugStringA(message);
            });

            // Populate plugins
            Babylon::Plugins::NativeEngine::Initialize(_env, true);
            Babylon::Plugins::NativeXr::Initialize(_env);

            // TODO send _nativeInput input events
            _nativeInput = &Babylon::Plugins::NativeInput::CreateForJavaScript(_env);

            _initializationSucceeded = true;
            _initialized = true;
            {
                std::lock_guard<std::mutex> lock(_initializedPromiseLock);
                for (const auto& promise : _initializedPromises)
                {
                    promise.Resolve(_initializationSucceeded);
                }
                _initializedPromises.clear();
            }
        }
    };
} // namespace winrt::BabylonNative::implementation
