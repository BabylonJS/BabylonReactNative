#include "BabylonNative.h"

#include <Babylon/Graphics.h>
#include <Babylon/JsRuntime.h>
#include <Babylon/Plugins/NativeEngine.h>
#include <Babylon/Plugins/NativeInput.h>
#include <Babylon/Plugins/NativeXr.h>
#include <Babylon/Polyfills/Window.h>
#include <Babylon/Polyfills/XMLHttpRequest.h>

#include <arcana/threading/task_schedulers.h>

#include <jsi/jsi.h>

#include <CoreFoundation/CoreFoundation.h>

#include <optional>
#include <sstream>
#include <unistd.h>

#include <DispatchFunction.h>

namespace Babylon
{
    using namespace facebook;

    namespace
    {
        bool isShuttingDown{false};
    }

    class ReactNativeModule : public jsi::HostObject
    {
    public:
//        static std::shared_ptr<ReactNativeModule> GetOrCreate(jsi::Runtime& jsiRuntime, std::shared_ptr<react::CallInvoker> jsCallInvoker)
//        {
//            jsiRuntime.global().getProperty(jsiRuntime, JS_INSTANCE_NAME)
//            auto nativeModule{std::make_shared<ReactNativeModule>(jsiRuntime, jsCallInvoker)};
//            //std::shared_ptr<ReactNativeModule> nativeModule{ new ReactNativeModule(jsiRuntime, jsCallInvoker) };
//            jsiRuntime.global().setProperty(jsiRuntime, JS_INSTANCE_NAME, jsi::Object::createFromHostObject(jsiRuntime, nativeModule));
//            return nativeModule;
//        }

        ReactNativeModule(jsi::Runtime& jsiRuntime, std::shared_ptr<react::CallInvoker> jsCallInvoker)
            : m_env{ Napi::Attach<facebook::jsi::Runtime&>(jsiRuntime) }
            , m_jsCallInvoker{ jsCallInvoker }
            , m_isRunning{ std::make_shared<bool>(true) }
        {
            isShuttingDown = false;

            // Initialize a JS promise that will be returned by whenInitialized, and completed when NativeEngine is initialized.
            m_initPromise = jsiRuntime.global().getPropertyAsFunction(jsiRuntime, "Promise").callAsConstructor
            (
                jsiRuntime,
                jsi::Function::createFromHostFunction(jsiRuntime, jsi::PropNameID::forAscii(jsiRuntime, "executor"), 0, [this](jsi::Runtime& rt, const jsi::Value&, const jsi::Value* args, size_t) -> jsi::Value
                {
                    m_resolveInitPromise = [&rt, resolve{ std::make_shared<jsi::Value>(rt, args[0]) }]()
                    {
                        resolve->asObject(rt).asFunction(rt).call(rt);
                    };
                    return {};
                })
            );
            
            // Initialize Babylon Native core components
            JsRuntime::CreateForJavaScript(m_env, CreateJsRuntimeDispatcher(m_env, jsiRuntime, m_jsCallInvoker, isShuttingDown));
         
            // Initialize Babylon Native plugins
            Plugins::NativeXr::Initialize(m_env);
            m_nativeInput = &Babylon::Plugins::NativeInput::CreateForJavaScript(m_env);
            
            // Initialize Babylon Native polyfills
            Polyfills::Window::Initialize(m_env);

            // NOTE: React Native's XMLHttpRequest is slow and allocates a lot of memory. This does not override
            // React Native's implementation, but rather adds a second one scoped to Babylon and used by WebRequest.ts.
            Polyfills::XMLHttpRequest::Initialize(m_env);
        }
        
        ~ReactNativeModule()
        {
            isShuttingDown = true;
            *m_isRunning = false;
            
            // NOTE: This only happens when the JS engine is shutting down (other than when the app exits, this only
            //       happens during a dev mode reload). In this case, EngineHook.ts won't call NativeEngine.dispose,
            //       so we need to manually do it here to properly clean up these resources.
//            auto native = JsRuntime::NativeObject::GetFromJavaScript(m_env);
//            auto engine = native.Get("engineInstance").As<Napi::Object>();
//            auto dispose = engine.Get("dispose").As<Napi::Function>();
//            dispose.Call(engine, {});
            //m_disposeEngine();
            
            Napi::Detach(m_env);
        }
        
        // NOTE: This only happens when the JS engine is shutting down (other than when the app exits, this only
        //       happens during a dev mode reload). In this case, EngineHook.ts won't call NativeEngine.dispose,
        //       so we need to manually do it here to properly clean up these resources.
        void Deinitialize()
        {
            auto native = JsRuntime::NativeObject::GetFromJavaScript(m_env);
            auto engine = native.Get("engineInstance").As<Napi::Object>();
            auto dispose = engine.Get("dispose").As<Napi::Function>();
            dispose.Call(engine, {});
        }
        
        void UpdateView(void* windowPtr, size_t width, size_t height)
        {
            m_jsCallInvoker->invokeAsync([this, windowPtr, width, height]() {
                if (!m_graphics)
                {
                    m_graphics = Graphics::CreateGraphics(windowPtr, width, height);
                    m_graphics->AddToJavaScript(m_env);
                    Plugins::NativeEngine::Initialize(m_env, true);
                    m_resolveInitPromise();
                }
                else
                {
                    m_graphics->UpdateWindow(windowPtr);
                    m_graphics->UpdateSize(width, height);
                    m_graphics->EnableRendering();
                }
            });
        }
        
//        void SetView(void* windowPtr, size_t width, size_t height)
//        {
//            if (m_graphics)
//            {
//                m_graphics->UpdateWindow(windowPtr);
//                m_graphics->UpdateSize(width, height);
//                m_graphics->EnableRendering();
//            }
//            else
//            {
//                m_graphics = Graphics::CreateGraphics(windowPtr, width, height);
//                m_graphics->AddToJavaScript(m_env);
//                Plugins::NativeEngine::Initialize(m_env, true);
//                m_resolveInitPromise();
//            }
//        }
//
//        void UpdateView(size_t width, size_t height)
//        {
//            if (m_graphics)
//            {
//                m_graphics->UpdateSize(width, height);
//            }
//        }

        void ResetView()
        {
            if (m_graphics)
            {
                m_graphics->DisableRendering();
            }
        }
        
        void SetPointerButtonState(uint32_t pointerId, uint32_t buttonId, bool isDown, uint32_t x, uint32_t y)
        {
            if (isDown)
            {
                m_nativeInput->PointerDown(pointerId, buttonId, x, y);
            }
            else
            {
                m_nativeInput->PointerUp(pointerId, buttonId, x, y);
            }
        }

        void SetPointerPosition(uint32_t pointerId, uint32_t x, uint32_t y)
        {
            m_nativeInput->PointerMove(pointerId, x, y);
        }

        jsi::Value get(jsi::Runtime& runtime, const jsi::PropNameID& prop) override
        {
            if (prop.utf8(runtime) == "initializationPromise")
            {
                return { runtime, m_initPromise };
            }
            else if (prop.utf8(runtime) == "reset")
            {
                return jsi::Function::createFromHostFunction(runtime, prop, 0, [this](jsi::Runtime& rt, const jsi::Value&, const jsi::Value*, size_t) -> jsi::Value
                {
                    this->ResetView();
                    return {};
                });
            }
//            else if (prop.utf8(runtime) == "setEngineInstance")
//            {
//                return jsi::Function::createFromHostFunction(runtime, prop, 0, [this](jsi::Runtime& rt, const jsi::Value&, const jsi::Value* args, size_t count) -> jsi::Value
//                {
//                    if (count > 0 && args[0].isObject())
//                    {
//
//                    }
//                    if (count == 0 || !args[0].isObject())
//                    {
//                        m_disposeEngine = {};
//                    }
//                    else
//                    {
//                        m_disposeEngine = [&rt, engineInstance{ std::make_shared<jsi::Value>(rt, args[0]) }]()
//                        {
//                            engineInstance->getObject(rt).getProperty(rt, "dispose").asObject(rt).getFunction(rt).call(rt);
//                        };
//                    }
//                    return {};
//                });
//            }
            
            return jsi::Value::undefined();
        }
        
//        void set(jsi::Runtime& runtime, const jsi::PropNameID& prop, const jsi::Value& value) override
//        {
//            if (prop.utf8(runtime) == "engineInstance")
//            {
//                if (value.IsNull() || value.IsUndefined())
//                {
//                    m_disposeEngine = {};
//                }
//                else if (value.IsObject())
//                {
//                    m_disposeEngine = [&runtime, engineInstance{ std::make_shared<jsi::Object>(runtime, value.asObject(runtime)) }]()
//                    {
//                        engineInstance->getProperty(runtime, "dispose").asObject(runtime).getFunction(runtime).call(runtime);
//                    };
//                }
//                else
//                {
//                    // TODO: throw?
//                }
//            }
//        }
        
    private:
        jsi::Value m_initPromise{};
        std::function<void()> m_resolveInitPromise{};

        Napi::Env m_env;
        std::shared_ptr<facebook::react::CallInvoker> m_jsCallInvoker{};

        std::shared_ptr<bool> m_isRunning{};
        std::unique_ptr<Graphics> m_graphics{};
        Plugins::NativeInput* m_nativeInput{};
        
//        std::function<void()> m_disposeEngine{};
    };

    namespace
    {
        constexpr auto JS_INSTANCE_NAME{ "BabylonNative" };
        std::weak_ptr<ReactNativeModule> g_nativeModule{};
    }

    void Initialize(facebook::jsi::Runtime& jsiRuntime, std::shared_ptr<facebook::react::CallInvoker> jsCallInvoker)
    {
        if (jsiRuntime.global().getProperty(jsiRuntime, JS_INSTANCE_NAME).isUndefined())
        {
            auto nativeModule{ std::make_shared<ReactNativeModule>(jsiRuntime, jsCallInvoker) };
            jsiRuntime.global().setProperty(jsiRuntime, JS_INSTANCE_NAME, jsi::Object::createFromHostObject(jsiRuntime, nativeModule));
            g_nativeModule = nativeModule;
        }
    }

    void Deinitialize()
    {
        if (auto nativeModule = g_nativeModule.lock())
        {
            nativeModule->Deinitialize();
        }
    }

    void UpdateView(void* windowPtr, size_t width, size_t height)
    {
        if (auto nativeModule = g_nativeModule.lock())
        {
            nativeModule->UpdateView(windowPtr, width, height);
        }
    }

//    void SetView(void* windowPtr, size_t width, size_t height)
//    {
//        if (auto nativeModule = g_nativeModule.lock())
//        {
//            nativeModule->SetView(windowPtr, width, height);
//        }
//    }
//
//    void UpdateView(size_t width, size_t height)
//    {
//        if (auto nativeModule = g_nativeModule.lock())
//        {
//            nativeModule->UpdateView(width, height);
//        }
//    }

//    void ResetView()
//    {
//        if (auto nativeModule = g_nativeModule.lock())
//        {
//            nativeModule->ResetView();
//        }
//    }

    void SetPointerButtonState(uint32_t pointerId, uint32_t buttonId, bool isDown, uint32_t x, uint32_t y)
    {
        if (auto nativeModule = g_nativeModule.lock())
        {
            nativeModule->SetPointerButtonState(pointerId, buttonId, isDown, x, y);
        }
    }

    void SetPointerPosition(uint32_t pointerId, uint32_t x, uint32_t y)
    {
        if (auto nativeModule = g_nativeModule.lock())
        {
            nativeModule->SetPointerPosition(pointerId, x, y);
        }
    }



    class Native::Impl
    {
    public:
        Impl(facebook::jsi::Runtime& jsiRuntime, std::shared_ptr<facebook::react::CallInvoker> callInvoker)
            : env{ Napi::Attach<facebook::jsi::Runtime&>(jsiRuntime) }
            , jsCallInvoker{ callInvoker }
        {
        }
        
        ~Impl()
        {
            Napi::Detach(env);
        }

        Napi::Env env;
        std::shared_ptr<facebook::react::CallInvoker> jsCallInvoker;
        std::unique_ptr<Graphics> graphics{};
        JsRuntime* runtime{};
        Plugins::NativeInput* nativeInput{};
    };

    Native::Native(facebook::jsi::Runtime& jsiRuntime, std::shared_ptr<facebook::react::CallInvoker> callInvoker, void* windowPtr, size_t width, size_t height)
        : m_impl{ std::make_unique<Native::Impl>(jsiRuntime, callInvoker) }
    {
        isShuttingDown = false;
        m_impl->graphics = Graphics::CreateGraphics(reinterpret_cast<void*>(windowPtr), width, height);

        m_impl->runtime = &JsRuntime::CreateForJavaScript(m_impl->env, CreateJsRuntimeDispatcher(m_impl->env, jsiRuntime, std::move(callInvoker), isShuttingDown));

        m_impl->graphics->AddToJavaScript(m_impl->env);

        Polyfills::Window::Initialize(m_impl->env);
        // NOTE: React Native's XMLHttpRequest is slow and allocates a lot of memory. This does not override
        // React Native's implementation, but rather adds a second one scoped to Babylon and used by WebRequest.ts.
        Polyfills::XMLHttpRequest::Initialize(m_impl->env);

        Plugins::NativeEngine::Initialize(m_impl->env, true);
        Plugins::NativeXr::Initialize(m_impl->env);

        m_impl->nativeInput = &Babylon::Plugins::NativeInput::CreateForJavaScript(m_impl->env);
    }

    // NOTE: This only happens when the JS engine is shutting down (other than when the app exits, this only
    //       happens during a dev mode reload). In this case, EngineHook.ts won't call NativeEngine.dispose,
    //       so we need to manually do it here to properly clean up these resources.
    Native::~Native()
    {
        auto native = JsRuntime::NativeObject::GetFromJavaScript(m_impl->env);
        auto engine = native.Get("engineInstance").As<Napi::Object>();
        auto dispose = engine.Get("dispose").As<Napi::Function>();
        dispose.Call(engine, {});
        isShuttingDown = true;
    }

    void Native::Refresh(void* windowPtr, size_t width, size_t height)
    {
        m_impl->graphics->UpdateWindow<void*>(windowPtr);
        m_impl->graphics->UpdateSize(width, height);
        m_impl->graphics->EnableRendering();
    }

    void Native::Resize(size_t width, size_t height)
    {
        m_impl->graphics->UpdateSize(width, height);
    }

    void Native::Reset()
    {
        m_impl->graphics->DisableRendering();
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
