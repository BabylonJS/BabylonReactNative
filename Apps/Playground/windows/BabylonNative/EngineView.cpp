#include "pch.h"
#include <winrt/Windows.UI.Xaml.Media.h>

#include "EngineView.h"
#include "JSValueReader.h"
#include "JSValueXaml.h"
#include "NativeModules.h"

using namespace winrt::Microsoft::ReactNative;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;

using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Media;
using namespace winrt::Windows::UI::Xaml::Controls;

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

        // TODO: unclear how to declare onSnapshotDataReturned
        // TODO: unclear how to declare camera
        // TODO: unclear how to declare onInitialized callback
        nativeProps.Insert(L"displayFrameRate", ViewManagerPropertyType::Boolean);

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
