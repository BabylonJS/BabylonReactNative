#include "pch.h"
#include "ReactPackageProvider.h"
#if __has_include("ReactPackageProvider.g.cpp")
#include "ReactPackageProvider.g.cpp"
#endif

#include "NativeModules.h"
#include "BabylonModule.h"
#include "EngineViewManager.h"
#include "EngineView.h"

using namespace winrt::Microsoft::ReactNative;

namespace winrt::BabylonReactNative::implementation {

void ReactPackageProvider::CreatePackage(IReactPackageBuilder const &packageBuilder) noexcept
{
  AddAttributedModules(packageBuilder);

  // Different module providers exist, we could look into codegen to make sure that javascript interfaces map 1:1 with constructed modules at compile time
  packageBuilder.AddModule(L"BabylonModule", MakeModuleProvider<BabylonModule>());
  packageBuilder.AddModule(L"EngineViewManager", MakeModuleProvider<EngineViewManager>());
  packageBuilder.AddViewManager(L"EngineView", []() { return winrt::make<EngineView>(); });
}

} // namespace winrt::BabylonReactNative::implementation
