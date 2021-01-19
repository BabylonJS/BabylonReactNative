#include "pch.h"
#include "EngineViewManager.h"

#include "JSValueReader.h"
#include "JSValueXaml.h"

using namespace winrt::Microsoft::ReactNative;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;

using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Media;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Input;

namespace winrt::BabylonReactNative::implementation {
    EngineViewManager::EngineViewManager() {}

    // IViewManager
    hstring EngineViewManager::Name() noexcept {
        return L"EngineView";
    }

    FrameworkElement EngineViewManager::CreateView() noexcept {
        _swapChainPanel = SwapChainPanel();
        _swapChainPanelPtr = nullptr;
        copy_to_abi(_swapChainPanel, _swapChainPanelPtr);
        _revokerData.SizeChangedRevoker = _swapChainPanel.SizeChanged(winrt::auto_revoke, { this, &EngineViewManager::OnSizeChanged });
        _revokerData.PointerPressedRevoker = _swapChainPanel.PointerPressed(winrt::auto_revoke, { this, &EngineViewManager::OnPointerPressed });
        _revokerData.PointerMovedRevoker = _swapChainPanel.PointerMoved(winrt::auto_revoke, { this, &EngineViewManager::OnPointerMoved });
        _revokerData.PointerReleasedRevoker = _swapChainPanel.PointerReleased(winrt::auto_revoke, { this, &EngineViewManager::OnPointerReleased });

        if (_revokerData.RenderingToken)
        {
            CompositionTarget::Rendering(_revokerData.RenderingToken);
        }

        _revokerData.RenderingToken = CompositionTarget::Rendering([weakThis{ this->get_weak() }](auto const&, auto const&)
        {
            if (auto trueThis = weakThis.get())
            {
                trueThis->OnRendering();
            }
        });

        return _swapChainPanel;
    }

    // IViewManagerWithReactContext
    IReactContext EngineViewManager::ReactContext() noexcept {
        return _reactContext;
    }

    void EngineViewManager::ReactContext(IReactContext reactContext) noexcept {
        _reactContext = reactContext;
    }

    // IViewManagerWithNativeProperties
    IMapView<hstring, ViewManagerPropertyType> EngineViewManager::NativeProps() noexcept {
        auto nativeProps = winrt::single_threaded_map<hstring, ViewManagerPropertyType>();

        // TODO: Add properties as needed

        return nativeProps.GetView();
    }

    void EngineViewManager::UpdateProperties(
        FrameworkElement const& /*view*/,
        IJSValueReader const& /*propertyMapReader*/) noexcept {

        // TODO: Implement as needed
    }

    // IViewManagerWithExportedEventTypeConstants
    ConstantProviderDelegate EngineViewManager::ExportedCustomBubblingEventTypeConstants() noexcept {
        return nullptr;
    }

    ConstantProviderDelegate EngineViewManager::ExportedCustomDirectEventTypeConstants() noexcept {
        return [](winrt::Microsoft::ReactNative::IJSValueWriter const& constantWriter) {
            WriteCustomDirectEventTypeConstant(constantWriter, "onSnapshotDataReturned");
        };
    }

    void EngineViewManager::OnSizeChanged(IInspectable const& /*sender*/, SizeChangedEventArgs const& args)
    {
        const auto size = args.NewSize();
        _swapChainPanelWidth = static_cast<size_t>(size.Width);
        _swapChainPanelHeight = static_cast<size_t>(size.Height);

        // Use windowTypePtr == 2 for xaml swap chain panels
        auto windowTypePtr = reinterpret_cast<void*>(2);

        Babylon::UpdateView(_swapChainPanelPtr, _swapChainPanelWidth, _swapChainPanelHeight, windowTypePtr);
    }

    void EngineViewManager::OnPointerPressed(IInspectable const& /*sender*/, PointerRoutedEventArgs const& args)
    {
        const auto pointerId = args.Pointer().PointerId();
        const auto buttonId = 0; // Update as needed
        const auto point = args.GetCurrentPoint(_swapChainPanel);
        const uint32_t x = point.Position().X < 0 ? 0 : static_cast<uint32_t>(point.Position().X);
        const uint32_t y = point.Position().Y < 0 ? 0 : static_cast<uint32_t>(point.Position().Y);
        Babylon::SetPointerButtonState(pointerId, buttonId, true, x, y);
    }

    void EngineViewManager::OnPointerMoved(IInspectable const& /*sender*/, PointerRoutedEventArgs const& args)
    {
        const auto pointerId = args.Pointer().PointerId();
        const auto point = args.GetCurrentPoint(_swapChainPanel);
        const uint32_t x = point.Position().X < 0 ? 0 : static_cast<uint32_t>(point.Position().X);
        const uint32_t y = point.Position().Y < 0 ? 0 : static_cast<uint32_t>(point.Position().Y);
        Babylon::SetPointerPosition(pointerId, x, y);
    }

    void EngineViewManager::OnPointerReleased(IInspectable const& /*sender*/, PointerRoutedEventArgs const& args)
    {
        const auto pointerId = args.Pointer().PointerId();
        const auto buttonId = 0; // Update as needed
        const auto point = args.GetCurrentPoint(_swapChainPanel);
        const uint32_t x = point.Position().X < 0 ? 0 : static_cast<uint32_t>(point.Position().X);
        const uint32_t y = point.Position().Y < 0 ? 0 : static_cast<uint32_t>(point.Position().Y);
        Babylon::SetPointerButtonState(pointerId, buttonId, false, x, y);
    }

    void EngineViewManager::OnRendering()
    {
        Babylon::RenderView();
    }
} // namespace winrt::BabylonReactNative::implementation
