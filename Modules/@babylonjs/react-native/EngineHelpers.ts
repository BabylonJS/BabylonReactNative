import { Engine } from '@babylonjs/core';

const disposedPropertyName = "EngineHelper_IsDisposed";

console.log("Declaring IsEngineDisposed");
export function IsEngineDisposed(engine: Engine): boolean {
    try {
        console.log("IsEngineDisposed called");
        return (engine as any)[disposedPropertyName];
    } catch (error) {
        console.log(error.stack);
        throw error;
    }
}
console.log("IsEngineDisposed:" + IsEngineDisposed);

console.log("Declaring DisposeEngine");
export function DisposeEngine(engine: Engine) {
    try {
        console.log("DisposeEngine called");
        if (engine && !IsEngineDisposed(engine)) {
            engine.dispose();
            (engine as any)[disposedPropertyName] = true;
        }
    } catch (error) {
        console.log(error.stack);
    }
}
console.log("DisposeEngine:" + DisposeEngine);