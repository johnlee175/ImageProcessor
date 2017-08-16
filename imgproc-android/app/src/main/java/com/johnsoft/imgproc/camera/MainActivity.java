package com.johnsoft.imgproc.camera;

import com.johnsoft.imgproc.R;

import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.View;

/**
 * Entry Activity
 * @author John Kenrinus Lee
 * @version 2017-08-18
 */
public class MainActivity extends AppCompatActivity {
    @Override
    protected void onCreate(@Nullable Bundle bundle) {
        super.onCreate(bundle);
        setContentView(R.layout.activity_main);
    }

    public void startCamera(View view) {
        startActivity(new Intent(this, CameraActivity.class));
    }

    public void startPip(View view) {
        startActivity(new Intent(this, PipActivity.class));
    }
}
