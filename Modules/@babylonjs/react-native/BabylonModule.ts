import { NativeModules } from 'react-native';
import { NativeEngine } from '@babylonjs/core';

declare const global: {
    nativeCallSyncHook: any;
};
const isRemoteDebuggingEnabled = !global.nativeCallSyncHook;

// This global object is owned by Babylon Native.
declare const _native: {
    whenGraphicsReady: () => Promise<void>;
};

// This JSI-based global object is owned by Babylon React Native.
// This will likely be converted to a TurboModule when they are fully supported.
declare const BabylonNative: {
    readonly initializationPromise: Promise<void>;
    setEngineInstance: (engine: NativeEngine | null) => void;
    reset: () => void;
};

// This legacy React Native module is created by Babylon React Native, and is only used to bootstrap the JSI object creation.
// This will likely be removed when the BabylonNative global object is eventually converted to a TurboModule.
const BabylonModule: {
    initialize(): Promise<void>;
} = NativeModules.BabylonModule;

export class ReactNativeEngine extends NativeEngine {
    private _isDisposed = false;

    private constructor() {
        super();
        BabylonNative.setEngineInstance(this);
    }

    public static async createAsync(): Promise<ReactNativeEngine> {
        // This waits Graphics/NativeEngine to be created (which in turn makes the whenGraphicsReady available).
        await BabylonNative.initializationPromise;

        // This waits for the Graphics system to be up and running.
        await _native.whenGraphicsReady();

        return new ReactNativeEngine();
    }

    public get isDisposed() {
        return this._isDisposed;
    }

    public dispose(): void {
        if (!this.isDisposed) {
            super.dispose();

            // Ideally we would always do a reset here as we don't want different behavior between debug and release. Unfortunately, fast refresh has some strange behavior that
            // makes it quite difficult to get this to work correctly (e.g. it re-runs previous useEffect instances, which means it can try to use Babylon Native in a de-initialized state).
            // TODO: https://github.com/BabylonJS/BabylonReactNative/issues/125
            if (!__DEV__) {
                BabylonNative.reset();
            }

            this._isDisposed = true;
        }

        BabylonNative.setEngineInstance(null);
    }
}

export async function ensureInitialized(): Promise<boolean> {
    if (isRemoteDebuggingEnabled) {
        // When remote debugging is enabled, JavaScript runs on the debugging host machine, not on the device where the app is running.
        // JSI (which Babylon Native uses heavily) can not work in this mode. In the future, this debugging mode will be phased out as it is incompatible with TurboModules for the same reason.
        return false;
    } else {
        // This does the first stage of Babylon Native initialization, including creating the BabylonNative JSI object.
        await BabylonModule.initialize();
        return true;
    }
}