# React Native Babylon

This project provides Babylon Native integration into React Native.

## Consuming

This quick overview will help you understand the constructs provided by Babylon React Native and how to use them in a React Native application. See the Playground app's [App.tsx](Apps/Playground/App.tsx) for example usage.

### `useEngine`

`useEngine` is a **custom React hook** that manages the lifecycle of a Babylon engine instance in the context of an owning React component. A callback is passed to `useEngine` that receives an engine instance which is used to create and configure scenes. For example:

```tsx
import { useEngine } from 'react-native-babylon';
import { Engine, Scene } from '@babylonjs/core';

const MyComponent: FunctionComponent<MyComponentProps> = (props: MyComponentProps) => {
    useEngine((engine: Engine) => {
        const scene = new Scene(engine);
        // Setup the scene!
    });

    return (
        <>
        </>
    );
}
```

### `EngineView`

`EngineView` is a **custom React Native view** that presents a `camera` from a Babylon `scene`. A `camera` therefore is assigned to the `EngineView`. For example:

```tsx
import { EngineView, useEngine } from 'react-native-babylon';
import { Engine, Scene, Camera } from '@babylonjs/core';

const MyComponent: FunctionComponent<MyComponentProps> = (props: MyComponentProps) => {
    const [camera, setCamera] = useState<Camera>();

    useEngine((engine: Engine) => {
        const scene = new Scene(engine);
        scene.createDefaultCamera(true);
        if (scene.activeCamera) {
            setCamera(scene.activeCamera);
        }
        // Setup the scene!
    });

    return (
        <>
            <EngineView style={{flex: 1}} camera={camera} />
        </>
    );
}
```

Note: Currently only one `EngineView` can be active at any given time. Multi-view will be supported in a future release.

## Contributing

This quick overview will help you get started developing in the Babylon React Native repository. We support development on Windows and macOS.

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

**Build the NPM Package**:

1. Run `npm pack` from the `Apps\Playground\node_modules\react-native-babylon` directory.

This will produce a zipped local NPM package that can be installed into a React Native application for testing purposes.