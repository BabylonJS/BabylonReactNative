# React Native Babylon

This project provides Babylon Native integration into React Native.

## Getting Started

This quick overview will help you get started developing in the Babylon Native repository. We support development on Windows and macOS.

### **All Development Platforms, Common First Steps**

**Required Tools:** [git](https://git-scm.com/), [CMake](https://cmake.org/), [Yarn](https://classic.yarnpkg.com/en/docs/install)

1. Run `yarn install` from the `Apps/Playground` directory.

### **Building for Android**

**Required Tools:** [Android Studio](https://developer.android.com/studio/)

**Mac Environment Configuration:**

- The `PATH` environment variable must include the path to adb (typically ~/Library/Android/sdk/platform-tools/).
- The `ANDROID_HOME` environment variable must be defined (typically ~/Library/Android/sdk).
- The `ANDROID_SDK_ROOT` environment variable must be defined (typically ~/Library/Android/sdk).
- The `ANDROID_AVD_HOME` environment variable must be defined if you plan to use Android emulators (typically ~/.android/avd).

You can typically configure your environment by editing `~/.zshrc` and adding the following:
```
export PATH=$PATH:~/Library/Android/sdk/platform-tools/
export ANDROID_HOME=~/Library/Android/sdk
export ANDROID_SDK_ROOT=~/Library/Android/sdk
export ANDROID_AVD_HOME=~/.android/avd
```

**Windows Environment Configuration:**

- The `PATH` environment variable must include the path to adb (typically %LOCALAPPDATA%/Android/sdk/platform-tools/).
- The `ANDROID_HOME` environment variable must be defined (typically %LOCALAPPDATA%/Android/sdk).

**Build and Run the Playground App**:

1. Run `npx react-native run-android` from the `Apps/Playground` directory.

After having run the above command, you can also open `Apps/Playground/android` in Android Studio and run the app from there.