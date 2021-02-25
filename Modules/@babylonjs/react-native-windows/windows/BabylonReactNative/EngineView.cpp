#include "pch.h"
#include "EngineView.h"
#include "EngineView.g.cpp"


namespace winrt::BabylonReactNative::implementation {
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Devices::Input;
    using namespace winrt::Windows::System::Threading;
    using namespace winrt::Windows::UI::Core;
    using namespace winrt::Windows::UI::Input;
    using namespace winrt::Windows::UI::Xaml;
    using namespace winrt::Windows::UI::Xaml::Input;
    using namespace winrt::Windows::UI::Xaml::Media;
    using namespace winrt::Windows::UI::Xaml::Controls;
    using namespace winrt::Windows::ApplicationModel;
    using namespace Concurrency;

    static EngineView* s_engineView{ nullptr };	// Only single view supported
    static ID3D11Device1* s_d3dDevice{ nullptr };	// It will be released on bgfx.
    static ::Microsoft::WRL::ComPtr<ID3D11DeviceContext1> s_d3dContext;
    static Concurrency::critical_section s_criticalSection;
    
    EngineView::EngineView() {
        if (s_engineView) return;

        _revokerData.SizeChangedRevoker = SizeChanged(winrt::auto_revoke, { this, &EngineView::OnSizeChanged });
        _revokerData.SuspendingRevoker = Application::Current().Suspending(winrt::auto_revoke, [weakThis{ get_weak() }](
            auto const& /*sender*/,
            auto const& /*args*/)
        {
            if (auto trueThis = weakThis.get())
                trueThis->OnSuspending();
        });

        _revokerData.ResumingRevoker = Application::Current().Resuming(winrt::auto_revoke, [weakThis{ get_weak() }](
            auto const& /*sender*/,
            auto const& /*args*/)
        {
            if (auto trueThis = weakThis.get())
            {
                if (CoreWindow::GetForCurrentThread().Visible())
                    trueThis->OnResuming();
            }
        });
        
        auto swapChainPanel = static_cast<SwapChainPanel>(*this);
        swapChainPanel.CompositionScaleChanged([this, weak_this{ get_weak() }]
        (SwapChainPanel const& sender, IInspectable const& object)
        {
            if (auto trueThis{ weak_this.get() })
                trueThis->OnCompositionScaleChanged(sender, object);
        });

        RegisterInput();
        RegisterRender();

        s_engineView = this;
    }

    EngineView::~EngineView()
    {
        if(_inputLoopWorker)
            _inputLoopWorker.Cancel();

        if(_renderLoopWorker)
            _renderLoopWorker.Cancel();
        
        s_criticalSection.lock();
        _dxgiOutput = nullptr;
        _backBufferPtr = nullptr;
        _swapChain = nullptr;
        s_criticalSection.unlock();

        s_engineView = nullptr;
    }

    void EngineView::RegisterInput()
    {
        WorkItemHandler workItemHandler([this](IAsyncAction const& /* action */) mutable
        {
            auto deviceTypes = static_cast<CoreInputDeviceTypes>(
                static_cast<uint32_t>(Windows::UI::Core::CoreInputDeviceTypes::Mouse) |
                static_cast<uint32_t>(Windows::UI::Core::CoreInputDeviceTypes::Touch) |
                static_cast<uint32_t>(Windows::UI::Core::CoreInputDeviceTypes::Pen));
            auto coreInput = CreateCoreIndependentInputSource(deviceTypes);

            auto PointerPressedRevoker = coreInput.PointerPressed(winrt::auto_revoke, { this, &EngineView::OnPointerPressed });
            auto PointerMovedRevoker = coreInput.PointerMoved(winrt::auto_revoke, { this, &EngineView::OnPointerMoved });
            auto PointerReleasedRevoker = coreInput.PointerReleased(winrt::auto_revoke, { this, &EngineView::OnPointerReleased });

            coreInput.Dispatcher().ProcessEvents(Windows::UI::Core::CoreProcessEventsOption::ProcessUntilQuit);
        });

        // TODO: move to std::thread compared to consuming ThreadPool resources once engine lifecycle bugs are addressed and EngineView's destructor can be successfully invoked.
        _inputLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
    }

    void EngineView::RegisterRender()
    {
        s_criticalSection.lock();
        CreateDeviceResources();
        CreateSizeDependentResources();
        s_criticalSection.unlock();
        
        // Calculate the updated frame and render once per vertical blanking interval
        WorkItemHandler workItemHandler([this](IAsyncAction const& action) mutable
        {
            while (action.Status() == AsyncStatus::Started)
                OnRendering();
        });

        _renderLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
    }

    void EngineView::OnSizeChanged(IInspectable const& /*sender*/, SizeChangedEventArgs const& args)
    {
        critical_section::scoped_lock lock(s_criticalSection);

        const auto size = args.NewSize();
        _width = std::max(size.Width, 1.0f);
        _height = std::max(size.Height, 1.0f);

        CreateSizeDependentResources();

        auto swapChainPanel = static_cast<SwapChainPanel>(*this);
        auto swapChainPanelNative = swapChainPanel.as<ISwapChainPanelNative>().get();
        swapChainPanelNative->SetSwapChain(_swapChain.Get());
    }

    void EngineView::OnPointerPressed(IInspectable const& /*sender*/, PointerEventArgs const& args)
    {
        const auto point = args.CurrentPoint();
        const auto properties = point.Properties();
        const auto deviceType = point.PointerDevice().PointerDeviceType();
        const auto position = point.Position();
        const uint32_t x = position.X < 0 ? 0 : static_cast<uint32_t>(position.X);
        const uint32_t y = position.Y < 0 ? 0 : static_cast<uint32_t>(position.Y);

        if (deviceType == PointerDeviceType::Mouse)
        {
            if (properties.IsLeftButtonPressed())
            {
                _pressedMouseButtons.insert(Babylon::LEFT_MOUSE_BUTTON_ID);
                Babylon::SetMouseButtonState(Babylon::LEFT_MOUSE_BUTTON_ID, true, x, y);
            }

            if (properties.IsMiddleButtonPressed())
            {
                _pressedMouseButtons.insert(Babylon::MIDDLE_MOUSE_BUTTON_ID);
                Babylon::SetMouseButtonState(Babylon::MIDDLE_MOUSE_BUTTON_ID, true, x, y);
            }

            if (properties.IsRightButtonPressed())
            {
                _pressedMouseButtons.insert(Babylon::RIGHT_MOUSE_BUTTON_ID);
                Babylon::SetMouseButtonState(Babylon::RIGHT_MOUSE_BUTTON_ID, true, x, y);
            }
        }
        else
        {
            const auto pointerId = point.PointerId();
            Babylon::SetTouchButtonState(pointerId, true, x, y);
        }
    }

    void EngineView::OnPointerMoved(IInspectable const& /*sender*/, PointerEventArgs const& args)
    {
        const auto point = args.CurrentPoint();
        const auto deviceType = point.PointerDevice().PointerDeviceType();
        const auto position = point.Position();
        const uint32_t x = position.X < 0 ? 0 : static_cast<uint32_t>(position.X);
        const uint32_t y = position.Y < 0 ? 0 : static_cast<uint32_t>(position.Y);

        if (deviceType == PointerDeviceType::Mouse)
        {
            Babylon::SetMousePosition(x, y);
        }
        else
        {
            const auto pointerId = point.PointerId();
            Babylon::SetTouchPosition(pointerId, x, y);
        }
    }

    void EngineView::OnPointerReleased(IInspectable const& /*sender*/, PointerEventArgs const& args)
    {
        const auto point = args.CurrentPoint();
        const auto properties = point.Properties();
        const auto deviceType = point.PointerDevice().PointerDeviceType();
        const auto position = point.Position();
        const uint32_t x = position.X < 0 ? 0 : static_cast<uint32_t>(position.X);
        const uint32_t y = position.Y < 0 ? 0 : static_cast<uint32_t>(position.Y);

        if (point.PointerDevice().PointerDeviceType() == PointerDeviceType::Mouse)
        {
            if (!properties.IsLeftButtonPressed() &&
                _pressedMouseButtons.find(Babylon::LEFT_MOUSE_BUTTON_ID) != _pressedMouseButtons.end())
            {
                _pressedMouseButtons.erase(Babylon::LEFT_MOUSE_BUTTON_ID);
                Babylon::SetMouseButtonState(Babylon::LEFT_MOUSE_BUTTON_ID, false, x, y);
            }

            if (!properties.IsMiddleButtonPressed() &&
                _pressedMouseButtons.find(Babylon::MIDDLE_MOUSE_BUTTON_ID) != _pressedMouseButtons.end())
            {
                _pressedMouseButtons.erase(Babylon::MIDDLE_MOUSE_BUTTON_ID);
                Babylon::SetMouseButtonState(Babylon::MIDDLE_MOUSE_BUTTON_ID, false, x, y);
            }

            if (!properties.IsRightButtonPressed() &&
                _pressedMouseButtons.find(Babylon::RIGHT_MOUSE_BUTTON_ID) != _pressedMouseButtons.end())
            {
                _pressedMouseButtons.erase(Babylon::RIGHT_MOUSE_BUTTON_ID);
                Babylon::SetMouseButtonState(Babylon::RIGHT_MOUSE_BUTTON_ID, false, x, y);
            }
        }
        else
        {
            const auto pointerId = point.PointerId();
            Babylon::SetTouchButtonState(pointerId, false, x, y);
        }
    }

    void EngineView::OnCompositionScaleChanged(SwapChainPanel const& sender, IInspectable const& /*object*/)
    {
        if (_compositionScaleX != sender.CompositionScaleX() || _compositionScaleY != sender.CompositionScaleY())
        {
            critical_section::scoped_lock lock(s_criticalSection);

            // Store values so they can be accessed from a background thread.
            _compositionScaleX = sender.CompositionScaleX();
            _compositionScaleY = sender.CompositionScaleY();

            // Recreate size-dependent resources when the composition scale changes.
            CreateSizeDependentResources();

            auto swapChainPanel = static_cast<SwapChainPanel>(*this);
            auto swapChainPanelNative = swapChainPanel.as<ISwapChainPanelNative>().get();;
            swapChainPanelNative->SetSwapChain(_swapChain.Get());
        }
    }

    void EngineView::OnSuspending()
    {
        critical_section::scoped_lock lock(s_criticalSection);

        ::Microsoft::WRL::ComPtr<IDXGIDevice3> dxgiDevice;
        s_d3dDevice->QueryInterface(__uuidof(IDXGIDevice3), &dxgiDevice);

        // Hints to the driver that the app is entering an idle state and that its memory can be used temporarily for other apps.
        dxgiDevice->Trim();
    }

    void EngineView::OnResuming()
    {}

    void EngineView::OnRendering()
    {
        Concurrency::critical_section::scoped_lock lock(s_criticalSection);

        if (!_backBufferPtr)
            return;

        // Clear the back buffer and depth stencil view.
        float clearColor[4]{ 0.0f, 0.0f, 0.0f, 1.0f };
        s_d3dContext->ClearRenderTargetView(_backBufferPtr.Get(), clearColor);

        Babylon::RenderView();

        // present
        DXGI_PRESENT_PARAMETERS parameters = { 0 };
        parameters.DirtyRectsCount = 0;
        parameters.pDirtyRects = nullptr;
        parameters.pScrollRect = nullptr;
        parameters.pScrollOffset = nullptr;

        HRESULT hr = S_OK;

        hr = _swapChain->Present1(1, 0, &parameters);

        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
            OnDeviceLost();
        }

        // Halt the thread until the next vblank is reached.
        // This ensures the app isn't updating and rendering faster than the display can refresh,
        // which would unnecessarily spend extra CPU and GPU resources.  This helps improve battery life.
        _dxgiOutput->WaitForVBlank();
    }

    void EngineView::OnDeviceLost()
    {
        _swapChain = nullptr;

        // Make sure the rendering state has been released.
        s_d3dContext->OMSetRenderTargets(0, nullptr, nullptr);
        s_d3dContext->Flush();

        if(s_d3dDevice)
        {
            s_d3dDevice->Release();
            s_d3dDevice = nullptr;
        }
        
        CreateDeviceResources();
        CreateSizeDependentResources();
    }

    void EngineView::CreateDeviceResources()
    {
        if(s_d3dDevice) return;
        
        D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1
        };

        ID3D11Device* device{ nullptr };
        ::Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
        D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            0,
            D3D11_CREATE_DEVICE_BGRA_SUPPORT
#if defined(_DEBUG)
            | D3D11_CREATE_DEVICE_DEBUG
#endif
            ,
            featureLevels,
            ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION,
            &device,
            NULL,
            &context
        );

        // Get D3D11.1 device
        if (s_d3dDevice) s_d3dDevice->Release();
        device->QueryInterface(__uuidof(ID3D11Device1), (void**)&s_d3dDevice);

        // Get D3D11.1 context
        context.As(&s_d3dContext);
    }

    void EngineView::CreateSizeDependentResources()
    {
        s_d3dContext->OMSetRenderTargets(0, nullptr, nullptr);
        s_d3dContext->Flush();

        // Set render target size to the rendered size of the panel including the composition scale, 
        // defaulting to the minimum of 1px if no size was specified.
        _renderTargetWidth = _width * _compositionScaleX;
        _renderTargetHeight = _height * _compositionScaleY;

        // If the swap chain already exists, then resize it.
        if (_swapChain)
        {
            _backBufferPtr = nullptr;

            HRESULT hr = _swapChain->ResizeBuffers(
                2,
                static_cast<UINT>(_renderTargetWidth),
                static_cast<UINT>(_renderTargetHeight),
                DXGI_FORMAT_B8G8R8A8_UNORM,
                0
            );

            if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
            {
                // If the device was removed for any reason, a new device and swap chain will need to be created.
                OnDeviceLost();
                return;

            }
        }
        else // Otherwise, create a new one
        {
            DXGI_SWAP_CHAIN_DESC1 scd{ 0 };
            scd.Width = static_cast<UINT>(_renderTargetWidth);
            scd.Height = static_cast<UINT>(_renderTargetHeight);
            scd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
            scd.Stereo = false;
            scd.SampleDesc.Count = 1;
            scd.SampleDesc.Quality = 0;
            scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            scd.BufferCount = 2;
            scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
            scd.Flags = 0;

            // Get underlying DXGI Device from D3D Device.
            ::Microsoft::WRL::ComPtr<IDXGIDevice1> dxgiDevice;
            s_d3dDevice->QueryInterface(__uuidof(IDXGIDevice1), &dxgiDevice);
            
            // Get adapter.
            ::Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
            dxgiDevice->GetAdapter(&dxgiAdapter);

            dxgiAdapter->EnumOutputs(0, &_dxgiOutput);

            // Get factory.
            ::Microsoft::WRL::ComPtr<IDXGIFactory2> dxgiFactory;
            dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));

            // Create swap chain.
            ::Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
            dxgiFactory->CreateSwapChainForComposition(
                s_d3dDevice,
                &scd,
                nullptr,
                &swapChain
            );
            swapChain.As(&_swapChain);

            // Ensure that DXGI does not queue more than one frame at a time. This both reduces 
            // latency and ensures that the application will only render after each VSync, minimizing 
            // power consumption.
            dxgiDevice->SetMaximumFrameLatency(1);
        }

        ::Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
        ::Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backBufferPtr;
        _swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&texture);
        s_d3dDevice->CreateRenderTargetView(texture.Get(), nullptr, &backBufferPtr);
        backBufferPtr.As(&_backBufferPtr);

        // Use windowTypePtr == 2 for xaml swap chain panels
        auto swapChainPanel = static_cast<SwapChainPanel>(*this);
        auto windowTypePtr = reinterpret_cast<void*>(2);
        auto windowPtr = get_abi(swapChainPanel);

        Babylon::UpdateView(windowPtr, (size_t)_renderTargetWidth, (size_t)_renderTargetHeight, windowTypePtr, s_d3dDevice, _backBufferPtr.Get());
    }
}