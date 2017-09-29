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
package com.johnsoft.imgproc.alg;

import java.util.ArrayList;

import android.app.ActivityManager;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.ConfigurationInfo;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.util.Pair;
import android.support.v4.view.PagerAdapter;
import android.support.v4.view.ViewPager;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.Toast;

/**
 * Gpu Activity
 *
 * @author John Kenrinus Lee
 * @version 2017-09-27
 */
public class ImageGpuProcessActivity extends AppCompatActivity {
    private static final String TAG = "ImageGpuProcessActivity";

    private static final String TYPE_DEFAULT = "origin";

    private GpuViewPager viewPager;
    private GpuPagerAdapter pagerAdapter;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);

        final ActivityManager activityManager =
                (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
        final ConfigurationInfo configurationInfo =
                activityManager.getDeviceConfigurationInfo();
        final int reqGlEsVersion = configurationInfo.reqGlEsVersion;
        Log.i(TAG, "Support OpenGLES Version: " + Integer.toHexString(reqGlEsVersion));

        final ActionBar actionBar = getSupportActionBar();
        if (actionBar != null) {
            actionBar.setSubtitle("Long click image to custom process");
        }

        viewPager = new GpuViewPager(this);
        final FrameLayout.LayoutParams lp = new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT,
                Gravity.CENTER);
        viewPager.setLayoutParams(lp);
        setContentView(viewPager, lp);
        pagerAdapter = new GpuPagerAdapter();
        viewPager.setAdapter(pagerAdapter);

        if (configurationInfo.reqGlEsVersion < 0x30000) {
            Toast.makeText(this, "OpenGLES 3.0+ required", Toast.LENGTH_LONG).show();
            return;
        }

        boolean originLoaded = false;
        final Intent intent = getIntent();
        if (intent != null) {
            final String[] strings = intent.getStringArrayExtra(TYPE_DEFAULT);
            final int len;
            if (strings != null && (len = strings.length) > 0) {
                for (int i = 0; i < len; i++) {
                    final String originImagePath = strings[i];
                    if (originImagePath != null && !originImagePath.trim().isEmpty()) {
                        pagerAdapter.addItemObject(new Pair<>(originImagePath, TYPE_DEFAULT));
                        originLoaded = true;
                    }
                }
            }
        }
        if (!originLoaded) {
            pagerAdapter.addItemObject(new Pair<>("file:///android_asset/image_test.jpg", TYPE_DEFAULT));
        }
    }

    private static class GpuViewPager extends ViewPager {
        public GpuViewPager(Context context) {
            super(context);
        }

        public GpuViewPager(Context context, AttributeSet attrs) {
            super(context, attrs);
        }

        @Override
        public boolean onInterceptTouchEvent(MotionEvent ev) {
            try {
                return super.onInterceptTouchEvent(ev);
            } catch (IllegalArgumentException e) {
                return false;
            }
        }
    }

    private static class GpuPagerAdapter extends PagerAdapter {
        private final ArrayList<Pair<String, String>> list = new ArrayList<>();

        GpuPagerAdapter() {
        }

        public void addItemObject(Pair<String, String> item) {
            list.add(item);
            notifyDataSetChanged();
        }

        public void removeItemObject(Pair<String, String> item) {
            list.remove(item);
            notifyDataSetChanged();
        }

        @Override
        public Object instantiateItem(ViewGroup container, int position) {
            final FrameLayout.LayoutParams lpContent = new FrameLayout.LayoutParams(
                    ViewGroup.LayoutParams.WRAP_CONTENT,
                    ViewGroup.LayoutParams.WRAP_CONTENT,
                    Gravity.CENTER);

            final Context context = container.getContext();

            final GpuImageView imageView = new GpuImageView(context);
            imageView.parsePath(list.get(position).first, list.get(position).second);
            imageView.setLayoutParams(lpContent);
            container.addView(imageView, lpContent);
            return imageView;
        }

        @Override
        public void destroyItem(ViewGroup container, int position, Object object) {
            container.removeView((View) object);
        }

        @Override
        public int getCount() {
            return list.size();
        }

        @Override
        public boolean isViewFromObject(View view, Object object) {
            return view == object;
        }
    }
}
