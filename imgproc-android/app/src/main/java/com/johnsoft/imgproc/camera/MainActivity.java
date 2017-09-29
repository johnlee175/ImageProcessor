/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements. See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache license, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the license for the specific language governing permissions and
 * limitations under the license.
 */
package com.johnsoft.imgproc.camera;

import com.johnsoft.imgproc.R;
import com.johnsoft.imgproc.alg.ImageGpuProcessActivity;
import com.johnsoft.imgproc.camera.simple.SimpleCameraActivity;

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

    public void startNativeCamera(View view) {
        startActivity(new Intent(this, NativeCameraActivity.class));
    }

    public void startSimpleCamera(View view) {
        startActivity(new Intent(this, SimpleCameraActivity.class));
    }

    public void startImageGpuProcess(View view) {
        startActivity(new Intent(this, ImageGpuProcessActivity.class));
    }
}
