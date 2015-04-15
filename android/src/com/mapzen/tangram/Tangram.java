package com.mapzen.tangram;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.app.Activity;
import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;
import android.opengl.GLSurfaceView.Renderer;
import android.util.DisplayMetrics;
import android.view.GestureDetector;
import android.view.GestureDetector.OnGestureListener;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.ScaleGestureDetector.OnScaleGestureListener;
import com.almeros.android.multitouch.RotateGestureDetector;
import com.almeros.android.multitouch.RotateGestureDetector.OnRotateGestureListener;
import com.almeros.android.multitouch.ShoveGestureDetector;
import com.almeros.android.multitouch.ShoveGestureDetector.OnShoveGestureListener;
import android.view.SurfaceHolder;

public class Tangram extends GLSurfaceView implements Renderer, OnScaleGestureListener, OnRotateGestureListener, OnGestureListener, OnShoveGestureListener {

    static {
        System.loadLibrary("c++_shared");
        System.loadLibrary("tangram");
    }

    private static native void init(AssetManager assetManager);
    private static native void resize(int width, int height);
    private static native void update(float dt);
    private static native void render();
    private static native void teardown();
    private static native void onContextDestroyed();
    private static native void setPixelScale(float scale);
    private static native void handleTapGesture(float posX, float posY);
    private static native void handleDoubleTapGesture(float posX, float posY);
    private static native void handlePanGesture(float startX, float startY, float endX, float endY);
    private static native void handlePinchGesture(float posX, float posY, float scale);
    private static native void handleRotateGesture(float posX, float posY, float rotation);
    private static native void handleShoveGesture(float distance);

    private long time = System.nanoTime();
    private boolean contextDestroyed = false;
    private AssetManager assetManager;
    private GestureDetector gestureDetector;
    private ScaleGestureDetector scaleGestureDetector;
    private RotateGestureDetector rotateGestureDetector;
    private ShoveGestureDetector shoveGestureDetector;
    private DisplayMetrics displayMetrics = new DisplayMetrics();

    public Tangram(Activity mainApp) {
        super(mainApp);
        
        setEGLContextClientVersion(2);
        setPreserveEGLContextOnPause(true);
        setEGLConfigChooser(8, 8, 8, 8, 24, 0);
        setRenderer(this);
        
        mainApp.getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
        
        this.assetManager = mainApp.getAssets();
        this.gestureDetector = new GestureDetector(mainApp, this);
        this.scaleGestureDetector = new ScaleGestureDetector(mainApp, this);
        this.rotateGestureDetector = new RotateGestureDetector(mainApp, this);
        this.shoveGestureDetector = new ShoveGestureDetector(mainApp, this);
        
    }
    
    @Override
    public void onResume() {
        super.onResume();
    }
    
    public void onDestroy() {
        teardown();
    }
    
    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        contextDestroyed = true;
        super.surfaceDestroyed(holder);
    }
    
    @Override
    public boolean onTouchEvent(MotionEvent event) { 
        
        //Pass the event to gesture detectors
        if (gestureDetector.onTouchEvent(event) |
            scaleGestureDetector.onTouchEvent(event) |
            rotateGestureDetector.onTouchEvent(event) |
            shoveGestureDetector.onTouchEvent(event)) {
            return true;
        } else {
            return super.onTouchEvent(event);
        }
        
    }
    
    // GLSurfaceView.Renderer methods
    // ==============================

    public void onDrawFrame(GL10 gl) {
        long newTime = System.nanoTime();
        float delta = (newTime - time) / 1000000000.0f;
        time = newTime;

        update(delta);
        render();
    }

    public void onSurfaceChanged(GL10 gl, int width, int height) {
        setPixelScale(displayMetrics.density);
        resize(width, height);
    }

    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        
        if (contextDestroyed) {
            onContextDestroyed();
            contextDestroyed = false;
        }
        
        init(assetManager);
    }

    // GestureDetetor.OnGestureListener methods
    // ========================================
    
    public boolean onDown(MotionEvent event) {
        return true;
    }

    public boolean onDoubleTap(MotionEvent event) {
        final float x = event.getX(), y = event.getY();
        queueEvent(new Runnable() {
            public void run() {
                handleDoubleTapGesture(x, y);
            }
        });
        return true;
    }

    public boolean onScroll(MotionEvent e1, MotionEvent e2, final float distanceX, final float distanceY) {
        // Only pan for scrolling events with just one pointer; otherwise vertical scrolling will
        // cause a simultaneous shove gesture
        if (e1.getPointerCount() == 1 && e2.getPointerCount() == 1) {
            // We flip the signs of distanceX and distanceY because onScroll provides the distances
            // by which the view being scrolled should move, while handlePanGesture expects the 
            // distances by which the touch point has moved on the screen (these are opposite)
            final float x = e2.getX();
            final float y = e2.getY();
            queueEvent(new Runnable() {
                public void run() {
                    handlePanGesture(x + distanceX, y + distanceY, x, y);
                }
            });
        }
        return true;
    }

    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
        //not handled
        return false;
    }

    public void onLongPress(MotionEvent event) {
        //not handled
    }

    public void onShowPress(MotionEvent event) {
        //not handled
    }

    public boolean onSingleTapUp(MotionEvent event) {
        final float x = event.getX(), y = event.getY();
        queueEvent(new Runnable() {
            public void run() {
                handleTapGesture(x, y);
            }
        });
        return true;
    }

    // ScaleGestureDetector.OnScaleGestureListener methods
    // ===================================================
    
    public boolean onScaleBegin(ScaleGestureDetector detector) {
        return true;
    }

    public boolean onScale(ScaleGestureDetector detector) {
        final float x = detector.getFocusX(), y = detector.getFocusY();
        final float scale = detector.getScaleFactor();
        queueEvent(new Runnable() {
            public void run() {
                handlePinchGesture(x, y, scale);
            }
        });
        return true;
    }

    public void onScaleEnd(ScaleGestureDetector detector) {
        return;
    }

    // RotateGestureDetector.OnRotateGestureListener methods
    // =====================================================

    public boolean onRotateBegin(RotateGestureDetector detector) {
        return true;
    }

    public boolean onRotate(RotateGestureDetector detector) {
        final float x = scaleGestureDetector.getFocusX(), y = scaleGestureDetector.getFocusY();
        final float rotation = -detector.getRotationDegreesDelta() * (float)(Math.PI / 180);
        queueEvent(new Runnable() {
            public void run() {
                handleRotateGesture(x, y, rotation);
            }
        });
        return true;
    }

    public void onRotateEnd(RotateGestureDetector detector) {
        return;
    }

    // ShoveGestureDetector.OnShoveGestureListener methods
    // ===================================================

    public boolean onShoveBegin(ShoveGestureDetector detector) {
        return true;
    }

    public boolean onShove(ShoveGestureDetector detector) {
        final float shove = detector.getShovePixelsDelta() / displayMetrics.heightPixels;
        queueEvent(new Runnable() {
            public void run() {
                handleShoveGesture(shove);
            }
        });
        return true;
    }

    public void onShoveEnd(ShoveGestureDetector detector) {
        return;
    }
}

