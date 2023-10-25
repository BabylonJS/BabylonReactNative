# Babylon React Native Basekit

## Usage

This quick overview will help you understand the constructs provided by Babylon React Native and how to use them in a React Native application.

### Dependencies

This package has several **peer dependencies**. If these dependencies are unmet, `npm install` will emit warnings. Be sure to add these dependencies to your project.

### Android Configuration

The minimum Android SDK version is 18. This must be set as `minSdkVersion` in the consuming project's `build.gradle` file.

### iOS Configuration

The minimum deployment target version is 12. This must be set as `iOS Deployment Target` in the consuming project's `project.pbxproj`, and must also be set as `platform` in the consuming project's `podfile`.

### Platform Native Packages

Babylon React Native supports react-native from 0.69 to 0.71+. See project Github main readme for supported versions.

### `useEngine`

`useEngine` is a **custom React hook** that manages the lifecycle of a Babylon engine instance in the context of an owning React component. `useEngine` creates an engine instance **asynchronously** which is used to create and configure scenes. Typically scene initialization code should exist in a `useEffect` triggered by an `engine` state change. For example:

```tsx
import { useEngine } from '@babylonjs/react-native-basekit';
import { Engine, Scene } from '@babylonjs/core';

const MyComponent: FunctionComponent<MyComponentProps> = (props: MyComponentProps) => {
    const engine = useEngine();

    useEffect(() => {
        if (engine) {
            const scene = new Scene(engine);
            // Setup the scene!
        }
    }, [engine]);

    return (
        <>
        </>
    );
}
```

### `EngineView`

`EngineView` is a **custom React Native view** that presents a `camera` from a Babylon `scene`. A `camera` therefore is assigned to the `EngineView`. For example:

```tsx
import { useEngine, EngineView } from '@babylonjs/react-native-basekit';
import { Engine, Scene, Camera } from '@babylonjs/core';

const MyComponent: FunctionComponent<MyComponentProps> = (props: MyComponentProps) => {
    const engine = useEngine();
    const [camera, setCamera] = useState<Camera>();

    useEffect(() => {
        if (engine) {
            const scene = new Scene(engine);
            scene.createDefaultCamera(true);
            setCamera(scene.activeCamera!);
            // Setup the scene!
        }
    }, [engine]);

    return (
        <>
            <EngineView style={{flex: 1}} camera={camera} />
        </>
    );
}
```

Also the `EngineView` has a boolean `isTransparent` flag which defines whether the background of the scene should be transparent or not.

e.g.

```tsx
<EngineView style={{flex: 1}} camera={camera} isTransparent={true} />
```
To configure anti-aliasing, a property called `antiAliasing` can be changed to a value of 0 or 1 (disable anti-aliasing, default), 2, 4, 8 or 16 (anti-aliasing samples).

e.g.

```tsx
<EngineView style={{flex: 1}} camera={camera} MSAA={4} />
```

Note: Currently only one `EngineView` can be active at any given time. Multi-view will be supported in a future release.

The Android specific `androidView` property can help set the type of the view used for rendering. Depending on user needs and performance, refer to the table below. [`TextureView`](https://developer.android.com/reference/android/view/TextureView) can be inserted anywhere in the view hierarchy, but is less efficient. [`SurfaceView`](https://developer.android.com/reference/android/view/SurfaceView) can only be full above or fully below the rest of the UI, but is more efficient.

| isTransparent | androidView        | Description |
| ----------- | ------------------------ | ----------- |
| False       | TextureView              | Opaque TextureView.
| False       | SurfaceView              | Simple surfaceView (default when no `androidView` set with `isTransparent=false`).
| False       | SurfaceViewZTopMost      | SurfaceView with [ZTopMost](https://developer.android.com/reference/android/view/SurfaceView#setZOrderOnTop(boolean)) set to `true`.
| False       | SurfaceViewZMediaOverlay | SurfaceView with [ZMediaOverlay](https://developer.android.com/reference/android/view/SurfaceView#setZOrderMediaOverlay(boolean)) set to `true`.
| True        | TextureView              | Transparent TextureView.
| True        | SurfaceView              | SurfaceView will stay opaque
| True        | SurfaceViewZTopMost      | SurfaceView with [ZTopMost](https://developer.android.com/reference/android/view/SurfaceView#setZOrderOnTop(boolean)) set to `true`. Transparent but top most. (default when no `androidView` set with `isTransparent=true`)
| True        | SurfaceViewZMediaOverlay | SurfaceView with [ZMediaOverlay](https://developer.android.com/reference/android/view/SurfaceView#setZOrderMediaOverlay(boolean)) set to `true`. Only Transparent on top of other SurfaceViews.

More infos on TextureView Vs SurfaceView performance here:
https://developer.android.com/reference/android/view/TextureView
