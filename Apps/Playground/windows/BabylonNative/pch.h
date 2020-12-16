#pragma once

#define NOMINMAX

#include <unknwn.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.System.Threading.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Data.h>
#include <winrt/Windows.UI.Xaml.Input.h>
#include <winrt/Windows.UI.Xaml.Interop.h>
#include <winrt/Windows.UI.Xaml.Markup.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.Navigation.h>
#include <winrt/Windows.UI.Xaml.h>

#include <winrt/Microsoft.ReactNative.h>

// BabylonNative
#ifndef NODE_ADDON_API_DISABLE_NODE_SPECIFIC
#define NODE_ADDON_API_DISABLE_NODE_SPECIFIC
#endif

#ifndef NODE_ADDON_API_DISABLE_DEPRECATED
#define NODE_ADDON_API_DISABLE_DEPRECATED
#endif

// Note: all referenced libs need to be updated to NOT consume windows runtime, this requires code changes in bx.lib
// Note: libs were updated to use std c++ lib 17, not sure if this is actually needed

// Add "../../../../Modules/@babylonjs/react-native/submodules/BabylonNative/Dependencies/napi/napi-direct/include" to additional include directories
#include "napi/env.h"

// Add "../../../../Modules/@babylonjs/react-native/submodules/BabylonNative/Core/JsRuntime/Include" to additional include directories
#include "Babylon/JsRuntime.h"

// Add "../../../../Modules/@babylonjs/react-native/submodules/BabylonNative/Core/Graphics/Include" to additional include directories
#include "Babylon/Graphics.h"

#include "../../../../Modules/@babylonjs/react-native/submodules/BabylonNative/Polyfills/Console/Include/Babylon/Polyfills/Console.h"
#include "../../../../Modules/@babylonjs/react-native/submodules/BabylonNative/Polyfills/Window/Include/Babylon/Polyfills/Window.h"
#include "../../../../Modules/@babylonjs/react-native/submodules/BabylonNative/Polyfills/XMLHttpRequest/Include/Babylon/Polyfills/XMLHttpRequest.h"
#include "../../../../Modules/@babylonjs/react-native/submodules/BabylonNative/Plugins/NativeEngine/Include/Babylon/Plugins/NativeEngine.h"
#include "../../../../Modules/@babylonjs/react-native/submodules/BabylonNative/Plugins/NativeXr/Include/Babylon/Plugins/NativeXr.h"
#include "../../../../Modules/@babylonjs/react-native/submodules/BabylonNative/Plugins/NativeInput/Include/Babylon/Plugins/NativeInput.h"
