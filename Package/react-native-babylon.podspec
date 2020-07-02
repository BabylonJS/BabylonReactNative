require "json"

package = JSON.parse(File.read(File.join(__dir__, "package.json")))

Pod::Spec.new do |s|
  s.name         = "react-native-babylon"
  s.version      = package["version"]
  s.summary      = package["description"]
  s.homepage     = package["homepage"]
  s.license      = package["license"]
  s.authors      = package["author"]

  s.platforms    = { :ios => "12.0" }
  s.source       = { :git => package["repository"]["url"], :tag => s.version }

  s.source_files = "ios/**/*.{h,m,mm}"
  s.requires_arc = true

  s.vendored_libraries = 'ios/libs/libastc-codec.a',
                         'ios/libs/libastc.a',
                         'ios/libs/libBabylonNative.a',
                         'ios/libs/libbgfx.a',
                         'ios/libs/libbimg.a',
                         'ios/libs/libbx.a',
                         'ios/libs/libglslang.a',
                         'ios/libs/libjsRuntime.a',
                         'ios/libs/libOGLCompiler.a',
                         'ios/libs/libOSDependent.a',
                         'ios/libs/libnapi.a',
                         'ios/libs/libNativeEngine.a',
                         'ios/libs/libNativeInput.a',
                         'ios/libs/libNativeWindow.a',
                         'ios/libs/libNativeXR.a',
                         'ios/libs/libSPIRV.a',
                         'ios/libs/libspirv-cross-core.a',
                         'ios/libs/libspirv-cross-glsl.a',
                         'ios/libs/libspirv-cross-hlsl.a',
                         'ios/libs/libspirv-cross-msl.a',
                         'ios/libs/libWindow.a',
                         'ios/libs/libxr.a'

  s.frameworks = "MetalKit", "ARKit"

  s.dependency "React"
end

