package org.mozilla.vrbviewer;

import android.app.Activity;
import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.TextView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MainActivity extends Activity {

    static final String LOGTAG = "VRB";
    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private GLSurfaceView mView;
    private TextView mFrameRate;
    private int mFrameCount;
    private long mLastFrameTime = System.currentTimeMillis();


    final Runnable mResumeRunnable = new Runnable() {
        @Override
        public void run() {
            synchronized (this) {
                Log.e(LOGTAG, "activityResumed: " + Thread.currentThread().toString());
                activityResumed();
                notifyAll();
            }
        }
    };

    final Runnable mPauseRunnable = new Runnable() {
        @Override
        public void run() {
            synchronized (this) {
                Log.e(LOGTAG, "activityPaused: " + Thread.currentThread().toString());
                activityPaused();
                notifyAll();
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mFrameRate = findViewById(R.id.frame_rate_text);
        mView = findViewById(R.id.gl_view);
        mView.setEGLContextClientVersion(3);
        mView.setEGLConfigChooser(8, 8, 8, 8, 16, 0);
        mView.setRenderer(new GLSurfaceView.Renderer() {
            @Override
            public void onSurfaceCreated(GL10 gl10, EGLConfig eglConfig) {
                Log.e(LOGTAG, "onSurfaceCreated: " + Thread.currentThread().toString());
            }

            @Override
            public void onSurfaceChanged(GL10 gl10, int width, int height) {
                Log.e(LOGTAG, "onSurfaceChanged: " + Thread.currentThread().toString());
                updateViewport(width, height);
            }

            @Override
            public void onDrawFrame(GL10 gl10) {

                mFrameCount++;
                long ctime = System.currentTimeMillis();
                if ((ctime - mLastFrameTime) >= 1000) {
                    final int value =  Math.round(mFrameCount / ((ctime - mLastFrameTime) / 1000.0f));
                    mLastFrameTime = ctime;
                    mFrameCount = 0;
                    runOnUiThread(() -> mFrameRate.setText(String.valueOf(value)));
                }
                draw();
            }
        });

        mView.queueEvent(new Runnable() {
            @Override
            public void run() {
                Log.e(LOGTAG, "initializeJava: " + Thread.currentThread().toString());
                initializeJava(getAssets());
            }
        });
    }

    @Override
    public void onDestroy() {
        mView.queueEvent(new Runnable() {
            @Override
            public void run() {
                Log.e(LOGTAG, "onDestroy: " + Thread.currentThread().toString());
                shutdownJava();
            }
        });
        super.onDestroy();
    }

    @Override
    public void onResume() {
        super.onResume();
        mView.onResume();
        synchronized (mResumeRunnable) {
            mView.queueEvent(mResumeRunnable);
            try {
                mResumeRunnable.wait();
            } catch(Exception e){

            }
        }
        setImmersiveSticky();
    }

    @Override
    public void onPause() {
        synchronized (mPauseRunnable) {
            mView.queueEvent(mPauseRunnable);
            try {
                mPauseRunnable.wait();
            } catch(Exception e){

            }
        }
        mView.onPause();
        super.onPause();
    }

    void setImmersiveSticky() {
        getWindow()
                .getDecorView()
                .setSystemUiVisibility(
                        View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                                | View.SYSTEM_UI_FLAG_FULLSCREEN
                                | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
    }

    @Override
    public boolean onTouchEvent(MotionEvent aEvent) {
        if (aEvent.getActionIndex() != 0) {
            Log.e(LOGTAG,"aEvent.getActionIndex()=" + aEvent.getActionIndex());
            return false;
        }

        int action = aEvent.getAction();
        boolean down;
        if (action == MotionEvent.ACTION_DOWN) {
            down = true;
        } else if (action == MotionEvent.ACTION_UP) {
            down = false;
        } else if (action == MotionEvent.ACTION_MOVE) {
            down = true;
        } else {
            return false;
        }

        final boolean isDown = down;

        final float xx = aEvent.getX(0);
        final float yy = aEvent.getY(0);
        mView.queueEvent(() -> touchEvent(isDown, xx, yy));
        return true;
    }

    private native void updateViewport(int width, int height);
    private native void draw();
    private native void initializeJava(AssetManager aManager);
    private native void shutdownJava();
    private native void activityResumed();
    private native void activityPaused();
    private native void touchEvent(boolean isDown, float xx, float yy);
}
