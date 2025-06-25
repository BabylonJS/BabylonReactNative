require "json"

package = JSON.parse(File.read(File.join(__dir__, "package.json")))

# This Podspec is used for local development

podspec_dir = File.dirname(__FILE__)
base_base_path = File.expand_path('../../../node_modules/@babylonjs/react-native/Build/iOS', podspec_dir)
base_base_path_symlink = File.expand_path('../../../../../Modules/@babylonjs/react-native/Build/iOS', podspec_dir)
base = "$(inherited) #{base_base_path}/\${CONFIGURATION}-\${PLATFORM_NAME} #{base_base_path_symlink}/\${CONFIGURATION}-\${PLATFORM_NAME} "

items = ['/shared/BabylonNative/Repo/Polyfills/Canvas', 
  '/shared/BabylonNative/Repo/Polyfills/Window', 
  '/shared/BabylonNative/Repo/Plugins/ExternalTexture', 
  '/shared/BabylonNative/Repo/Plugins/NativeCamera', 
  '/shared/BabylonNative/Repo/Plugins/NativeCapture', 
  '/shared/BabylonNative/Repo/Plugins/NativeEngine',
  '/shared/BabylonNative/Repo/Plugins/NativeInput', 
  '/shared/BabylonNative/Repo/Plugins/NativeOptimizations', 
  '/shared/BabylonNative/Repo/Plugins/NativeTracing', 
  '/shared/BabylonNative/Repo/Plugins/NativeXr', 
  '/shared/BabylonNative/Repo/Core/Graphics',
  '/shared/BabylonNative/Repo/Dependencies/xr', 
  '/_deps/bgfx.cmake-build/cmake/bgfx',
  '/_deps/bgfx.cmake-build/cmake/bimg',
  '/_deps/bgfx.cmake-build/cmake/bx',
  '/_deps/glslang-build/glslang',
  '/_deps/glslang-build/glslang/OSDependent/Unix',
  '/_deps/glslang-build/OGLCompilersDLL',
  '/_deps/glslang-build/SPIRV',
  '/_deps/urllib-build',
  '/_deps/libwebp-build',
  '/_deps/jsruntimehost-build/Polyfills/XMLHttpRequest',
  '/_deps/jsruntimehost-build/Polyfills/Scheduling',
  '/_deps/jsruntimehost-build/Core/JsRuntime',
  '/_deps/jsruntimehost-build/Core/Node-API-JSI',
  '/_deps/spirv-cross-build']


result = base + items.map { |item| "#{base_base_path}#{item}/\${CONFIGURATION}-\${PLATFORM_NAME}" }.join(" ") + 
  items.map { |itemb| "#{base_base_path_symlink}#{itemb}/\${CONFIGURATION}-\${PLATFORM_NAME}" }.join(" ")

Pod::Spec.new do |s|
  s.name         = "react-native-babylon"
  s.version      = package["version"]
  s.summary      = package["description"]
  s.homepage     = package["homepage"]
  s.license      = package["license"]
  s.authors      = package["author"]

  s.platforms    = { :ios => "12.0" }
  s.source       = { :git => package["repository"]["url"], :tag => s.version }

  s.source_files = "ios/*.{h,m,mm}"
  s.requires_arc = true
  s.xcconfig     = { 'USER_HEADER_SEARCH_PATHS' => '$(inherited) ${PODS_TARGET_SRCROOT}/shared ${PODS_TARGET_SRCROOT}/../react-native/shared',
  'LIBRARY_SEARCH_PATHS'=>  result}

  s.vendored_frameworks = "ios/libs/*.xcframework"

  s.frameworks = "MetalKit", "ARKit", "AVFoundation", "CoreMedia"

  s.libraries = 'BabylonNative',
                'bgfx',
                'bimg',
                'bimg_encode',
                'bimg_decode',
                'Scheduling',
                'bx',
                'Canvas',
                'GenericCodeGen',
                'glslang',
                'glslang-default-resource-limits',
                'Graphics',
                'jsRuntime',
                'OGLCompiler',
                'OSDependent',
                'MachineIndependent',
                'napi',
                'NativeCamera',
                'NativeCapture',
                'NativeEngine',
                'NativeInput',
                'NativeOptimizations',
                'NativeTracing',
                'NativeXR',
                'SPIRV',
                'spirv-cross-core',
                'spirv-cross-msl',
                'UrlLib',
                'Window',
                'XMLHttpRequest',
                'webp',
                'sharpyuv',
                'xr'
  # install_modules_dependencies has been defined in RN 0.70
  # This check ensure that the library can work on older versions of RN
  if defined?(install_modules_dependencies)
    install_modules_dependencies(s)
  else
    s.dependency "React-Core"

    # Don't install the dependencies when we run `pod install` in the old architecture.
    if new_arch_enabled then
      s.compiler_flags = folly_compiler_flags + " -DRCT_NEW_ARCH_ENABLED=1"
      s.pod_target_xcconfig    = {
        "HEADER_SEARCH_PATHS" => "\"$(PODS_ROOT)/boost\"",
        "OTHER_CPLUSPLUSFLAGS" => "-DFOLLY_NO_CONFIG -DFOLLY_MOBILE=1 -DFOLLY_USE_LIBCPP=1",
        "CLANG_CXX_LANGUAGE_STANDARD" => "c++17"
      }
      s.dependency "React-Codegen"
      s.dependency "RCT-Folly"
      s.dependency "RCTRequired"
      s.dependency "RCTTypeSafety"
      s.dependency "ReactCommon/turbomodule/core"
      s.dependency "React-RCTFabric"
    end
  end
end

