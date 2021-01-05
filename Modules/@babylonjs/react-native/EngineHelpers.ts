import { Engine } from '@babylonjs/core';

const disposedPropertyName = "EngineHelper_IsDisposed";

console.log("Declaring IsEngineDisposed");
export function IsEngineDisposed(engine: Engine): boolean {
    console.log("Checking if engine is disposed");
    return (engine as any)[disposedPropertyName];
}

console.log("Declaring DisposeEngine");
export function DisposeEngine(engine: Engine) {
    if (engine && !IsEngineDisposed(engine)) {
        engine.dispose();
        (engine as any)[disposedPropertyName] = true;
    }
}