package org.mozilla.vrbviewer;

import android.app.Activity;
import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.widget.TextView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MainActivity extends Activity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    GLSurfaceView mView;
    Runnable mResumeRunnable = new Runnable() {
        @Override
        public void run() {
            synchronized (this) {
                activityResumed();
                notifyAll();
            }
        }
    };

    Runnable mPauseRunnable = new Runnable() {
        @Override
        public void run() {
            synchronized (this) {
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
        mView.setEGLConfigChooser(8, 8, 8, 0, 16, 0);
        mView.setRenderer(new GLSurfaceView.Renderer() {
            @Override
            public void onSurfaceCreated(GL10 gl10, EGLConfig eglConfig) {
            }

            @Override
            public void onSurfaceChanged(GL10 gl10, int width, int height) {
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
                initializeJava(getAssets());
            }
        });
    }

    @Override
    public void onDestroy() {
        mView.queueEvent(new Runnable() {
            @Override
            public void run() {
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
    native void updateViewport(int width, int height);
    native void draw();
    native void initializeJava(AssetManager aManager);
    native void shutdownJava();
    native void activityResumed();
    native void activityPaused();
}
