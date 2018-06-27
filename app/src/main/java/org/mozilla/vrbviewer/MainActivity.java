package org.mozilla.vrbviewer;

import android.app.Activity;
import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MainActivity extends Activity {

    static final String LOGTAG = "VRB";
    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    GLSurfaceView mView;
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

    private native void updateViewport(int width, int height);
    private native void draw();
    private native void initializeJava(AssetManager aManager);
    private native void shutdownJava();
    private native void activityResumed();
    private native void activityPaused();
}
