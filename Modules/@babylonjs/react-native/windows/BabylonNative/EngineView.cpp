#include "pch.h"
#include "EngineView.h"
#include "JSI/JsiApi.h"

#include "JSValueReader.h"
#include "JSValueXaml.h"
#include "BabylonModule.h"

using namespace winrt::Microsoft::ReactNative;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;

using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Media;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Input;

namespace winrt::BabylonNative::implementation {
    std::atomic<bool> EngineView::s_initialized{ false };
    std::mutex EngineView::s_initializedPromiseLock{};
    std::vector<ReactPromise<bool>> EngineView::s_initializedPromises{};

    EngineView::EngineView() {}

    // IViewManager
    hstring EngineView::Name() noexcept {
        return L"EngineView";
    }

    FrameworkElement EngineView::CreateView() noexcept {
        // TODO: this will get called repeatedly when toggling the engine view

        _swapChainPanel = SwapChainPanel();
        copy_to_abi(_swapChainPanel, _swapChainPanelPtr);
        _swapChainPanel.SizeChanged({ this, &EngineView::OnSizeChanged });
        _swapChainPanel.PointerPressed({ this, &EngineView::OnPointerPressed });
        _swapChainPanel.PointerMoved({ this, &EngineView::OnPointerMoved });
        _swapChainPanel.PointerReleased({ this, &EngineView::OnPointerReleased });

        CompositionTarget::Rendering([weakThis{ this->get_weak() }](auto const&, auto const&)
        {
            if (auto trueThis = weakThis.get())
            {
                trueThis->OnRendering();
            }
        });

        winrt::Microsoft::ReactNative::ExecuteJsi(_reactContext, [weakThis{ this->get_weak() }](facebook::jsi::Runtime& jsiRuntime) {
            if (auto trueThis = weakThis.get())
            {
                if (trueThis->s_initialized)
                {
                    trueThis->CleanupBabylonNative(jsiRuntime);
                }

                trueThis->SetupBabylonNative(jsiRuntime);
            }
        });

        return _swapChainPanel;
    }

    // IViewManagerWithReactContext
    IReactContext EngineView::ReactContext() noexcept {
        return _reactContext;
    }

    void EngineView::ReactContext(IReactContext reactContext) noexcept {
        _reactContext = reactContext;
        _jsDispatcher = _reactContext.JSDispatcher();
        _uiDispatcher = _reactContext.UIDispatcher();
    }

    // IViewManagerWithNativeProperties
    IMapView<hstring, ViewManagerPropertyType> EngineView::NativeProps() noexcept {
        auto nativeProps = winrt::single_threaded_map<hstring, ViewManagerPropertyType>();

        // TODO remove
        nativeProps.Insert(L"label", ViewManagerPropertyType::String);
        nativeProps.Insert(L"color", ViewManagerPropertyType::Color);
        nativeProps.Insert(L"backgroundColor", ViewManagerPropertyType::Color);

        // TODO: unclear how to declare onSnapshotDataReturned
        // TODO: unclear how to declare camera
        // TODO: unclear how to declare onInitialized callback
        nativeProps.Insert(L"displayFrameRate", ViewManagerPropertyType::Boolean);

        return nativeProps.GetView();
    }

    void EngineView::UpdateProperties(
        FrameworkElement const& view,
        IJSValueReader const& propertyMapReader) noexcept {
        if (auto control = view.try_as<SwapChainPanel>()) {
            JSValueObject propertyMap = JSValueObject::ReadFrom(propertyMapReader);
            for (const auto& [name, property] : propertyMap) {
                // TODO
            }
        }
    }

    // IViewManagerWithExportedEventTypeConstants
    ConstantProviderDelegate EngineView::ExportedCustomBubblingEventTypeConstants() noexcept {
        return nullptr;
    }

    ConstantProviderDelegate EngineView::ExportedCustomDirectEventTypeConstants() noexcept {
        return [](winrt::Microsoft::ReactNative::IJSValueWriter const& constantWriter) {
            constantWriter.WritePropertyName(L"onSnapshotDataReturned");
            constantWriter.WriteObjectBegin();
            WriteProperty(constantWriter, L"registrationName", L"onSnapshotDataReturned");
            constantWriter.WriteObjectEnd();
        };
    }

    Babylon::JsRuntime::DispatchFunctionT EngineView::CreateJsRuntimeDispatcher()
    {
        return[weakThis{ this->get_weak() }](std::function<void(Napi::Env)> func)
        {
            if (auto trueThis = weakThis.get())
            {
                // We need to avoid synchronous execution or things will start to break down
                // So we Post to the dispatcher instead of using ExcecuteJSI, which supports synchronous execution
                trueThis->_jsDispatcher.Post([weakThis, func{ std::move(func) }]() {
                    if (auto trueThis = weakThis.get())
                    {
                        auto& jsiRuntime = winrt::Microsoft::ReactNative::GetOrCreateContextRuntime(trueThis->_reactContext);
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

    void EngineView::SetupBabylonNative(facebook::jsi::Runtime& jsiRuntime)
    {
        if (!s_initialized)
        {
            _env = Napi::Attach<facebook::jsi::Runtime&>(jsiRuntime);

            _jsRuntime = &Babylon::JsRuntime::CreateForJavaScript(_env, CreateJsRuntimeDispatcher());

            // Use windowTypePtr == 2 for xaml swap chain panels
            void* windowTypePtr = reinterpret_cast<void*>(2);
            _graphics = Babylon::Graphics::CreateGraphics(_swapChainPanelPtr, windowTypePtr, _swapChainPanelWidth, _swapChainPanelHeight);
            _graphics->AddToJavaScript(_env);
            _graphics->UpdateWindow(_swapChainPanelPtr, windowTypePtr);
            _rendering = true;

            // Populate polyfills
            Babylon::Polyfills::Window::Initialize(_env);
            Babylon::Polyfills::XMLHttpRequest::Initialize(_env);
            // Note: we are using react-native-windows console polyfill that supports assert

            // Populate plugins
            Babylon::Plugins::NativeEngine::Initialize(_env, false);
            Babylon::Plugins::NativeXr::Initialize(_env);
            _nativeInput = &Babylon::Plugins::NativeInput::CreateForJavaScript(_env);

            s_initialized = true;
            {
                std::lock_guard<std::mutex> lock(s_initializedPromiseLock);
                for (const auto& promise : s_initializedPromises)
                {
                    promise.Resolve(true);
                }
                s_initializedPromises.clear();
            }
        }
    }

    void EngineView::CleanupBabylonNative(facebook::jsi::Runtime& /*jsiRuntime*/)
    {
        // TODO: clean up everything
        s_initialized = false;
    }

    void EngineView::OnSizeChanged(IInspectable const& /*sender*/, SizeChangedEventArgs const& args)
    {
        const auto size = args.NewSize();
        _swapChainPanelWidth = static_cast<size_t>(size.Width);
        _swapChainPanelHeight = static_cast<size_t>(size.Height);
        if (_graphics)
        {
            _graphics->UpdateSize(_swapChainPanelWidth, _swapChainPanelHeight);
        }
    }

    void EngineView::OnPointerPressed(IInspectable const& /*sender*/, PointerRoutedEventArgs const& args)
    {
        if (_nativeInput &&
            _swapChainPanel)
        {
            const auto pointerPoint = args.GetCurrentPoint(_swapChainPanel);
            const auto x = pointerPoint.Position().X < 0 ? 0 : static_cast<uint32_t>(pointerPoint.Position().X);
            const auto y = pointerPoint.Position().Y < 0 ? 0 : static_cast<uint32_t>(pointerPoint.Position().Y);
            _nativeInput->PointerDown(pointerPoint.PointerId(), 0 /*TODO: Implement buttonId as needed*/, x, y);
            _pressedPointers.insert(pointerPoint.PointerId());
        }
    }

    void EngineView::OnPointerMoved(IInspectable const& /*sender*/, PointerRoutedEventArgs const& args)
    {
        if (_nativeInput &&
            _swapChainPanel)
        {
            const auto pointerPoint = args.GetCurrentPoint(_swapChainPanel);
            if (_pressedPointers.count(pointerPoint.PointerId()) > 0)
            {
                const auto x = pointerPoint.Position().X < 0 ? 0 : static_cast<uint32_t>(pointerPoint.Position().X);
                const auto y = pointerPoint.Position().Y < 0 ? 0 : static_cast<uint32_t>(pointerPoint.Position().Y);
                _nativeInput->PointerMove(pointerPoint.PointerId(), x, y);
            }
        }
    }

    void EngineView::OnPointerReleased(IInspectable const& /*sender*/, PointerRoutedEventArgs const& args)
    {
        if (_nativeInput &&
            _swapChainPanel)
        {
            const auto pointerPoint = args.GetCurrentPoint(_swapChainPanel);
            const auto x = pointerPoint.Position().X < 0 ? 0 : static_cast<uint32_t>(pointerPoint.Position().X);
            const auto y = pointerPoint.Position().Y < 0 ? 0 : static_cast<uint32_t>(pointerPoint.Position().Y);
            _nativeInput->PointerUp(pointerPoint.PointerId(), 0 /*TODO: Implement buttonId as needed*/, x, y);
            _pressedPointers.erase(pointerPoint.PointerId());
        }
    }

    void EngineView::OnRendering()
    {
        if (_rendering &&
            _graphics != nullptr)
        {
            _graphics->RenderCurrentFrame();
        }
    }

    void EngineView::CompleteOnInitialization(const ReactPromise<bool>& result)
    {
        if (s_initialized)
        {
            result.Resolve(true);
            return;
        }

        std::lock_guard<std::mutex> lock(s_initializedPromiseLock);
        if (s_initialized)
        {
            result.Resolve(true);
            return;
        }

        s_initializedPromises.push_back(result);
    }

    void EngineView::Reset(const ReactPromise<bool>& result)
    {
        // TODO
        result.Resolve(false);
    }
} // namespace winrt::BabylonNative::implementation
