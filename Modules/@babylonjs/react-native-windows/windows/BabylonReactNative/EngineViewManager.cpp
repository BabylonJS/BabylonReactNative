#include "pch.h"
#include "EngineViewManager.h"

#include "JSValueReader.h"
#include "JSValueXaml.h"

using namespace winrt::Microsoft::ReactNative;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;

using namespace winrt::Windows::UI::Xaml;

namespace winrt::BabylonReactNative::implementation {
    EngineViewManager::EngineViewManager() {}

    // IViewManager
    hstring EngineViewManager::Name() const noexcept {
        return L"EngineView";
    }

    FrameworkElement EngineViewManager::CreateView() noexcept {
        return make<EngineView>();
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
} // namespace winrt::BabylonReactNative::implementation
