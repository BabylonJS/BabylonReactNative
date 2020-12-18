import { NativeModules } from 'react-native';
import { Engine, NativeEngine } from '@babylonjs/core';

const disposedPropertyName = "BabylonReactNative_IsDisposed";

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

export async function createEngine(): Promise<NativeEngine> {
    // This waits Graphics/NativeEngine to be created (which in turn makes the whenGraphicsReady available).
    await BabylonNative.initializationPromise;

    // This waits for the Graphics system to be up and running.
    await _native.whenGraphicsReady();

    const engine = new NativeEngine();
    BabylonNative.setEngineInstance(engine);
    return engine;
}

export function isEngineDisposed(engine: Engine): boolean {
    return (engine as any)[disposedPropertyName];
}

export function disposeEngine(engine: NativeEngine): void {
    if (engine && !isEngineDisposed(engine)) {
        engine.dispose();
        (engine as any)[disposedPropertyName] = true;

        // Ideally we would always do a reset here as we don't want different behavior between debug and release. Unfortunately, fast refresh has some strange behavior that
        // makes it quite difficult to get this to work correctly (e.g. it re-runs previous useEffect instances, which means it can try to use Babylon Native in a de-initialized state).
        // TODO: https://github.com/BabylonJS/BabylonReactNative/issues/125
        if (!__DEV__) {
            BabylonNative.reset();
        }
    }

    BabylonNative.setEngineInstance(null);
}