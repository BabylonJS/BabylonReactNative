import { NativeModules } from 'react-native';
import { NativeEngine } from '@babylonjs/core';

// This global object is part of Babylon Native.
console.log("Declaring _native");
declare const _native: {
    graphicsInitializationPromise: Promise<void>;
    engineInstance: NativeEngine;
}

console.log("Declaring NativeBabylonModule");
const NativeBabylonModule: {
    initialize(): Promise<boolean>;
    whenInitialized(): Promise<boolean>;
} = NativeModules.BabylonModule;

console.log("Declaring BabylonModule");
export const BabylonModule = {
    initialize: async () => {
        console.log("Calling NativeBabylonModule.initialize");
        const initialized = await NativeBabylonModule.initialize();
        if (initialized) {
            console.log("Awaiting graphics initialization promise");
            await _native.graphicsInitializationPromise;
        }
        return initialized;
    },

    whenInitialized: NativeBabylonModule.whenInitialized,

    createEngine: () => {
        console.log("Creating NativeEngine");
        const engine = new NativeEngine();
        _native.engineInstance = engine;
        return engine;
    }
};