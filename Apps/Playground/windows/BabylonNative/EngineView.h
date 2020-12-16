#pragma once
#include "NativeModules.h"
#include "winrt/Microsoft.ReactNative.h"
#include "JSI/jsi.h"

namespace winrt::BabylonNative::implementation {

    struct EngineView
        : winrt::implements<
        EngineView,
        winrt::Microsoft::ReactNative::IViewManager,
        winrt::Microsoft::ReactNative::IViewManagerWithReactContext,
        winrt::Microsoft::ReactNative::IViewManagerWithNativeProperties,
        winrt::Microsoft::ReactNative::IViewManagerWithExportedEventTypeConstants>
        , std::enable_shared_from_this<EngineView> {
    public:
        EngineView();

        // IViewManager
        winrt::hstring Name() noexcept;
        winrt::Windows::UI::Xaml::FrameworkElement CreateView() noexcept;

        // IViewManagerWithReactContext
        winrt::Microsoft::ReactNative::IReactContext ReactContext() noexcept;
        void ReactContext(winrt::Microsoft::ReactNative::IReactContext reactContext) noexcept;

        // IViewManagerWithNativeProperties
        winrt::Windows::Foundation::Collections::
            IMapView<winrt::hstring, winrt::Microsoft::ReactNative::ViewManagerPropertyType>
            NativeProps() noexcept;
        void UpdateProperties(
            winrt::Windows::UI::Xaml::FrameworkElement const& view,
            winrt::Microsoft::ReactNative::IJSValueReader const& propertyMapReader) noexcept;

        // IViewManagerWithExportedEventTypeConstants
        winrt::Microsoft::ReactNative::ConstantProviderDelegate ExportedCustomBubblingEventTypeConstants() noexcept;
        winrt::Microsoft::ReactNative::ConstantProviderDelegate ExportedCustomDirectEventTypeConstants() noexcept;

        static void CompleteOnInitialization(const winrt::Microsoft::ReactNative::ReactPromise<bool>& result)
        {
            // TODO we should have these promsises queued per react context
            if (s_initialized)
            {
                result.Resolve(s_initializationSucceeded);
                return;
            }

            {
                std::lock_guard<std::mutex> lock(s_initializedPromiseLock);
                if (s_initialized)
                {
                    result.Resolve(s_initializationSucceeded);
                    return;
                }

                s_initializedPromises.push_back(result);
            }
        }

    private:
        Babylon::JsRuntime::DispatchFunctionT CreateJsRuntimeDispatcher();
        void SetupBabylonNative(facebook::jsi::Runtime& jsiRuntime);
        void OnSizeChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::SizeChangedEventArgs const& args);

        winrt::Microsoft::ReactNative::IReactContext _reactContext{ nullptr };
        winrt::Microsoft::ReactNative::IReactDispatcher _jsDispatcher;
        winrt::Microsoft::ReactNative::IReactDispatcher _uiDispatcher;

        Napi::Env _env{ nullptr };
        std::atomic<bool> _isShuttingDown{ false };
        Babylon::JsRuntime* _jsRuntime{ nullptr };
        std::unique_ptr<Babylon::Graphics> _graphics{ nullptr };
        Babylon::Plugins::NativeInput* _nativeInput{ nullptr };
        void* _swapChainPanel{ nullptr };
        size_t _swapChainPanelWidth{ 1 };
        size_t _swapChainPanelHeight{ 1 };

        static std::atomic<bool> s_initialized;
        static std::atomic<bool> s_initializationSucceeded;
        static std::mutex s_initializedPromiseLock;
        static std::vector<winrt::Microsoft::ReactNative::ReactPromise<bool>> s_initializedPromises;
    };

} // namespace winrt::BabylonNative::implementation

