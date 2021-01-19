#pragma once
#include "NativeModules.h"
#include "winrt/Microsoft.ReactNative.h"
#include "JSI/jsi.h"
#include <unordered_set>

namespace winrt::BabylonReactNative::implementation {

    struct EngineViewManager
        : winrt::implements<
        EngineViewManager,
        winrt::Microsoft::ReactNative::IViewManager,
        winrt::Microsoft::ReactNative::IViewManagerWithReactContext,
        winrt::Microsoft::ReactNative::IViewManagerWithNativeProperties,
        winrt::Microsoft::ReactNative::IViewManagerWithExportedEventTypeConstants> {
    public:
        EngineViewManager();

        // IViewManager
        winrt::hstring Name() const noexcept;
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

    private:
        void OnSizeChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::SizeChangedEventArgs const& args);
        void OnPointerPressed(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Input::PointerRoutedEventArgs const& args);
        void OnPointerMoved(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Input::PointerRoutedEventArgs const& args);
        void OnPointerReleased(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Input::PointerRoutedEventArgs const& args);
        void OnRendering();

        winrt::Microsoft::ReactNative::IReactContext _reactContext{ nullptr };
        winrt::Windows::UI::Xaml::Controls::SwapChainPanel _swapChainPanel{ nullptr };
        size_t _swapChainPanelWidth{ 1 };
        size_t _swapChainPanelHeight{ 1 };

        struct RevokerData
        {
            winrt::Windows::UI::Xaml::FrameworkElement::SizeChanged_revoker SizeChangedRevoker{};
            winrt::Windows::UI::Xaml::FrameworkElement::PointerPressed_revoker PointerPressedRevoker{};
            winrt::Windows::UI::Xaml::FrameworkElement::PointerMoved_revoker PointerMovedRevoker{};
            winrt::Windows::UI::Xaml::FrameworkElement::PointerReleased_revoker PointerReleasedRevoker{};
            winrt::Windows::UI::Xaml::Media::CompositionTarget::Rendering_revoker RenderingRevoker{};
        };
        RevokerData _revokerData{};
    };

} // namespace winrt::BabylonReactNative::implementation

