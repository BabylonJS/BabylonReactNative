#include "pch.h"
#include "EngineView.h"
#include "EngineView.g.cpp"

namespace winrt::BabylonReactNative::implementation {
	using namespace winrt::Windows::Devices::Input;
	using namespace winrt::Windows::Foundation;
	using namespace winrt::Windows::System::Threading;
	using namespace winrt::Windows::UI::Core;
	using namespace winrt::Windows::UI::Input;
	using namespace winrt::Windows::UI::Xaml;
	using namespace winrt::Windows::UI::Xaml::Input;
	using namespace winrt::Windows::UI::Xaml::Media;
	using namespace winrt::Windows::UI::Xaml::Controls;
	using namespace winrt::Windows::ApplicationModel;

	static ::Microsoft::WRL::ComPtr<ID3D11Device1> s_d3dDevice;
	static ::Microsoft::WRL::ComPtr<ID3D11DeviceContext1> s_d3dContext;
	
	
	EngineView::EngineView() {
		_revokerData.SizeChangedRevoker = SizeChanged(winrt::auto_revoke, { this, &EngineView::OnSizeChanged });
		_revokerData.SuspendingRevoker = Application::Current().Suspending(winrt::auto_revoke, [weakThis{ get_weak() }](
			auto const& /*sender*/,
			auto const& /*args*/)
		{
			if (auto trueThis = weakThis.get())
			{
				trueThis->OnSuspending();
			}
		});

		_revokerData.ResumingRevoker = Application::Current().Resuming(winrt::auto_revoke, [weakThis{ get_weak() }](
			auto const& /*sender*/,
			auto const& /*args*/)
		{
			if (auto trueThis = weakThis.get())
			{
				if (CoreWindow::GetForCurrentThread().Visible())
				{
					trueThis->OnResuming();
				}
			}
		});

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

		// init device
		if (!s_d3dDevice)
		{
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

			::Microsoft::WRL::ComPtr<ID3D11Device> device;
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
			device.As(&s_d3dDevice);

			// Get D3D11.1 context
			context.As(&s_d3dContext);
		}
	}


	void EngineView::OnSizeChanged(IInspectable const& /*sender*/, SizeChangedEventArgs const& args)
	{
		Concurrency::critical_section::scoped_lock lock(_criticalSection);

		const auto size = args.NewSize();
		_width = static_cast<size_t>(size.Width);
		_height = static_cast<size_t>(size.Height);

		auto swapChainPanel = static_cast<winrt::Windows::UI::Xaml::Controls::SwapChainPanel>(*this);
		auto nativeSwapChainPanel = swapChainPanel.as<ISwapChainPanelNative>().get();

		// Use windowTypePtr == 2 for xaml swap chain panels
		auto windowTypePtr = reinterpret_cast<void*>(2);
		auto windowPtr = reinterpret_cast<void*>(0);

		DXGI_SWAP_CHAIN_DESC1 scd{ 0 };
		scd.Width = static_cast<UINT>(_width);
		scd.Height = static_cast<UINT>(_height);
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
		s_d3dDevice.As(&dxgiDevice);

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
			s_d3dDevice.Get(),
			&scd,
			nullptr,
			&swapChain
		);
		swapChain.As(&_swapChain);

		// Ensure that DXGI does not queue more than one frame at a time. This both reduces 
		// latency and ensures that the application will only render after each VSync, minimizing 
		// power consumption.
		dxgiDevice->SetMaximumFrameLatency(1);

		::Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		::Microsoft::WRL::ComPtr <ID3D11RenderTargetView> backBufferPtr;
		_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&texture);
		s_d3dDevice->CreateRenderTargetView(texture.Get(), nullptr, &backBufferPtr);
		backBufferPtr.As(&_backBufferPtr);

		nativeSwapChainPanel->SetSwapChain(_swapChain.Get());

		Babylon::UpdateView(windowPtr, _width, _height, windowTypePtr, s_d3dDevice.Get(), _backBufferPtr.Get());
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

	void EngineView::OnSuspending()
	{
		Concurrency::critical_section::scoped_lock lock(_criticalSection);

		::Microsoft::WRL::ComPtr<IDXGIDevice3> dxgiDevice;
		s_d3dDevice.As(&dxgiDevice);

		// Hints to the driver that the app is entering an idle state and that its memory can be used temporarily for other apps.
		dxgiDevice->Trim();
	}

	void EngineView::OnResuming()
	{
	}

	void EngineView::OnRendering()
	{
		Concurrency::critical_section::scoped_lock lock(_criticalSection);

		if (_backBufferPtr)
		{
			// Set render targets to the screen.
			ID3D11RenderTargetView* const targets[1] = { _backBufferPtr.Get() };
			s_d3dContext->OMSetRenderTargets(1, targets, nullptr);

			// Clear the back buffer and depth stencil view.
			float clearColor[4]{ 0.392156899f, 0.584313750f, 0.929411829f, 1.000000000f };
			s_d3dContext->ClearRenderTargetView(_backBufferPtr.Get(), clearColor);
		}

		Babylon::RenderView();

		// present
		if (_swapChain)
		{
			DXGI_PRESENT_PARAMETERS parameters = { 0 };
			parameters.DirtyRectsCount = 0;
			parameters.pDirtyRects = nullptr;
			parameters.pScrollRect = nullptr;
			parameters.pScrollOffset = nullptr;

			_swapChain->Present1(1, 0, &parameters);
		}

		if (_dxgiOutput)
		{
			// Halt the thread until the next vblank is reached.
			// This ensures the app isn't updating and rendering faster than the display can refresh,
			// which would unnecessarily spend extra CPU and GPU resources.  This helps improve battery life.
			_dxgiOutput->WaitForVBlank();
		}
	}
}