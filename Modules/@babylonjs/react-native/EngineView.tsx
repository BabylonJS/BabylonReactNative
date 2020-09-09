import React, { FunctionComponent, Component, useEffect, useState, useRef, SyntheticEvent } from 'react';
import { requireNativeComponent, NativeModules, ViewProps, AppState, AppStateStatus, View, Text, findNodeHandle, UIManager } from 'react-native';
import { Camera } from '@babylonjs/core';
import { IsEngineDisposed } from './EngineHelpers';
import { BabylonModule } from './BabylonModule';

declare const global: any;
const isRemoteDebuggingEnabled = !global['nativeCallSyncHook'];

const EngineViewManager: {
    setJSThread(): void;
} = NativeModules.EngineViewManager;

// Not all platforms need this, but for those that do, this is intended to be a synchronous call to boostrap the ability to run native code on the JavaScript thread.
if (EngineViewManager && EngineViewManager.setJSThread && !isRemoteDebuggingEnabled) {
    EngineViewManager.setJSThread();
}

interface NativeEngineViewProps extends ViewProps {
    snapshotDataReturned: (event: SyntheticEvent) => void;
}

const NativeEngineView: {
    prototype: Component<NativeEngineViewProps>;
    new(props: Readonly<NativeEngineViewProps>): Component<NativeEngineViewProps>;
} = requireNativeComponent('EngineView');

export interface EngineViewProps extends ViewProps {
    camera?: Camera;
    displayFrameRate?: boolean;
    initialized?: (view: EngineViewHooks) => void;
}

export interface EngineViewHooks {
    takeScreenshot: () => Promise<string>;
}

export const EngineView: FunctionComponent<EngineViewProps> = (props: EngineViewProps) => {
    const [failedInitialization, setFailedInitialization] = useState(false);
    const [fps, setFps] = useState<number>();
    const engineViewRef = useRef<Component<NativeEngineViewProps>>(null);
    const [screenshotPromise, setScreenshotPromise] = useState<{promise: Promise<string>, resolve: (data: string) => void, reject: () => void}>();

    useEffect(() => {
        (async () => {
            if (!await BabylonModule.whenInitialized()) {
                setFailedInitialization(true);
            }
        })();
    }, []);

    useEffect(() => {
        if (props.camera) {
            const engine = props.camera.getScene().getEngine();

            if (!IsEngineDisposed(engine)) {
                const onAppStateChanged = (appState: AppStateStatus) => {
                    if (appState === "active") {
                        engine.runRenderLoop(() => {
                            for (let scene of engine.scenes) {
                                scene.render();
                            }
                        });
                    } else {
                        engine.stopRenderLoop();
                    }
                };

                onAppStateChanged(AppState.currentState);
                AppState.addEventListener("change", onAppStateChanged);

                return () => {
                    if (!IsEngineDisposed(engine)) {
                        engine.stopRenderLoop();
                    }
                    AppState.removeEventListener("change", onAppStateChanged);
                };
            }
        }
    }, [props.camera]);

    useEffect(() => {
        if (props.camera && (props.displayFrameRate ?? __DEV__)) {
            const engine = props.camera.getScene().getEngine();

            if (!IsEngineDisposed(engine)) {
                setFps(engine.getFps());
                const timerHandle = setInterval(() => {
                    setFps(engine.getFps());
                }, 1000);

                return () => {
                    clearInterval(timerHandle);
                };
            }
        }

        setFps(undefined);
    }, [props.camera, props.displayFrameRate]);

    // Call initialized and include the hook to takeScreenshot
    if (props.initialized) {
        props.initialized(
            {
                takeScreenshot: () => {
                    if (!screenshotPromise) {
                        let resolutionFunctions: { resolve: (data: string) => void, reject: () => void } | undefined;
                        const promise = new Promise<string>((resolutionFunc, rejectionFunc) => {
                            resolutionFunctions = {resolve: resolutionFunc, reject: rejectionFunc};
                        });

                        if (resolutionFunctions) {
                            setScreenshotPromise({ promise: promise, resolve: resolutionFunctions.resolve, reject: resolutionFunctions.reject });
                        }

                        UIManager.dispatchViewManagerCommand(
                            findNodeHandle(engineViewRef.current),
                            UIManager.getViewManagerConfig("EngineView").Commands["takeSnapshot"],
                            []);

                        return promise;
                    }

                    return screenshotPromise.promise;
            }
        });
    }

    // Handle snapshot data returned.
    const snapshotDataReturnedHandler = (event: SyntheticEvent) => {
        let { data } = event.nativeEvent;
        if (screenshotPromise) {
            screenshotPromise.resolve(data);
            setScreenshotPromise(undefined);
        }
    }

    if (!failedInitialization) {
        return (
            <View style={[props.style, {overflow: "hidden"}]}>
                <NativeEngineView ref={engineViewRef} style={{flex: 1}} snapshotDataReturned={snapshotDataReturnedHandler} />
                { fps && <Text style={{color: 'yellow', position: 'absolute', margin: 10, right: 0, top: 0}}>FPS: {Math.round(fps)}</Text> }
            </View>
        );
    } else {
        return (
            <View style={{flex: 1, justifyContent: 'center', alignItems: 'center'}}>
                <Text style={{fontSize: 24}}>Could not initialize Babylon Native.</Text>
                { isRemoteDebuggingEnabled && <Text style={{fontSize: 12}}>React Native remote debugging does not work with Babylon Native.</Text> }
            </View>
        );
    }
}