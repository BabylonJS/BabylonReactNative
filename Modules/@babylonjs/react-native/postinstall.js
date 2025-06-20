const os = require("os");
const { exec } = require('child_process'); 

function iosCMake() {
process.stdout.write('ffffff');
  console.log("Running CMake for iOS...");
  exec("cmake -S ios -B Build/iOS -G Xcode -DFETCHCONTENT_FULLY_DISCONNECTED=ON -DBABYLON_NATIVE_BUILD_APPS=OFF -DBABYLON_NATIVE_PLUGIN_TESTUTILS=OFF -DBABYLON_NATIVE_INSTALL=OFF -DFETCHCONTENT_SOURCE_DIR_BGFX.CMAKE=./shared/BabylonNative/deps/bgfx.cmake-src -DFETCHCONTENT_SOURCE_DIR_ARCANA.CPP=./shared/BabylonNative/deps/arcana.cpp-src -DFETCHCONTENT_SOURCE_DIR_CMAKEEXTENSIONS=./shared/BabylonNative/deps/cmakeextensions-src -DFETCHCONTENT_SOURCE_DIR_JSRUNTIMEHOST=./shared/BabylonNative/deps/jsruntimehost-src -DFETCHCONTENT_SOURCE_DIR_URLLIB=./shared/BabylonNative/deps/urllib-src -DFETCHCONTENT_SOURCE_DIR_SPIRV-CROSS=./shared/BabylonNative/deps/spirv-cross-src -DFETCHCONTENT_SOURCE_DIR_LIBWEBP=./shared/BabylonNative/deps/libwebp-src -DFETCHCONTENT_SOURCE_DIR_GLSLANG=./shared/BabylonNative/deps/glslang-src -DFETCHCONTENT_SOURCE_DIR_BASE-N=./shared/BabylonNative/deps/base-n-src -DFETCHCONTENT_SOURCE_DIR_IOS-CMAKE=./shared/BabylonNative/deps/ios-cmake-src", 
    (error, stdout, stderr)=>{      
      if (error) {
        console.error(`Error: ${error.message}`);
        return;
      }
      if (stderr) {
        console.error(`Stderr: ${stderr}`);
        return;
      }
      console.error(`Output: ${stdout}`);

    });
}

function postInstall() {
  if (os.platform() === "darwin") {
    iosCMake();

  
    /*exec("npm install && npx gulp buildIOSRNTA", {
      cwd: "../../Package",
    });*/

    //console.log(chalk.black.bgCyan("Installing iOS pods..."));
    //exec("pod install", { cwd: "ios" });
  }
}

postInstall();
