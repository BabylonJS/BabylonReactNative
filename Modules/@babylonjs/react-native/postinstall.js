const os = require("os");
const { spawn } = require('child_process');

function iosCMake() {
  const { spawn, spawnSync } = require('child_process');
  const fs = require('fs');
  const path = require('path');

  // Determine cmake path in order of preference
  let cmakePath = process.env.BABYLON_CMAKE_PATH;
  
  if (!cmakePath) {
    // Check if cmake-runtime package is installed
    try {
      // Try multiple ways to find cmake-runtime
      const possiblePaths = [
        path.join(__dirname, 'node_modules', 'cmake-runtime', 'bin', 'cmake'),
        path.join(process.cwd(), 'node_modules', 'cmake-runtime', 'bin', 'cmake'),
        path.join(__dirname, '..', 'node_modules', 'cmake-runtime', 'bin', 'cmake'),
        path.join(__dirname, '..', '..', 'node_modules', 'cmake-runtime', 'bin', 'cmake')
      ];
      
      for (const possiblePath of possiblePaths) {
        if (fs.existsSync(possiblePath)) {
          cmakePath = possiblePath;
          break;
        }
      }
      
      // If not found by path, try require.resolve as fallback
      if (!cmakePath) {
        try {
          const cmakeRuntimePath = require.resolve('cmake-runtime/cmake');
          if (fs.existsSync(cmakeRuntimePath)) {
            cmakePath = cmakeRuntimePath;
          }
        } catch (e) {
          // cmake-runtime not resolvable
          console.warn(`cmake-runtime not resolvable.`);
        }
      }
    } catch (e) {
      // cmake-runtime not installed, continue to default
      console.warn(`CMake not found in installed packages.`);
    }
  }
  
  if (!cmakePath) {
    cmakePath = 'cmake';
  }

  // Check if cmake exists and get version
  const versionResult = spawnSync(cmakePath, ['--version'], { encoding: 'utf8' });
  
  if (versionResult.error) {
    console.error('CMake is not installed or not found in PATH');
    process.exit(1);
  }

  // Parse version from output (format: "cmake version 3.26.0")
  const versionMatch = versionResult.stdout.match(/cmake version (\d+)\.(\d+)\.(\d+)/);
  if (!versionMatch) {
    console.error('Unable to parse CMake version');
    process.exit(1);
  }

  const majorVersion = parseInt(versionMatch[1]);
  const minorVersion = parseInt(versionMatch[2]);
  
  // Check if version is >= 3.26
  if (majorVersion < 3 || (majorVersion === 3 && minorVersion < 26)) {
    console.error(`CMake version ${majorVersion}.${minorVersion} is not supported. Please upgrade to version 3.26 or higher.`);
    process.exit(1);
  }

  console.log(`Using CMake version ${majorVersion}.${minorVersion}.${versionMatch[3]} from ${cmakePath}`);

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
