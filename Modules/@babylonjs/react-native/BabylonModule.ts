import { NativeModules } from 'react-native';
import { NativeEngine } from '@babylonjs/core';

// This global object is part of Babylon Native.
console.log("Declaring _native");
declare const _native: {
    graphicsInitializationPromise: Promise<void>;
    engineInstance: NativeEngine;
}
try {
    console.log("_native:" + _native);
} catch (error) {
    console.log(error.stack);
}

console.log("Declaring NativeBabylonModule");
const NativeBabylonModule: {
    initialize(): Promise<boolean>;
    whenInitialized(): Promise<boolean>;
} = NativeModules.BabylonModule;
try {
    console.log("NativeBabylonModule:" + NativeBabylonModule);
} catch (error) {
    console.log(error.stack);
}

console.log("Declaring BabylonModule");
export const BabylonModule = {
    initialize: async () => {
        try {
            console.log("Calling NativeBabylonModule.initialize");
            const initialized = await NativeBabylonModule.initialize();
            if (initialized) {
                console.log("Awaiting graphics initialization promise");
                await _native.graphicsInitializationPromise;
            }

            return initialized;
        } catch (error) {
            console.log(error.stack);
            throw error;
        }
    },

    whenInitialized: NativeBabylonModule.whenInitialized,

    createEngine: () => {
        try {
            console.log("Creating NativeEngine");
            const engine = new NativeEngine();
            _native.engineInstance = engine;
            return engine;
        } catch (error) {
            console.log(error.stack);
            throw error;
        }
    }
};
try {
    console.log("BabylonModule:" + BabylonModule);
} catch (error) {
    console.log(error.stack);
}