import BabylonModule from './spec/NativeBabylonModule';

export async function ensureInitialized(): Promise<boolean> {
    await BabylonModule.initialize();
    return true;
}

export async function reset(): Promise<void> {
    return BabylonModule.resetView();
}
