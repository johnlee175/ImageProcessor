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

import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.Map;

import com.github.chrisbanes.photoview.PhotoView;
import com.johnsoft.App;
import com.johnsoft.alg.SimpleGpuProcProxy;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.AttributeSet;
import android.util.Log;
import android.view.ContextMenu;
import android.view.MenuItem;
import android.view.View;
import android.widget.ImageView;

/**
 * Gpu Image View
 * @author John Kenrinus Lee
 * @version 2017-09-28
 */
public class GpuImageView extends PhotoView
        implements View.OnLongClickListener,
        View.OnCreateContextMenuListener,
        MenuItem.OnMenuItemClickListener {

    private static final String LOG_TAG = "GpuImageView";

    private static String[] fsSources = new String[3];

    static {
        fsSources[0] = loadFs("hello1");
        fsSources[1] = loadFs("hello2");
        fsSources[2] = loadFs("hello3");
    }

    private static String loadFs(String namePrefix) {
        try {
            InputStream is = App.self().getAssets()
                    .open(namePrefix + ".fs");
            BufferedInputStream bis = new BufferedInputStream(is);
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            byte[] bytes = new byte[4096];
            int len;
            while ((len = bis.read(bytes, 0, bytes.length)) > 0) {
                baos.write(bytes, 0, len);
            }
            String result = new String(baos.toByteArray(), "UTF-8");
            baos.close();
            bis.close();
            return result;
        } catch (Throwable thr) {
            thr.printStackTrace();
            return null;
        }
    }

    private static Handler createWorkHandler() {
        HandlerThread thread = new HandlerThread("gpu-worker");
        thread.start();
        return new Handler(thread.getLooper());
    }

    private static final int GROUP_FILTER = 1;

    private final Map<String, Object> map = new HashMap<>();
    private final Handler handler = createWorkHandler();

    private String path;
    private String type;
    private int nextPointer;

    public GpuImageView(Context context) {
        super(context);
        initialize();
    }

    public GpuImageView(Context context, AttributeSet attr) {
        super(context, attr);
        initialize();
    }

    public GpuImageView(Context context, AttributeSet attr, int defStyle) {
        super(context, attr, defStyle);
        initialize();
    }

    protected void initialize() {
        setScaleType(ImageView.ScaleType.FIT_CENTER);
        setOnLongClickListener(this);
        setOnCreateContextMenuListener(this);
    }

    public void parsePath(String path, String type) {
        this.path = path;
        this.type = type;
        try {
            final String prefix = "file:///android_asset/";
            final int schema = path.indexOf(prefix);
            if (schema == 0) {
                setImageBitmap(BitmapFactory.decodeStream(getContext()
                        .getAssets().open(path.substring(schema + prefix.length()))));
            } else {
                final File file = new File(path);
                if (file.exists() && file.isFile() && file.canRead()) {
                    setImageBitmap(BitmapFactory.decodeFile(file.getAbsolutePath()));
                }
            }
        } catch (IOException e) {
            Log.w(LOG_TAG, e);
        }
    }

    @Override
    public boolean onLongClick(View v) {
        showContextMenu();
        return true;
    }

    @Override
    public void onCreateContextMenu(ContextMenu menu, View v,
                                    ContextMenu.ContextMenuInfo menuInfo) {
        menu.add(GROUP_FILTER, 0, 0, "Origin").setOnMenuItemClickListener(this);
        menu.add(GROUP_FILTER, 1, 1, "Negative").setOnMenuItemClickListener(this);
        menu.add(GROUP_FILTER, 2, 2, "Sketch").setOnMenuItemClickListener(this);
        menu.add(GROUP_FILTER, 3, 3, "WhiteSkin").setOnMenuItemClickListener(this);
    }

    @Override
    public boolean onMenuItemClick(MenuItem item) {
        final int itemId = item.getItemId();
        switch (itemId) {
            case 1:
            case 2:
            case 3:
                map.put("fragment_shader_source", fsSources[itemId - 1]);
                break;
            default: {
                parsePath(path, type);
            }
                return true;
        }

        handler.post(new Runnable() {
            @Override
            public void run() {
                final Bitmap resultBitmap = processImage();
                if (resultBitmap != null) {
                    post(new Runnable() {
                        @Override
                        public void run() {
                            final Bitmap bitmap = getOriginBitmap();
                            setImageBitmap(resultBitmap);
                            bitmap.recycle();
                        }
                    });
                }
            }
        });
        return true;
    }

    private Bitmap processImage() {
        final SimpleGpuProcProxy proc = ((ImageGpuProcessActivity) getContext()).getProc();
        final Bitmap bitmap = getOriginBitmap();
        final int width = bitmap.getWidth();
        final int height = bitmap.getHeight();
        final int[] data = new int[width * height * 4];
        bitmap.getPixels(data, 0, width, 0, 0, width, height);
        final int [] result = proc.imageProc(data, width, height, 0, map);
        if (result == null) {
            return null;
        }
        final Bitmap resultBitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        resultBitmap.setPixels(result, 0, width, 0, 0, width, height);
        return resultBitmap;
    }

    public Bitmap getOriginBitmap() {
        return ((BitmapDrawable) getDrawable()).getBitmap();
    }

    public void getShaderBitmap(final OnBitmapCallback callback) {
        if (callback == null) {
            return;
        }
        handler.post(new Runnable() {
            @Override
            public void run() {
                final Bitmap resultBitmap = processImage();
                post(new Runnable() {
                    @Override
                    public void run() {
                        callback.onBitmap(resultBitmap);
                    }
                });
            }
        });
    }

    public void applyNextShader() {
        map.put("fragment_shader_source", fsSources[nextPointer % fsSources.length]);
        ++nextPointer;
    }

    public interface OnBitmapCallback {
        void onBitmap(Bitmap bitmap);
    }
}
