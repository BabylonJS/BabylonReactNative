#pragma once

#include <Babylon/JsRuntime.h>

#include <jsi/jsi.h>
#include <ReactCommon/CallInvoker.h>
#include <android/log.h>

namespace Babylon
{
    using namespace facebook;

    //namespace
    //{
        void log(const char *str)
        {
            __android_log_print(ANDROID_LOG_VERBOSE, "BabylonNative", "%s", str);
        }
    //}

    // Creates a JsRuntime::DispatchFunctionT that integrates with the React Native execution environment.
    inline JsRuntime::DispatchFunctionT CreateJsRuntimeDispatcher(Napi::Env env, jsi::Runtime& jsiRuntime, std::shared_ptr<react::CallInvoker> callInvoker, const bool& isShuttingDown)
    {
        return [env, &jsiRuntime, callInvoker, &isShuttingDown](std::function<void(Napi::Env)> func)
        {
            // Ideally we would just use CallInvoker::invokeAsync directly, but currently it does not seem to integrate well with the React Native logbox.
            // To work around this, we wrap all functions in a try/catch, and when there is an exception, we do the following:
            // 1. Call the JavaScript setImmediate function.
            // 2. Have the setImmediate callback call back into native code (throwFunc).
            // 3. Re-throw the exception from throwFunc.
            // This works because:
            // 1. setImmediate queues the callback, and that queue is drained immediately following the invocation of the function passed to CallInvoker::invokeAsync.
            // 2. The immediates queue is drained as part of the class bridge, which knows how to display the logbox for unhandled exceptions.
            // In the future, CallInvoker::invokeAsync likely will properly integrate with logbox, at which point we can remove the try/catch and just call func directly.
            log("DISPATCHING");
            callInvoker->invokeAsync([env, &jsiRuntime, func{std::move(func)}, &isShuttingDown]
            {
                log("INVOKEASYNC");
                try
                {
                    if (!isShuttingDown)
                    {
                        log("INVOKING FUNC");
                        func(env);
                    }
                }
                catch (...)
                {
                    auto ex{std::current_exception()};
                    auto setImmediate{jsiRuntime.global().getPropertyAsFunction(jsiRuntime, "setImmediate")};
                    auto throwFunc{jsi::Function::createFromHostFunction(jsiRuntime, jsi::PropNameID::forAscii(jsiRuntime, "throwFunc"), 0,
                        [ex](jsi::Runtime &, const jsi::Value &, const jsi::Value *, size_t) -> jsi::Value
                        {
                            std::rethrow_exception(ex);
                        })};
                    setImmediate.call(jsiRuntime, {std::move(throwFunc)});
                }
            });
        };
    }
}