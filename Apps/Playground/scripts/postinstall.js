const os = require("os");
const shelljs = require('shelljs');
const version = shelljs.exec('npm --version', {silent: true});

console.log("Checking NPM version...");
if (!version.trim().match(/6\.\d+\.\d+/g)) {
    throw `Error: BabylonReactNative Playground development requires npm version 6.13.*, Your current npm version is ${version}. Run npm install -g npm@6.13 to update your npm version.`;
}

console.log("Updating submodules...");
shelljs.exec("git submodule update --init --recursive", {cwd: "../../"});

if (os.platform() === "darwin") {
    console.log("Running CMake for iOS...");
    shelljs.exec("cmake -G Xcode -DCMAKE_TOOLCHAIN_FILE=../submodules/BabylonNative/Dependencies/ios-cmake/ios.toolchain.cmake -DPLATFORM=OS64COMBINED -DENABLE_ARC=0 -DENABLE_BITCODE=1 -DDEPLOYMENT_TARGET=12 -DENABLE_GLSLANG_BINARIES=OFF -DSPIRV_CROSS_CLI=OFF -DENABLE_PCH=OFF .", {cwd: "node_modules/@babylonjs/react-native/ios"});

    console.log("Installing iOS pods...");
    shelljs.exec("pod install", {cwd: "ios"});
}