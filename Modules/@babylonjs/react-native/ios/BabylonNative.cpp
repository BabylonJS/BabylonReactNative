#include "BabylonNative.h"

#include <Babylon/Graphics.h>
#include <Babylon/JsRuntime.h>
#include <Babylon/Plugins/NativeWindow.h>
#include <Babylon/Plugins/NativeEngine.h>
#include <Babylon/Plugins/NativeInput.h>
#include <Babylon/Plugins/NativeXr.h>
#include <Babylon/Polyfills/Window.h>

#include <arcana/threading/task_schedulers.h>

#include <jsi/jsi.h>

#include <CoreFoundation/CoreFoundation.h>

#include <optional>
#include <sstream>
#include <unistd.h>

namespace Babylon
{
    using namespace facebook;

    class Native::Impl
    {
    public:
        Impl(facebook::jsi::Runtime* jsiRuntime)
            : env{ Napi::Attach<facebook::jsi::Runtime&>(*jsiRuntime) }
        {
        }

        std::unique_ptr<Graphics> m_graphics{};
        Napi::Env env;
        JsRuntime* runtime{};
        Plugins::NativeInput* nativeInput{};
    };

    Native::Native(facebook::jsi::Runtime* jsiRuntime, void* windowPtr, size_t width, size_t height)
        : m_impl{ std::make_unique<Native::Impl>(jsiRuntime) }
    {
        dispatch_sync(dispatch_get_main_queue(), ^{
            m_impl->m_graphics = Graphics::InitializeFromWindow<void*>(windowPtr, width, height);
        });

        auto run_loop_scheduler = std::make_shared<arcana::run_loop_scheduler>(arcana::run_loop_scheduler::get_for_current_thread());

        JsRuntime::DispatchFunctionT dispatchFunction{[env = m_impl->env, run_loop_scheduler = std::move(run_loop_scheduler)](std::function<void(Napi::Env)> func)
        {
            (*run_loop_scheduler)([env, func = std::move(func)]()
            {
                func(env);
            });
        }};

        m_impl->runtime = &JsRuntime::CreateForJavaScript(m_impl->env, std::move(dispatchFunction));
        
        m_impl->m_graphics->AddToJavaScript(m_impl->env);

        Polyfills::Window::Initialize(m_impl->env);

        Plugins::NativeWindow::Initialize(m_impl->env, windowPtr, width, height);
        Plugins::NativeEngine::Initialize(m_impl->env);
        Plugins::NativeXr::Initialize(m_impl->env);

        m_impl->nativeInput = &Babylon::Plugins::NativeInput::CreateForJavaScript(m_impl->env);
    }

    Native::~Native()
    {
    }

    void Native::Refresh(void* windowPtr, size_t width, size_t height)
    {
        m_impl->m_graphics->ReinitializeFromWindow<void*>(windowPtr, width, height);
        Plugins::NativeWindow::Reinitialize(m_impl->env, windowPtr, width, height);
    }

    void Native::Resize(size_t width, size_t height)
    {
        Plugins::NativeWindow::UpdateSize(m_impl->env, width, height);
    }

    void Native::SetPointerButtonState(uint32_t pointerId, uint32_t buttonId, bool isDown, uint32_t x, uint32_t y)
    {
        if (isDown)
        {
            m_impl->nativeInput->PointerDown(pointerId, buttonId, x, y);
        }
        else
        {
            m_impl->nativeInput->PointerUp(pointerId, buttonId, x, y);
        }
    }

    void Native::SetPointerPosition(uint32_t pointerId, uint32_t x, uint32_t y)
    {
        m_impl->nativeInput->PointerMove(pointerId, x, y);
    }
}
