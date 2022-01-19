package com.babylonreactnative;

import android.annotation.TargetApi;
import android.graphics.Bitmap;
import android.graphics.SurfaceTexture;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Base64;
import android.view.MotionEvent;
import android.view.PixelCopy;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.TextureView;
import android.view.View;
import android.widget.FrameLayout;

import androidx.annotation.NonNull;

import com.facebook.react.bridge.ReactContext;
import com.facebook.react.uimanager.UIManagerModule;
import com.facebook.react.uimanager.events.EventDispatcher;

import java.io.ByteArrayOutputStream;

public final class EngineView extends FrameLayout implements TextureView.SurfaceTextureListener, View.OnTouchListener {
    private static final FrameLayout.LayoutParams childViewLayoutParams = new FrameLayout.LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);
    private TextureView textureView;
    private Surface surface;
    private SurfaceView xrSurfaceView;
    private final EventDispatcher reactEventDispatcher;
    private Runnable renderRunnable;

    public EngineView(ReactContext reactContext) {
        super(reactContext);

        this.textureView = new TextureView(reactContext);
        this.textureView.setLayoutParams(EngineView.childViewLayoutParams);
        this.textureView.setSurfaceTextureListener(this);
        this.textureView.setOpaque(true);
        this.addView(this.textureView);

        this.xrSurfaceView = new SurfaceView(reactContext);
        this.xrSurfaceView.setLayoutParams(childViewLayoutParams);
        this.xrSurfaceView.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                // surfaceChanged is also called when the surface is created, so just do all the handling there
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
                BabylonNativeInterop.updateXRView(holder.getSurface());
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {
                BabylonNativeInterop.updateXRView(null);
            }
        });
        this.xrSurfaceView.setVisibility(View.INVISIBLE);
        this.addView(this.xrSurfaceView);

        this.setOnTouchListener(this);

        this.reactEventDispatcher = reactContext.getNativeModule(UIManagerModule.class).getEventDispatcher();
    }

    // ------------------------------------
    // TextureView related

    public void setIsTransparent(Boolean isTransparent) {
        this.textureView.setOpaque(!isTransparent);
    }

    @Override
    public void onSurfaceTextureAvailable(@NonNull SurfaceTexture surfaceTexture, int i, int i1) {
        this.startRenderLoop();
        surface = new Surface(surfaceTexture);
        BabylonNativeInterop.updateView(surface);
    }

    @Override
    public void onSurfaceTextureSizeChanged(@NonNull SurfaceTexture surfaceTexture, int i, int i1) {
        surface = new Surface(surfaceTexture);
        BabylonNativeInterop.updateView(surface);
    }

    @Override
    public boolean onSurfaceTextureDestroyed(@NonNull SurfaceTexture surfaceTexture) {
        this.stopRenderLoop();
        surface.release();
        return false;
    }

    @Override
    public void onSurfaceTextureUpdated(@NonNull SurfaceTexture surfaceTexture) {
        surface = new Surface(surfaceTexture);
        BabylonNativeInterop.updateView(surface);
    }

    // ------------------------------------
    // Common

    @Override
    public boolean onTouch(View view, MotionEvent motionEvent) {
        BabylonNativeInterop.reportMotionEvent(motionEvent);
        return true;
    }

    @TargetApi(24)
    public void takeSnapshot() {
        // Only supported on API level 24 and up, return a blank image.
        if (android.os.Build.VERSION.SDK_INT < Build.VERSION_CODES.N) {
            SnapshotDataReturnedEvent snapshotEvent = new SnapshotDataReturnedEvent(this.getId(), "");
            reactEventDispatcher.dispatchEvent(snapshotEvent);
        }

        // Create a bitmap that matches the width and height of the EngineView.
        final Bitmap bitmap = Bitmap.createBitmap(
                getWidth(),
                getHeight(),
                Bitmap.Config.ARGB_8888);

        // Offload the snapshot worker to a helper thread.
        final HandlerThread helperThread = new HandlerThread("ScreenCapture",-1);
        helperThread.start();
        final Handler helperThreadHandler = new Handler(helperThread.getLooper());

        Surface sourceSurface = this.surface;
        if (BabylonNativeInterop.isXRActive()) {
            sourceSurface = this.xrSurfaceView.getHolder().getSurface();
        }
        PixelCopy.request(sourceSurface, bitmap, getOnPixelCopyFinishedListener(bitmap, helperThread), helperThreadHandler);
    }

    // ---------------------------------------------------------------------------------------------
    // Returns the listener for the PixelCopy.request function call
    @NonNull
    private PixelCopy.OnPixelCopyFinishedListener getOnPixelCopyFinishedListener(Bitmap bitmap, HandlerThread helperThread) {
        return (copyResult) -> {
            // If the pixel copy was a success then convert the image to a base 64 encoded jpeg and fire the event.
            String encoded = "";
            if (copyResult == PixelCopy.SUCCESS) {
                ByteArrayOutputStream byteArrayStream = new ByteArrayOutputStream();
                bitmap.compress(Bitmap.CompressFormat.JPEG, 80, byteArrayStream);
                byte[] byteArray = byteArrayStream.toByteArray();
                bitmap.recycle();
                encoded = Base64.encodeToString(byteArray, Base64.DEFAULT);
            }
            SnapshotDataReturnedEvent snapshotEvent = new SnapshotDataReturnedEvent(this.getId(), encoded);
            reactEventDispatcher.dispatchEvent(snapshotEvent);
            helperThread.quitSafely();
        };
    }

    private void startRenderLoop() {
        if(this.renderRunnable == null){
            this.renderRunnable = new Runnable() {
                @Override
                public void run() {
                    if (BabylonNativeInterop.isXRActive()) {
                        EngineView.this.xrSurfaceView.setVisibility(View.VISIBLE);
                    } else {
                        EngineView.this.xrSurfaceView.setVisibility(View.INVISIBLE);
                    }
                    BabylonNativeInterop.renderView();
                    EngineView.this.postOnAnimation(this);
                }
            };
            this.postOnAnimation(this.renderRunnable);
        }
    }

    private void stopRenderLoop() {
        this.removeCallbacks(this.renderRunnable);
        this.renderRunnable = null;
    }
}
