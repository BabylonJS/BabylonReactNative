import { NativeModules } from 'react-native';
import { NativeEngine } from '@babylonjs/core';
import { DisposeEngine } from './EngineHelpers';

// This global object is part of Babylon Native.
declare const _native: {
    whenGraphicsReady: () => Promise<void>;
    engineInstance: NativeEngine;
};

declare const BabylonNative: {
    readonly initializationPromise: Promise<void>;
    //setEngineInstance: (engine: NativeEngine | null) => void;
    reset: () => void;
};

const NativeBabylonModule: {
    initialize2(): void;
    // initialize(): Promise<boolean>;
    // whenInitialized(): Promise<boolean>;
    // reset(): Promise<boolean>;
} = NativeModules.BabylonModule;

let resolveInitializationPromise: (result: boolean) => void;
const initializationPromise = new Promise<boolean>(resolve => resolveInitializationPromise = resolve); // TODO: Promise<void>

export const BabylonModule = {
    initialize: async () => {
        console.log("INITIALIZING");
        // const initialized = await NativeBabylonModule.initialize();
        // if (initialized) {
        //     await _native.whenGraphicsReady();
        // }
        // return initialized;
        NativeBabylonModule.initialize2();
        await BabylonNative.initializationPromise;
        await _native.whenGraphicsReady();
        resolveInitializationPromise(true);
        return true; // TODO: remove, and prevent most of this code from running if we are in remote debugging mode (see EngineView.tsx for example)
    },

    //whenInitialized: NativeBabylonModule.whenInitialized,
    whenInitialized: () => {
        console.log("WHEN INITIALIZED");
        //return BabylonNative.initializationPromise;
        return initializationPromise;
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
        DisposeEngine(engine);
        //BabylonNative.setEngineInstance(null);
    },
};