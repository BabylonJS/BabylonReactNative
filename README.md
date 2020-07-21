# Babylon React Native

This project provides Babylon Native integration into React Native.

[![](https://github.com/BabylonJS/BabylonReactNative/workflows/Publish%20Package/badge.svg)](https://github.com/BabylonJS/BabylonReactNative/actions?query=workflow%3A%22Publish+Package%22)
[![npm version](https://badge.fury.io/js/%40babylonjs%2Freact-native.svg)](https://badge.fury.io/js/%40babylonjs%2Freact-native)

## Current Status

Babylon React Native is in the early phase of its development, and has the following limitations:

1. Android and iOS support only - support for Windows is planned, but the timeline is currently unknown.
1. JavaScriptCore (JSC) only - the published @babylonjs/react-native package is configured specifically for JSC, though it is possible to rebuild it and target other JavaScript engines supported by React Native. In the future, the published package will directly support all JavaScript engines that can be used with React Native.

## Usage

See the [package usage](Apps/Playground/node_modules/@babylonjs/react-native/README.md) for installation instructions and/or the Playground app's [App.tsx](Apps/Playground/App.tsx) for example usage.

## Contributing

This quick overview will help you get started developing in the Babylon React Native repository. We support development on Windows and MacOS, but assume the use of [PowerShell](https://docs.microsoft.com/en-us/powershell/scripting/install/installing-powershell) in the instructions below (unless otherwise noted).

If you are interested in making contributions, be sure to also review [CONTRIBUTING.md](CONTRIBUTING.md).

### **Preparing a new Repo**

**Required Tools:** [git](https://git-scm.com/), [Node.js](https://nodejs.org/en/download/)

Step 1 for all development environments and targets is to clone the repo. Use a git-enabled terminal to follow the steps below.

```
git clone https://github.com/BabylonJS/BabylonReactNative
```

Babylon React Native makes extensive use of submodules to supply its dependencies, so it's also necessary to set up the submodules.

```
cd BabylonReactNative
git submodule update --init --recursive
```

The Playground sample/test app is a standard React Native app, and as such also makes extensive use of NPM packages to supply its dependencies, so it's also necessary to install these packages.

```
cd Apps/Playground
npm install
```

For iOS, CocoaPods are also used, and these must be installed.

```
cd Apps/Playground/ios
pod install --repo-update
```

### **Configuring a Mac Dev Environment**

**Required Tools:** [Android Studio](https://developer.android.com/studio/) (including NDK 21.3.6528147), [CMake](https://cmake.org/), [Ninja](https://ninja-build.org/), [JDK 13](https://www.oracle.com/java/technologies/javase-jdk13-downloads.html)

- The `PATH` environment variable must include the path to adb (typically ~/Library/Android/sdk/platform-tools/).
- The `PATH` environment variable must include the path to Ninja, or Ninja must be [installed via a package manager](https://github.com/ninja-build/ninja/wiki/Pre-built-Ninja-packages).
- The `ANDROID_HOME` environment variable must be defined (typically ~/Library/Android/sdk).
- The `ANDROID_SDK_ROOT` environment variable must be defined (typically ~/Library/Android/sdk).
- The `ANDROID_AVD_HOME` environment variable must be defined if you plan to use Android emulators (typically ~/.android/avd).
- The `JAVA_HOME` environment variable must be defined to point to the correct version of the JDK (typically /usr/libexec/java_home -v 13).

You can typically configure your environment by editing `~/.zshrc` and adding the following:

```
export PATH=$PATH:~/Library/Android/sdk/platform-tools/
export PATH=$PATH:~/path_to_ninja_binary/ # Only for manual installations of Ninja (not package manager-based installations).
export ANDROID_HOME=~/Library/Android/sdk
export ANDROID_SDK_ROOT=~/Library/Android/sdk
export ANDROID_AVD_HOME=~/.android/avd
export JAVA_HOME=$(/usr/libexec/java_home -v 13)
```

### **Configuring a Windows Dev Environment**

**Required Tools:** [Android Studio](https://developer.android.com/studio/) (including NDK 21.3.6528147), [CMake](https://cmake.org/), [Ninja](https://ninja-build.org/)

- The `PATH` environment variable must include the path to adb (typically %LOCALAPPDATA%/Android/sdk/platform-tools/).
- The `PATH` environment variable must include the path to Ninja, or Ninja must be [installed via a package manager](https://github.com/ninja-build/ninja/wiki/Pre-built-Ninja-packages).  
- The `ANDROID_HOME` environment variable must be defined (typically %LOCALAPPDATA%/Android/sdk).
- The `JAVA_HOME` environment variable must be defined (typically %ProgramFiles%/Android/Android Studio/jre).


### **Building and Running the Playground App**

On either Mac or Windows, NPM is used to build and run the Playground sample/test app from the command line. Open a command prompt at the root of the BabylonReactNative repo if you don't have one already open.

#### Android

```
cd Apps/Playground
npm run android
```

After having run the above commands, you can also open `Apps/Playground/android` in Android Studio and run the app from there.

#### iOS

iOS can only be built on a Mac. Additionally, `CMake` must manually be run to generate the XCode project that the [Playground XCode workspace](Apps/Playground/ios/Playground.xcworkspace/contents.xcworkspacedata) includes.

```
pushd Apps/Playground/node_modules/@babylonjs/react-native/ios
cmake -G Xcode -DCMAKE_TOOLCHAIN_FILE=../submodules/BabylonNative/Dependencies/ios-cmake/ios.toolchain.cmake -DPLATFORM=OS64COMBINED -DENABLE_ARC=0 -DENABLE_BITCODE=1 -DDEPLOYMENT_TARGET=12 -DENABLE_GLSLANG_BINARIES=OFF -DSPIRV_CROSS_CLI=OFF .
popd

cd Apps/Playground
npm run ios
```

After having run the above commands, you can also open `Apps/Playground/ios/Playground.xcworkspace` in XCode and run the app from there.

### **Building the NPM Package**

An NPM package can be built in two different ways: as source, and as binaries. Source is useful if you want to debug the Babylon React Native source in the context of the project consuming it, though configuration is a bit more involved. Binaries are useful in that they simplify configuration in the consuming app, though they cannot be debugged so easily. The binary package is what is published to [npmjs.org](https://www.npmjs.com/package/@babylonjs/react-native).

#### Source Package

If you want to test using a local build of the source-based NPM package with your own React Native app, you can do so with the `npm pack` command on either Mac or Windows.

```
cd Apps/Playground/node_modules/@babylonjs/react-native
npm pack
```

This will produce a zipped local NPM source-based package that can be installed into a React Native application for testing purposes. Note that when testing a source based package for iOS, the XCode project needs to be generated with `CMake` as described [above](#ios).

#### Binary Package

If you want to test using a local build of the binary-based NPM package with your own React Native app, you can do so with a `gulp` command on a Mac (this requires a Mac as it builds binaries for both iOS and Android).

```
cd Package
npm install
gulp pack
```

This will produce a zipped local NPM binary-based package that can be installed into a React Native application for testing purposes.

## Security

If you believe you have found a security vulnerability in this repository, please see [SECURITY.md](SECURITY.md).
