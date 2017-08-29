package com.johnsoft.imgproc.camera;

import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;

import android.content.pm.ActivityInfo;
import android.graphics.Color;
import android.graphics.Point;
import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.widget.FrameLayout;

/**
 * Native Demo. Test on Google Nexus 6P.
 * @author John Kenrinus Lee
 * @version 2017-08-18
 */
public class NativeCameraActivity extends AppCompatActivity implements CameraView.OnFrameRgbaDataCallback {
    protected static int NH = 960;
    protected static int NW = 1280;
    protected static long delayMillis = 500L;
    protected static boolean globalFullScreen = true;

    private Handler handler;
    private int cameraIndex;
    private FrameLayout layout;
    private volatile boolean hadFullView;
    private volatile boolean capture;
    private CameraView.FrameCallbackThread callbackThread;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        final Window window = getWindow();
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        window.addFlags(WindowManager.LayoutParams.FLAG_HARDWARE_ACCELERATED);
        if (globalFullScreen) {
            window.requestFeature(Window.FEATURE_NO_TITLE);
            window.getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                    | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                    | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                    | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                    | View.SYSTEM_UI_FLAG_FULLSCREEN);
            window.setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                    WindowManager.LayoutParams.FLAG_FULLSCREEN);
            final android.app.ActionBar actionBar = getActionBar();
            if (actionBar != null) {
                actionBar.hide();
            }
            final android.support.v7.app.ActionBar supportActionBar = getSupportActionBar();
            if (supportActionBar != null) {
                supportActionBar.hide();
            }
        }

        fullContentView();

        handler = new Handler();
        hadFullView = false;

        final CameraManager.DefaultPreviewSizeChooser previewSizeChooser = new CameraManager
                .DefaultPreviewSizeChooser(NW, NH);
        cameraIndex = CameraManager.cameraIndex(false);
        CameraManager.singleInstance.flag(cameraIndex, true, true, false);
        CameraManager.singleInstance.open(cameraIndex, previewSizeChooser);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        CameraManager.singleInstance.close(cameraIndex);
        if (callbackThread != null) {
            callbackThread.quit();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                CameraManager.singleInstance.startPreview(cameraIndex);
                fullCameraViews();
            }
        }, delayMillis);
    }

    @Override
    protected void onPause() {
        super.onPause();
        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                CameraManager.singleInstance.stopPreview(cameraIndex);
            }
        }, delayMillis);
    }

    @Override
    public void onFrameRgbaData(ByteBuffer rgba) {
        if (capture) {
            capture = false;
            try {
                final String filePath = "/sdcard/nctest" + System.currentTimeMillis()
                        + "_le_" + NW + 'x' + NH + ".rgba";
                final FileChannel channel = new RandomAccessFile(filePath, "rwd").getChannel();
                channel.write(rgba);
                channel.close();
                Log.i("NativeCameraActivity", "Capture rgba data in " + filePath);
            } catch (IOException e) {
                Log.w("NativeCameraActivity", e);
            }
        }
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_VOLUME_DOWN) {
            return true;
        }
        return super.onKeyDown(keyCode, event);
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_VOLUME_DOWN) {
            capture = true;
            return true;
        }
        return super.onKeyUp(keyCode, event);
    }

    private void fullContentView() {
        final FrameLayout.LayoutParams lpFrameLayout = new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT,
                Gravity.CENTER);
        layout = new FrameLayout(NativeCameraActivity.this);
        layout.setLayoutParams(lpFrameLayout);
        layout.setBackgroundColor(Color.BLACK);
        setContentView(layout, lpFrameLayout);
    }

    private void fullCameraViews() {
        if (!hadFullView) {
            hadFullView = true;
            callbackThread = new CameraView.FrameCallbackThread(this,
                    NW * NH * 4, DirectByteBuffers.createNativeDirectMemory());
            callbackThread.start();
            Point size = CameraManager.singleInstance.getFrameSize(cameraIndex);
            if (size == null) {
                Log.w("CameraActivity", "camera frame size return null!");
                size = new Point(0, 0);
            }
            final FrameLayout.LayoutParams lp = new FrameLayout.LayoutParams(
                    size.x, size.y);
            lp.gravity = Gravity.CENTER;
            final CameraView cameraView = new CameraNativeView(NativeCameraActivity.this);
            cameraView.markCameraIndex(cameraIndex).markAsFrontCamera(false);
            cameraView.setShaderSourceCode(null, CameraActivity.fragShaderCode2);
            cameraView.setOnFrameRgbaDataCallback(callbackThread);
            cameraView.setLayoutParams(lp);
            layout.addView(cameraView, 0, lp);
        }
    }
}
