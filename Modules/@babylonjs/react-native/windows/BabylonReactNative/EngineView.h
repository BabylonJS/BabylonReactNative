#pragma once
#include "NativeModules.h"
#include "winrt/Microsoft.ReactNative.h"
#include "JSI/jsi.h"
#include <unordered_set>

namespace winrt::BabylonReactNative::implementation {

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

        static void CompleteOnInitialization(const winrt::Microsoft::ReactNative::ReactPromise<bool>& result);
        static void Reset(const winrt::Microsoft::ReactNative::ReactPromise<bool>& result);

    private:
        Babylon::JsRuntime::DispatchFunctionT CreateJsRuntimeDispatcher();
        void SetupBabylonNative(facebook::jsi::Runtime& jsiRuntime);
        void CleanupBabylonNative(facebook::jsi::Runtime& jsiRuntime);
        void OnSizeChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::SizeChangedEventArgs const& args);
        void OnPointerPressed(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Input::PointerRoutedEventArgs const& args);
        void OnPointerMoved(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Input::PointerRoutedEventArgs const& args);
        void OnPointerReleased(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Input::PointerRoutedEventArgs const& args);
        void OnRendering();

        winrt::Microsoft::ReactNative::IReactContext _reactContext{ nullptr };
        winrt::Microsoft::ReactNative::IReactDispatcher _jsDispatcher;
        winrt::Microsoft::ReactNative::IReactDispatcher _uiDispatcher;

        winrt::Windows::UI::Xaml::Controls::SwapChainPanel _swapChainPanel{ nullptr };
        void* _swapChainPanelPtr{ nullptr };
        size_t _swapChainPanelWidth{ 1 };
        size_t _swapChainPanelHeight{ 1 };
        std::unordered_set<uint32_t> _pressedPointers{};

        Napi::Env _env{ nullptr };
        std::atomic<bool> _isShuttingDown{ false };
        Babylon::JsRuntime* _jsRuntime{ nullptr };
        std::unique_ptr<Babylon::Graphics> _graphics{ nullptr };
        std::atomic<bool> _rendering{ false };
        Babylon::Plugins::NativeInput* _nativeInput{ nullptr };

        static std::atomic<bool> s_initialized;
        static std::mutex s_initializedPromiseLock;
        static std::vector<winrt::Microsoft::ReactNative::ReactPromise<bool>> s_initializedPromises;
    };

} // namespace winrt::BabylonReactNative::implementation

