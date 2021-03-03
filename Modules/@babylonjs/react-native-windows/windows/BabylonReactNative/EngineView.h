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
        
        float _renderTargetHeight{ 1.0f };
        float _renderTargetWidth{ 1.0f };
              
        float _compositionScaleX{ 1.0f };
        float _compositionScaleY{ 1.0f };
              
        float _height{ 1.0f };
        float _width{ 1.0f };

        winrt::Windows::Foundation::IAsyncAction _inputLoopWorker{};
        std::unordered_set<uint32_t> _pressedMouseButtons{};

        struct RevokerData
        {
            winrt::Windows::UI::Xaml::FrameworkElement::SizeChanged_revoker SizeChangedRevoker{};
            winrt::Windows::UI::Xaml::IApplication::Suspending_revoker SuspendingRevoker{};
            winrt::Windows::UI::Xaml::IApplication::Resuming_revoker ResumingRevoker{};
        };
        RevokerData _revokerData{};

        DXGI_SAMPLE_DESC _sampleDesc{ 1, 0 };
        ::Microsoft::WRL::ComPtr<IDXGISwapChain2>_swapChain;
        ::Microsoft::WRL::ComPtr <ID3D11RenderTargetView> _backBufferPtr;
        ::Microsoft::WRL::ComPtr<IDXGIOutput> _dxgiOutput;
        winrt::Windows::Foundation::IAsyncAction _renderLoopWorker{};
    };
}

namespace winrt::BabylonReactNative::factory_implementation {

struct EngineView : EngineViewT<EngineView, implementation::EngineView> {};

} // namespace winrt::BabylonReactNative::factory_implementation