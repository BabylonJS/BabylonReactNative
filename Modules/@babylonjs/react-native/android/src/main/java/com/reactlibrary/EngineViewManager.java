package com.reactlibrary;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.facebook.react.bridge.ReadableArray;
import com.facebook.react.common.MapBuilder;
import com.facebook.react.uimanager.SimpleViewManager;
import com.facebook.react.uimanager.ThemedReactContext;

import java.util.Map;

public final class EngineViewManager extends SimpleViewManager<EngineView> {
    public static final int COMMAND_TAKE_SNAPSHOT = 0;

    @NonNull
    @Override
    public String getName() {
        return "EngineView";
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
    public Map<String,Integer> getCommandsMap() {
        return MapBuilder.of(
            "takeSnapshot",
            COMMAND_TAKE_SNAPSHOT
        );
    }

    @Override
    public void receiveCommand(final EngineView view, int commandId, ReadableArray args) {
    // This will be called whenever a command is sent from react-native.
    switch (commandId) {
      case COMMAND_TAKE_SNAPSHOT:
        view.takeSnapshot();
        break;
    }
  }
}