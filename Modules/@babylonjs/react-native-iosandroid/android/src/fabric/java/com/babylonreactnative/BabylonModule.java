package com.babylonreactnative;

import androidx.annotation.NonNull;

import com.facebook.react.bridge.Promise;
import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.module.annotations.ReactModule;

@ReactModule(name = BabylonModule.NAME)
public final class BabylonModule extends NativeBabylonModuleSpec {
    static final String NAME = "EngineViewNativeComponent";

    BabylonModule(ReactApplicationContext reactContext) {
        super(reactContext);
    }

    @NonNull
    @Override
    public String getName() {
       return BabylonModule.NAME;
    }

    @Override
    public void initialize(Promise promise) {
//        this.getReactApplicationContext().runOnJSQueueThread(() -> {
//            BabylonNativeInterop.initialize(this.getReactApplicationContext());
//            promise.resolve(null);
//        });
    }

    @Override
    public void resetView(Promise promise) {
//        this.getReactApplicationContext().runOnUiQueueThread(() -> {
//            BabylonNativeInterop.resetView();
//            promise.resolve(null);
//        });
    }
}
