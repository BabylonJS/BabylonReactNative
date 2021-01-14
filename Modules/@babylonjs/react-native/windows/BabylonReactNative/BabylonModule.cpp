#include "pch.h"
#include "BabylonModule.h"
#include "EngineView.h"

#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.Graphics.Display.h>
#include <winrt/Windows.Foundation.Numerics.h>
#include <winrt/Windows.UI.Xaml.Controls.h>

using namespace winrt::BabylonReactNative::implementation;

REACT_INIT(Initialize);
void BabylonModule::Initialize(const winrt::Microsoft::ReactNative::ReactContext& /*reactContext*/) noexcept
{
}

REACT_METHOD(CustomInitialize, L"initialize");
void BabylonModule::CustomInitialize(const winrt::Microsoft::ReactNative::ReactPromise<bool>& result) noexcept
{
    EngineView::CompleteOnInitialization(result);
}

REACT_METHOD(WhenInitialized, L"whenInitialized");
void BabylonModule::WhenInitialized(const winrt::Microsoft::ReactNative::ReactPromise<bool>& result) noexcept
{
    EngineView::CompleteOnInitialization(result);
}

REACT_METHOD(Reset, L"reset");
void BabylonModule::Reset(const winrt::Microsoft::ReactNative::ReactPromise<bool>& result) noexcept
{
    EngineView::Reset(result);
}

BabylonModule::BabylonModule()
{
}

BabylonModule::~BabylonModule()
{
}