import { NativeModules } from 'react-native';
import { NativeEngine } from '@babylonjs/core';
import { DisposeEngine } from './EngineHelpers';

declare const global: {
    nativeCallSyncHook: any;
};
const isRemoteDebuggingEnabled = !global.nativeCallSyncHook;

// This global object is owned by Babylon Native.
declare const _native: {
    whenGraphicsReady: () => Promise<void>;
    engineInstance: NativeEngine;
};

// This JSI-based global object is owned by Babylon React Native.
// This will likely be converted to a TurboModule when they are fully supported.
declare const BabylonNative: {
    readonly initializationPromise: Promise<void>;
    reset: () => void;
};

// This legacy React Native module is created by Babylon React Native, and is only used to bootstrap the JSI object creation.
// This will likely be removed when the BabylonNative global object is eventually converted to a TurboModule.
const NativeBabylonModule: {
    initialize(): void;
} = NativeModules.BabylonModule;

export const BabylonModule = {
    ensureInitialized: async () => {
        console.log("INITIALIZING");
        if (isRemoteDebuggingEnabled) {
            return false;
        } else {
            NativeBabylonModule.initialize();
            await BabylonNative.initializationPromise;
            await _native.whenGraphicsReady();
            return true;
        }
    },

    //reset: NativeBabylonModule.reset,
    reset: () => {
        console.log("RESET");
        BabylonNative.reset();
    },

    createEngine: () => {
        const engine = new NativeEngine();
        _native.engineInstance = engine;
        //BabylonNative.setEngineInstance(engine);
        return engine;
    },

    disposeEngine: (engine: NativeEngine) => {
        console.log("Beginning dispose");
        DisposeEngine(engine);
        console.log("Finished dispose");
        //BabylonNative.setEngineInstance(null);
    },
};