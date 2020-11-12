#include "pch.h"
#include <winrt/Windows.UI.Xaml.Media.h>

#include "EngineView.h"
#include "JSValueReader.h"
#include "JSValueXaml.h"
#include "NativeModules.h"

using namespace winrt;
using namespace Microsoft::ReactNative;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;

using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Controls;

namespace winrt::BabylonNative::implementation {
    typedef InkCanvas ControlType;

    EngineView::EngineView() {}

    // IViewManager
    hstring EngineView::Name() noexcept {
        return L"EngineView";
    }

    FrameworkElement EngineView::CreateView() noexcept {
        auto const& view = ControlType();
        return view;
    }

    // IViewManagerWithReactContext
    IReactContext EngineView::ReactContext() noexcept {
        return m_reactContext;
    }

    void EngineView::ReactContext(IReactContext reactContext) noexcept {
        m_reactContext = reactContext;
    }

    // IViewManagerWithNativeProperties
    IMapView<hstring, ViewManagerPropertyType> EngineView::NativeProps() noexcept {
        auto nativeProps = winrt::single_threaded_map<hstring, ViewManagerPropertyType>();

        // TODO
        nativeProps.Insert(L"label", ViewManagerPropertyType::String);
        nativeProps.Insert(L"color", ViewManagerPropertyType::Color);
        nativeProps.Insert(L"backgroundColor", ViewManagerPropertyType::Color);

        return nativeProps.GetView();
    }

    void EngineView::UpdateProperties(
        FrameworkElement const& view,
        IJSValueReader const& propertyMapReader) noexcept {
        if (auto control = view.try_as<ControlType>()) {
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

} // namespace winrt::BabylonNative::implementation
