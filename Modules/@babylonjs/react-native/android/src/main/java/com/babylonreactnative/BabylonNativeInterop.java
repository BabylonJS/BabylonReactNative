package com.babylonreactnative;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.view.MotionEvent;
import android.view.Surface;

import com.facebook.react.bridge.ActivityEventListener;
import com.facebook.react.bridge.LifecycleEventListener;
import com.facebook.react.bridge.ReactContext;
import com.facebook.react.turbomodule.core.interfaces.CallInvokerHolder;

public final class BabylonNativeInterop {

    private static class BabylonNative {
        static {
            System.loadLibrary("BabylonNative");
        }

        public static native void initialize(Context context, long jsiRuntimeRef, CallInvokerHolder jsCallInvokerHolder);
        public static native void deinitialize();
        public static native void setCurrentActivity(Activity activity);
        public static native void pause();
        public static native void resume();
        public static native void updateView(Surface surface);
        public static native void setPointerButtonState(int pointerId, int buttonId, boolean isDown, int x, int y);
        public static native void setPointerPosition(int pointerId, int x, int y);
    }

    private static ReactContext currentContext;

    private final static LifecycleEventListener lifeCycleEventListener = new LifecycleEventListener() {
        @Override
        public void onHostResume() {
            BabylonNative.setCurrentActivity(BabylonNativeInterop.currentContext.getCurrentActivity());
            BabylonNative.resume();
        }

        @Override
        public void onHostPause() {
            BabylonNative.pause();
        }

        @Override
        public void onHostDestroy() {
            BabylonNative.deinitialize();
        }
    };

    private final static ActivityEventListener activityEventListener = new ActivityEventListener() {
        @Override
        public void onActivityResult(Activity activity, int requestCode, int resultCode, Intent data) {
            // Nothing to do here
        }

        @Override
        public void onNewIntent(Intent intent) {
            BabylonNative.setCurrentActivity(BabylonNativeInterop.currentContext.getCurrentActivity());
        }
    };

    public static void initialize(ReactContext reactContext) {
        BabylonNativeInterop.currentContext = reactContext;

        long jsiRuntimeRef = BabylonNativeInterop.currentContext.getJavaScriptContextHolder().get();
        CallInvokerHolder jsCallInvokerHolder = BabylonNativeInterop.currentContext.getCatalystInstance().getJSCallInvokerHolder();
        BabylonNative.initialize(BabylonNativeInterop.currentContext, jsiRuntimeRef, jsCallInvokerHolder);

        BabylonNativeInterop.currentContext.removeLifecycleEventListener(lifeCycleEventListener);
        BabylonNativeInterop.currentContext.addLifecycleEventListener(lifeCycleEventListener);
    }

    public static void deinitialize() {
        BabylonNative.deinitialize();
    }

    public static void updateView(Surface surface) {
        BabylonNative.updateView(surface);
    }

    public static void reportMotionEvent(MotionEvent motionEvent) {
        int maskedAction = motionEvent.getActionMasked();
        boolean isPointerDown = maskedAction == MotionEvent.ACTION_DOWN || maskedAction == MotionEvent.ACTION_POINTER_DOWN;
        boolean isPointerUp = maskedAction == MotionEvent.ACTION_UP || maskedAction == MotionEvent.ACTION_POINTER_UP;
        boolean isPointerMove = maskedAction == MotionEvent.ACTION_MOVE;

        if (isPointerDown || isPointerUp) {
            int pointerIndex = motionEvent.getActionIndex();
            int pointerId = motionEvent.getPointerId(pointerIndex);
            int buttonId = motionEvent.getActionButton();
            int x = (int)motionEvent.getX(pointerIndex);
            int y = (int)motionEvent.getY(pointerIndex);
            BabylonNative.setPointerButtonState(pointerId, buttonId, isPointerDown, x, y);
        } else if (isPointerMove) {
            for (int pointerIndex = 0; pointerIndex < motionEvent.getPointerCount(); pointerIndex++) {
                int pointerId = motionEvent.getPointerId(pointerIndex);
                int x = (int)motionEvent.getX(pointerIndex);
                int y = (int)motionEvent.getY(pointerIndex);
                BabylonNative.setPointerPosition(pointerId, x, y);
            }
        }
    }
}
