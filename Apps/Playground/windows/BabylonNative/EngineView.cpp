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

namespace winrt::BabylonNative::implementation {
    std::atomic<bool> EngineView::s_initialized{ false };
    std::atomic<bool> EngineView::s_initializationSucceeded{ true };
    std::mutex EngineView::s_initializedPromiseLock{};
    std::vector<ReactPromise<bool>> EngineView::s_initializedPromises{};

     EngineView::EngineView() {}

    // IViewManager
    hstring EngineView::Name() noexcept {
        return L"EngineView";
    }

    FrameworkElement EngineView::CreateView() noexcept {
        assert(!s_initialized);

        const auto& view = SwapChainPanel();
        copy_to_abi(view, _swapChainPanel);
        view.SizeChanged({ this, &EngineView::OnSizeChanged });
        //_swapChainPanelWidth = static_cast<size_t>(view.Width());
        //_swapChainPanelHeight = static_cast<size_t>(view.Height());

        if (!s_initialized)
        {
            // TODO add more robust state logic
            // TODO use shared_ptr/weak_ptr to this module compared to handing around references
            // modules are kept around by react native, this module can be killed
            winrt::Microsoft::ReactNative::ExecuteJsi(_reactContext, [weakThis{ this->get_weak() }](facebook::jsi::Runtime& jsiRuntime) {
                if (auto trueThis = weakThis.get())
                {
                    trueThis->SetupBabylonNative(jsiRuntime);
                }
            });
        }

        return view;
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

            _graphics = Babylon::Graphics::CreateGraphics(_swapChainPanel, _swapChainPanelWidth, _swapChainPanelHeight);
            _graphics->AddToJavaScript(_env);
            _graphics->UpdateWindow(_swapChainPanel);
            _graphics->UpdateWindowType(reinterpret_cast<void*>(2));

            // Populate polyfills
            Babylon::Polyfills::Window::Initialize(_env);
            Babylon::Polyfills::XMLHttpRequest::Initialize(_env);
            Babylon::Polyfills::Console::Initialize(_env, [](const char* message, auto)
            {
                OutputDebugStringA(message);
            });

            // Populate plugins
            // Note: we need to prevent automatic rendering or we'll attempt to setup the SwapChainPanel on the js thread when it needs to occur on the UI thread
            Babylon::Plugins::NativeEngine::Initialize(_env, false);
            Babylon::Plugins::NativeXr::Initialize(_env);

            // TODO send _nativeInput input events
            _nativeInput = &Babylon::Plugins::NativeInput::CreateForJavaScript(_env);

            s_initialized = true;
            s_initializationSucceeded = true;
            {
                std::lock_guard<std::mutex> lock(s_initializedPromiseLock);
                for (const auto& promise : s_initializedPromises)
                {
                    promise.Resolve(s_initializationSucceeded);
                }
                s_initializedPromises.clear();
            }
        }
    }

    void EngineView::OnSizeChanged(IInspectable const& /*sender*/, SizeChangedEventArgs const& args)
    {
        _swapChainPanelWidth = args.NewSize().Width;
        _swapChainPanelHeight = args.NewSize().Height;
        if (s_initialized)
        {
            _jsDispatcher.Post([weakThis{ this->get_weak() }]{
                if (auto trueThis = weakThis.get())
                {
                    trueThis->_graphics->UpdateSize(trueThis->_swapChainPanelWidth, trueThis->_swapChainPanelHeight);
                }
                });
        }
    }
} // namespace winrt::BabylonNative::implementation
