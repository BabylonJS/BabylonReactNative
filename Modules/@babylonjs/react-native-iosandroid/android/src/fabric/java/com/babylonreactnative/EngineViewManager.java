package com.babylonreactnative;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.facebook.react.module.annotations.ReactModule;
import com.facebook.react.uimanager.SimpleViewManager;
import com.facebook.react.uimanager.ThemedReactContext;
import com.facebook.react.uimanager.ViewManagerDelegate;
import com.facebook.react.viewmanagers.EngineViewManagerDelegate;
import com.facebook.react.viewmanagers.EngineViewManagerInterface;

@ReactModule(name = EngineViewManager.NAME)
public final class EngineViewManager extends SimpleViewManager<EngineView> implements EngineViewManagerInterface<EngineView> {
    private final ViewManagerDelegate<EngineView> mDelegate;

    static final String NAME = "EngineView";

    @NonNull
    @Override
    public String getName() {
        return EngineViewManager.NAME;
    }

    public EngineViewManager() {
        mDelegate = new EngineViewManagerDelegate<>(this);
    }

    @Nullable
    @Override
    protected ViewManagerDelegate<EngineView> getDelegate() {
        return mDelegate;
    }

    @NonNull
    @Override
    protected EngineView createViewInstance(@NonNull ThemedReactContext reactContext) {
        return new EngineView(reactContext);
    }

    @Override
    public void setIsTransparent(EngineView view, boolean value) {
        view.setIsTransparent(value);
    }

    @Override
    public void setAntiAliasing(EngineView view, int value) {
        view.setAntiAliasing(value);
    }

    @Override
    public void setAndroidView(EngineView view, @Nullable String value) {
        view.setAndroidView(value);
    }

    @Override
    public void takeSnapshot(EngineView view) {
        view.takeSnapshot();
    }
}
