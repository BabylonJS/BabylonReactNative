package com.babylonreactnative;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.facebook.react.module.annotations.ReactModule;
import com.facebook.react.uimanager.SimpleViewManager;
import com.facebook.react.uimanager.ThemedReactContext;
import com.facebook.react.uimanager.ViewManagerDelegate;
import com.facebook.react.viewmanagers.EngineViewNativeComponentManagerDelegate;
import com.facebook.react.viewmanagers.EngineViewNativeComponentManagerInterface;

@ReactModule(name = EngineViewManager.NAME)
public final class EngineViewManager extends SimpleViewManager<EngineView> implements EngineViewNativeComponentManagerInterface<EngineView> {
    private final ViewManagerDelegate<EngineView> mDelegate;

    static final String NAME = "EngineViewNativeComponent";
    @NonNull
    @Override
    public String getName() {
        return EngineViewManager.NAME;
    }

    public EngineViewManager() {
        mDelegate = new EngineViewNativeComponentManagerDelegate<>(this);
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
    public void onDropViewInstance(@NonNull EngineView view) {
        super.onDropViewInstance(view);
        // TODO: Native view specific cleanup
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
