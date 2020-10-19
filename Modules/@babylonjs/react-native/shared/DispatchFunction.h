#pragma once

#include <Babylon/JsRuntime.h>

#include <jsi/jsi.h>
#include <ReactCommon/CallInvoker.h>

namespace Babylon
{
    using namespace facebook;

    inline JsRuntime::DispatchFunctionT CreateJsRuntimeDispatcher(Napi::Env env, jsi::Runtime& jsiRuntime, std::shared_ptr<react::CallInvoker> callInvoker)
    {
        return [env, &jsiRuntime, callInvoker](std::function<void(Napi::Env)> func)
        {
            callInvoker->invokeAsync([env, &jsiRuntime, func{std::move(func)}]
            {
                try
                {
                    func(env);
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