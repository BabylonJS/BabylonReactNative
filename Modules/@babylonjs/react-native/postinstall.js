const os = require("os");
const { spawn } = require('child_process');



function iosCMake() {

  const cmakePath = process.env.BABYLON_CMAKE_PATH ?? "cmake";

  const cmakeVersion = spawn(cmakePath, ['--version'], { stdio: 'inherit' });

  const cmake = spawn(cmakePath, [
    '-S', 'ios',
    '-B', 'Build/iOS',
    '-G', 'Xcode',
    '-DFETCHCONTENT_FULLY_DISCONNECTED=ON',
    '-DBABYLON_NATIVE_BUILD_APPS=OFF',
    '-DBABYLON_NATIVE_PLUGIN_TESTUTILS=OFF',
    '-DBABYLON_NATIVE_INSTALL=OFF',
    '-DFETCHCONTENT_SOURCE_DIR_BGFX.CMAKE=./shared/BabylonNative/deps/bgfx.cmake-src',
    '-DFETCHCONTENT_SOURCE_DIR_ARCANA.CPP=./shared/BabylonNative/deps/arcana.cpp-src',
    '-DFETCHCONTENT_SOURCE_DIR_CMAKEEXTENSIONS=./shared/BabylonNative/deps/cmakeextensions-src',
    '-DFETCHCONTENT_SOURCE_DIR_JSRUNTIMEHOST=./shared/BabylonNative/deps/jsruntimehost-src',
    '-DFETCHCONTENT_SOURCE_DIR_URLLIB=./shared/BabylonNative/deps/urllib-src',
    '-DFETCHCONTENT_SOURCE_DIR_SPIRV-CROSS=./shared/BabylonNative/deps/spirv-cross-src',
    '-DFETCHCONTENT_SOURCE_DIR_LIBWEBP=./shared/BabylonNative/deps/libwebp-src',
    '-DFETCHCONTENT_SOURCE_DIR_GLSLANG=./shared/BabylonNative/deps/glslang-src',
    '-DFETCHCONTENT_SOURCE_DIR_BASE-N=./shared/BabylonNative/deps/base-n-src',
    '-DFETCHCONTENT_SOURCE_DIR_IOS-CMAKE=./shared/BabylonNative/deps/ios-cmake-src'
  ], { stdio: 'inherit' });

  cmake.on('exit', code => {
    if (code !== 0) {
      console.error(`CMake exited with code ${code}`);
      process.exit(code);
    }
  });
}

function postInstall() {
  if (
    os.platform() === "darwin" &&
    process.env.BABYLON_NO_CMAKE_POSTINSTALL !== "1"
  ) {
    iosCMake();
  }
}

postInstall();
