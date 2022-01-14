#pragma once

#include <jsi/jsi.h>

#if defined(__APPLE__)
#include <MetalKit/MTKView.h>
#elif defined(ANDROID)
#include <android/native_window.h>
#elif WINAPI_FAMILY == WINAPI_FAMILY_APP
#include <winrt/Windows.UI.Xaml.Controls.h>
#endif

namespace BabylonNative
{
    #if defined(__APPLE__)
        using WindowType = MTKView*;
    #elif defined(ANDROID)
        using WindowType = ANativeWindow*;
    #elif WINAPI_FAMILY == WINAPI_FAMILY_APP
        using WindowType = winrt::Windows::UI::Xaml::Controls::SwapChainPanel;
    #else
        #error Unsupported platform
    #endif

    using Dispatcher = std::function<void(std::function<void()>)>;

    void Initialize(facebook::jsi::Runtime& jsiRuntime, Dispatcher jsDispatcher);
    void Deinitialize();

    void UpdateView(WindowType window, size_t width, size_t height);

    void RenderView();
    void ResetView();
    void SetMouseButtonState(uint32_t buttonId, bool isDown, uint32_t x, uint32_t y);
    void SetMousePosition(uint32_t x, uint32_t y);
    void SetTouchButtonState(uint32_t pointerId, bool isDown, uint32_t x, uint32_t y);
    void SetTouchPosition(uint32_t pointerId, uint32_t x, uint32_t y);
    bool IsXRActive();
    void UpdateXRView(WindowType window);

    extern const uint32_t LEFT_MOUSE_BUTTON_ID;
    extern const uint32_t MIDDLE_MOUSE_BUTTON_ID;
    extern const uint32_t RIGHT_MOUSE_BUTTON_ID;
}
