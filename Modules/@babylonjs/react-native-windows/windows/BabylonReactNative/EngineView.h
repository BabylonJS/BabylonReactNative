#pragma once
#include "EngineView.g.h"
#include <unordered_set>
#include <concrt.h>

namespace winrt::BabylonReactNative::implementation {
    struct EngineView : EngineViewT<EngineView>
    {
    public:
        EngineView();
        ~EngineView() noexcept;

        void Reset();

    private:
        void OnSizeChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::SizeChangedEventArgs const& args);
        void OnPointerPressed(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Core::PointerEventArgs const& args);
        void OnPointerMoved(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Core::PointerEventArgs const& args);
        void OnPointerReleased(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Core::PointerEventArgs const& args);
        void OnCompositionScaleChanged(Windows::UI::Xaml::Controls::SwapChainPanel const& sender, Windows::Foundation::IInspectable const& object);
        void OnSuspending();
        void OnResuming();
        void OnDeviceLost();
        void OnRendering();
        uint32_t GetButtonId(winrt::Windows::Devices::Input::PointerDeviceType deviceType, winrt::Windows::UI::Input::PointerPointProperties properties);

        void RegisterInput();
        void RegisterRender();

        void CreateDeviceResources();
        void CreateSizeDependentResources();
        

		float _renderTargetHeight;
		float _renderTargetWidth;
              
        float _compositionScaleX{ 1.0f };
        float _compositionScaleY{ 1.0f };
              
        float _height{ 1.0f };
        float _width{ 1.0f };

        winrt::Windows::Foundation::IAsyncAction _inputLoopWorker{};
        std::unordered_set<uint32_t> _pressedMouseButtons{};
        winrt::Windows::UI::Core::CoreIndependentInputSource _inputSource{ nullptr };
        std::unordered_set<uint32_t> _pressedPointers{};

        struct RevokerData
        {
            winrt::Windows::UI::Xaml::FrameworkElement::SizeChanged_revoker SizeChangedRevoker{};
            winrt::Windows::UI::Xaml::IApplication::Suspending_revoker SuspendingRevoker{};
            winrt::Windows::UI::Xaml::IApplication::Resuming_revoker ResumingRevoker{};
            winrt::Windows::UI::Core::CoreIndependentInputSource::PointerPressed_revoker PointerPressedRevoker{};
            winrt::Windows::UI::Core::CoreIndependentInputSource::PointerMoved_revoker PointerMovedRevoker{};
            winrt::Windows::UI::Core::CoreIndependentInputSource::PointerReleased_revoker PointerReleasedRevoker{};
        };
        RevokerData _revokerData{};

        Concurrency::critical_section _criticalSection;
		::Microsoft::WRL::ComPtr<ID3D11Device1> _d3dDevice;
		::Microsoft::WRL::ComPtr<ID3D11DeviceContext1> _d3dContext;
		::Microsoft::WRL::ComPtr<IDXGISwapChain2>_swapChain;
		::Microsoft::WRL::ComPtr <ID3D11RenderTargetView> _backBufferPtr;
        ::Microsoft::WRL::ComPtr<IDXGIOutput> _dxgiOutput;
        winrt::Windows::Foundation::IAsyncAction _renderLoopWorker{};
        std::future<void> _resetView;
    };
}

namespace winrt::BabylonReactNative::factory_implementation {

struct EngineView : EngineViewT<EngineView, implementation::EngineView> {};

} // namespace winrt::BabylonReactNative::factory_implementation