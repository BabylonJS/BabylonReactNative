#pragma once

#include <jsi/jsi.h>

// See https://github.com/BabylonJS/BabylonReactNative/issues/60 for original issue.
// This is a work around for what appears to be custom handling of promise continuations and
// asynchronous functions in React Native (e.g. setTimeout, requestAnimationFrame, etc.)
// Without a call to one of these asynchronous functions, the promise continuation is not
// triggered. This creates a no-op setTimeout function that is called in the Babylon Native
// JsRuntime dispatch function to poke the promise continuation execution.
inline std::shared_ptr<facebook::jsi::Function> GetSetTimeout(facebook::jsi::Runtime& rt)
{
    auto code{std::make_shared<const facebook::jsi::StringBuffer>("() => setTimeout(() => {})")};
    return std::make_shared<facebook::jsi::Function>(rt.evaluateJavaScript(std::move(code), "").asObject(rt).asFunction(rt));
}
