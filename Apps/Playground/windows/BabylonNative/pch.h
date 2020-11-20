#pragma once

#define NOMINMAX

#include <unknwn.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.System.Threading.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Data.h>
#include <winrt/Windows.UI.Xaml.Interop.h>
#include <winrt/Windows.UI.Xaml.Markup.h>
#include <winrt/Windows.UI.Xaml.Navigation.h>
#include <winrt/Windows.UI.Xaml.h>

#include <winrt/Microsoft.ReactNative.h>

// BabylonNative
// Required project references Console, JsRuntime, napi, Window
#ifndef NODE_ADDON_API_DISABLE_NODE_SPECIFIC
#define NODE_ADDON_API_DISABLE_NODE_SPECIFIC
#endif

#ifndef NODE_ADDON_API_DISABLE_DEPRECATED
#define NODE_ADDON_API_DISABLE_DEPRECATED
#endif

// Add "../../../../Modules/@babylonjs/react-native/submodules/BabylonNative/Dependencies/napi/napi-direct/include" to additional include directories
#include "napi/env.h"

#include "../../../../Modules/@babylonjs/react-native/submodules/BabylonNative/Polyfills/Console/Include/Babylon/Polyfills/Console.h"
#include "../../../../Modules/@babylonjs/react-native/submodules/BabylonNative/Polyfills/Window/Include/Babylon/Polyfills/Window.h"
#include "../../../../Modules/@babylonjs/react-native/submodules/BabylonNative/Polyfills/XMLHttpRequest/Include/Babylon/Polyfills/XMLHttpRequest.h"
