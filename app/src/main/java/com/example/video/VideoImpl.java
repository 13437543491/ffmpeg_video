package com.example.video;

import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class VideoImpl implements SurfaceHolder.Callback {

    private static final int STATE_NORMAL = 0;
    private static final int STATE_PREPARE = 1;
    private static final int STATE_START = 2;
    private static final int STATE_PAUSE = 3;
    private static final int STATE_STOP = 4;

    private int mState = STATE_NORMAL;
    private boolean isPrepare = false;
    private OnVideoPlayCallback mCallback;

    private Surface mSurface;

    public VideoImpl() {

    }

    public void prepare() {
        nativePrepare();
    }

    public void start(String filePath) {
        nativeStart(filePath, this.mSurface);

//        if (!isPrepare) {
//            return;
//        }
//        if (mState != STATE_START) {
//        }
    }

    public void setmSurface(Surface surface) {
        this.mSurface = surface;
    }

    public void pause() {
        nativePause();
    }

    public void stop() {
        nativeStop();

    }

    public void setCallback(OnVideoPlayCallback callback) {
        this.mCallback = callback;
    }

    private void nativePrepareCallback(int state, String error) {
        if (state == 0) {
            mState = STATE_PREPARE;
            if (mCallback != null) {
                mCallback.onPrepare();
            }
        } else {
            mCallback.onError(error);
        }
    }

    private void nativeStartCallback(int state, String error) {
        if (state == 0) {
            mState = STATE_START;
            if (mCallback != null) {
                mCallback.onStart();
            }
        } else {
            mCallback.onError(error);
        }
    }

    private void nativePauseCallback(int state, String error) {
        if (state == 0) {
            mState = STATE_PAUSE;
            if (mCallback != null) {
                mCallback.onPause();
            }
        } else {
            mCallback.onError(error);
        }
    }

    private void nativeStopCallback(int state, String error) {
        if (state == 0) {
            mState = STATE_STOP;
            if (mCallback != null) {
                mCallback.onStop();
            }
        } else {
            mCallback.onError(error);
        }
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }

    private native void nativePrepare();

    private native void nativeStart(String path, Surface surface);

    private native void nativePause();

    private native void nativeStop();

    public interface OnVideoPlayCallback {
        void onPrepare();

        void onStart();

        void onPause();

        void onStop();

        void onError(String error);
    }
}
