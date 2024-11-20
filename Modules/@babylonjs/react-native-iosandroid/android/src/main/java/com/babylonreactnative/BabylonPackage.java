package com.babylonreactnative;

import androidx.annotation.NonNull;

import java.util.Arrays;
import java.util.List;

import com.facebook.react.ReactPackage;
import com.facebook.react.bridge.NativeModule;
import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.uimanager.ViewManager;

public class BabylonPackage implements ReactPackage {
    @NonNull
    @Override
    public List<NativeModule> createNativeModules(ReactApplicationContext reactContext) {
        return Arrays.<NativeModule>asList(new BabylonModule(reactContext));
    }

    @NonNull
    @Override
    public List<ViewManager> createViewManagers(ReactApplicationContext reactContext) {
        // We have two implementations of EngineViewManager, one per architecture.
        // Implementation source code is determined by the sourceSets in the build.gradle file.
        return Arrays.<ViewManager>asList(new EngineViewManager());
    }
}
