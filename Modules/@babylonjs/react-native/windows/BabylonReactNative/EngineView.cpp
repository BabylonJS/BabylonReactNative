#include "pch.h"
#include "EngineView.h"
#include "EngineView.g.cpp"

using namespace winrt::Windows::Devices::Input;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::System::Threading;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Input;
using namespace winrt::Windows::UI::Xaml::Media;
using namespace winrt::Windows::UI::Xaml::Controls;

namespace winrt::BabylonReactNative::implementation {
    EngineView::EngineView() {

        _revokerData.SizeChangedRevoker = SizeChanged(winrt::auto_revoke, { this, &EngineView::OnSizeChanged });

        WorkItemHandler workItemHandler([weakThis{ this->get_weak() }](IAsyncAction const& /* action */)
        {
            if (auto trueThis = weakThis.get())
            {
                auto deviceTypes = static_cast<CoreInputDeviceTypes>(
                    static_cast<uint32_t>(Windows::UI::Core::CoreInputDeviceTypes::Mouse) |
                    static_cast<uint32_t>(Windows::UI::Core::CoreInputDeviceTypes::Touch) |
                    static_cast<uint32_t>(Windows::UI::Core::CoreInputDeviceTypes::Pen));
                auto coreInput = trueThis->CreateCoreIndependentInputSource(deviceTypes);

                trueThis->_revokerData.PointerPressedRevoker = coreInput.PointerPressed(winrt::auto_revoke, { trueThis.get(), &EngineView::OnPointerPressed });
                trueThis->_revokerData.PointerMovedRevoker = coreInput.PointerMoved(winrt::auto_revoke, { trueThis.get(), &EngineView::OnPointerMoved });
                trueThis->_revokerData.PointerReleasedRevoker = coreInput.PointerReleased(winrt::auto_revoke, { trueThis.get(), &EngineView::OnPointerReleased });

                coreInput.Dispatcher().ProcessEvents(Windows::UI::Core::CoreProcessEventsOption::ProcessUntilQuit);
            }
        });

        _inputLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);

        _revokerData.RenderingRevoker = CompositionTarget::Rendering(winrt::auto_revoke, [weakThis{ this->get_weak() }](auto const&, auto const&)
        {
            if (auto trueThis = weakThis.get())
            {
                trueThis->OnRendering();
            }
        });
    }

    void EngineView::OnSizeChanged(IInspectable const& /*sender*/, SizeChangedEventArgs const& args)
    {
        const auto size = args.NewSize();
        _width = static_cast<size_t>(size.Width);
        _height = static_cast<size_t>(size.Height);

        // Use windowTypePtr == 2 for xaml swap chain panels
        auto windowTypePtr = reinterpret_cast<void*>(2);
        auto windowPtr = get_abi(static_cast<winrt::Windows::UI::Xaml::Controls::SwapChainPanel>(*this));
        Babylon::UpdateView(windowPtr, _width, _height, windowTypePtr);
    }

    void EngineView::OnPointerPressed(IInspectable const& /*sender*/, PointerEventArgs const& args)
    {
        const auto point = args.CurrentPoint();
        const auto pointerId = point.PointerId();
        _pressedPointers.insert(pointerId);

        const auto buttonId = 0; // Update as needed
        const auto position = point.Position();
        const bool isMouse = point.PointerDevice().PointerDeviceType() == PointerDeviceType::Mouse;
        const uint32_t x = position.X < 0 ? 0 : static_cast<uint32_t>(position.X);
        const uint32_t y = position.Y < 0 ? 0 : static_cast<uint32_t>(position.Y);
        Babylon::SetPointerButtonState(pointerId, buttonId, true, x, y, isMouse);
    }

    void EngineView::OnPointerMoved(IInspectable const& /*sender*/, PointerEventArgs const& args)
    {
        const auto point = args.CurrentPoint();
        const auto pointerId = point.PointerId();

        if (_pressedPointers.count(pointerId) > 0)
        {
            const auto position = point.Position();
            const bool isMouse = point.PointerDevice().PointerDeviceType() == PointerDeviceType::Mouse;
            const uint32_t x = position.X < 0 ? 0 : static_cast<uint32_t>(position.X);
            const uint32_t y = position.Y < 0 ? 0 : static_cast<uint32_t>(position.Y);
            Babylon::SetPointerPosition(pointerId, x, y, isMouse);
        }
    }

    void EngineView::OnPointerReleased(IInspectable const& /*sender*/, PointerEventArgs const& args)
    {
        const auto point = args.CurrentPoint();
        const auto pointerId = point.PointerId();
        _pressedPointers.erase(pointerId);

        const auto buttonId = 0; // Update as needed
        const auto position = point.Position();
        const bool isMouse = point.PointerDevice().PointerDeviceType() == PointerDeviceType::Mouse;
        const uint32_t x = position.X < 0 ? 0 : static_cast<uint32_t>(position.X);
        const uint32_t y = position.Y < 0 ? 0 : static_cast<uint32_t>(position.Y);
        Babylon::SetPointerButtonState(pointerId, buttonId, false, x, y, isMouse);
    }

    void EngineView::OnRendering()
    {
        Babylon::RenderView();
    }
}