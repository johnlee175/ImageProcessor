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
package com.johnsoft.viewer;

import static com.johnsoft.viewer.FileParser.FileHead.FORMAT_A;
import static com.johnsoft.viewer.FileParser.FileHead.FORMAT_ARGB8888;
import static com.johnsoft.viewer.FileParser.FileHead.FORMAT_B;
import static com.johnsoft.viewer.FileParser.FileHead.FORMAT_G;
import static com.johnsoft.viewer.FileParser.FileHead.FORMAT_YUV;
import static com.johnsoft.viewer.FileParser.FileHead.FORMAT_NV21;
import static com.johnsoft.viewer.FileParser.FileHead.FORMAT_NV42;
import static com.johnsoft.viewer.FileParser.FileHead.FORMAT_NV61;
import static com.johnsoft.viewer.FileParser.FileHead.FORMAT_R;
import static com.johnsoft.viewer.FileParser.FileHead.FORMAT_RGB565;
import static com.johnsoft.viewer.FileParser.FileHead.FORMAT_RGBA8888;
import static com.johnsoft.viewer.FileParser.FileHead.FORMAT_UYVY422;
import static com.johnsoft.viewer.FileParser.FileHead.FORMAT_VYUY422;
import static com.johnsoft.viewer.FileParser.FileHead.FORMAT_YUV420P;
import static com.johnsoft.viewer.FileParser.FileHead.FORMAT_YUV420SP;
import static com.johnsoft.viewer.FileParser.FileHead.FORMAT_YUV422P;
import static com.johnsoft.viewer.FileParser.FileHead.FORMAT_YUV422SP;
import static com.johnsoft.viewer.FileParser.FileHead.FORMAT_YUV444;
import static com.johnsoft.viewer.FileParser.FileHead.FORMAT_YUV444P;
import static com.johnsoft.viewer.FileParser.FileHead.FORMAT_YUV444SP;
import static com.johnsoft.viewer.FileParser.FileHead.FORMAT_YUYV422;
import static com.johnsoft.viewer.FileParser.FileHead.FORMAT_YV12;
import static com.johnsoft.viewer.FileParser.FileHead.FORMAT_YV16;
import static com.johnsoft.viewer.FileParser.FileHead.FORMAT_YV24;
import static com.johnsoft.viewer.FileParser.FileHead.FORMAT_YVU444;
import static com.johnsoft.viewer.FileParser.FileHead.FORMAT_YVYU422;

import java.io.File;
import java.nio.ByteOrder;

/**
 * @author John Kenrinus Lee
 * @version 2017-04-06
 */
public abstract class FileParser {
    /**
     * @see SafeColorSetter
     */
    public interface ColorSetter {
        void set(int x, int y, int w, int h, int alpha, int rOrY, int gOrCb, int bOrCr);
    }

    public static abstract class SafeColorSetter implements ColorSetter {
        @Override
        public final void set(int x, int y, int w, int h, int alpha, int rOrY, int gOrCb, int bOrCr) {
            onColor(x, y, w, h, (alpha & 0xFF), (rOrY & 0xFF), (gOrCb & 0xFF), (bOrCr & 0xFF));
        }

        public abstract void onColor(int x, int y, int w, int h, int alpha, int rOrY, int gOrCb, int bOrCr);
    }

    public static class FileHead {
        public static final int FORMAT_ARGB8888 = 1;
        public static final int FORMAT_RGBA8888 = 2;
        public static final int FORMAT_RGB565 = 4;

        public static final int FORMAT_YUV420P = 1001;
        public static final int FORMAT_YUV422P = 1002;
        public static final int FORMAT_YUV444P = 1003;
        public static final int FORMAT_YV12 = 1004;
        public static final int FORMAT_YV16 = 1005;
        public static final int FORMAT_YV24 = 1006;
        public static final int FORMAT_YUV420SP = 1007;
        public static final int FORMAT_YUV422SP = 1008;
        public static final int FORMAT_YUV444SP = 1009;
        public static final int FORMAT_NV21 = 1010;
        public static final int FORMAT_NV61 = 1011;
        public static final int FORMAT_NV42 = 1012;
        public static final int FORMAT_YUV444 = 1013;
        public static final int FORMAT_YVU444 = 1014;
        public static final int FORMAT_YUYV422 = 1015;
        public static final int FORMAT_UYVY422 = 1016;
        public static final int FORMAT_YVYU422 = 1017;
        public static final int FORMAT_VYUY422 = 1018;

        public static final int FORMAT_YUV = 2001;
        public static final int FORMAT_R = 2002;
        public static final int FORMAT_G = 2003;
        public static final int FORMAT_B = 2004;
        public static final int FORMAT_A = 2005;

        public File file;
        public int width;
        public int height;
        public int format;
        public boolean endianMatches;
    }

    public static FileParser parseFormat(File file) {
        if (file == null || !file.exists()) {
            throw new IllegalArgumentException("input file is null or not exists!");
        }
        FileParser fileParser;
        FileHead head = new FileHead();
        head.file = file;
        String name = file.getName();
        int i = name.lastIndexOf('.');
        String formatStr = name.substring(i + 1);
        if (formatStr.equals("argb")) {
            fileParser = new ARGBFileParser();
            head.format = FORMAT_ARGB8888;
        } else if (formatStr.equals("rgba")) {
            fileParser = new ARGBFileParser();
            head.format = FORMAT_RGBA8888;
        } else if (formatStr.equals("rgb")) {
            fileParser = new ARGBFileParser();
            head.format = FORMAT_RGB565;
        } else if (formatStr.equals("yuv420p") || formatStr.equals("i420")) {
            fileParser = new YCbCrFileParser();
            head.format = FORMAT_YUV420P;
        } else if (formatStr.equals("yuv422p") || formatStr.equals("i422")) {
            fileParser = new YCbCrFileParser();
            head.format = FORMAT_YUV422P;
        } else if (formatStr.equals("yuv444p") || formatStr.equals("i444")) {
            fileParser = new YCbCrFileParser();
            head.format = FORMAT_YUV444P;
        } else if (formatStr.equals("yuv420sp") || formatStr.equals("nv12")) {
            fileParser = new YCbCrFileParser();
            head.format = FORMAT_YUV420SP;
        } else if (formatStr.equals("yuv422sp") || formatStr.equals("nv16")) {
            fileParser = new YCbCrFileParser();
            head.format = FORMAT_YUV422SP;
        } else if (formatStr.equals("yuv444sp") || formatStr.equals("nv24")) {
            fileParser = new YCbCrFileParser();
            head.format = FORMAT_YUV444SP;
        } else if (formatStr.equals("yuv444")) {
            fileParser = new YCbCrFileParser();
            head.format = FORMAT_YUV444;
        } else if (formatStr.equals("yvu444")) {
            fileParser = new YCbCrFileParser();
            head.format = FORMAT_YVU444;
        } else if (formatStr.equals("yuyv422") || formatStr.equals("yuy2")) {
            fileParser = new YCbCrFileParser();
            head.format = FORMAT_YUYV422;
        } else if (formatStr.equals("uyvy422")) {
            fileParser = new YCbCrFileParser();
            head.format = FORMAT_UYVY422;
        } else if (formatStr.equals("yvyu422")) {
            fileParser = new YCbCrFileParser();
            head.format = FORMAT_YVYU422;
        } else if (formatStr.equals("vyuy422")) {
            fileParser = new YCbCrFileParser();
            head.format = FORMAT_VYUY422;
        } else if (formatStr.equals("yv12")) {
            fileParser = new YCbCrFileParser();
            head.format = FORMAT_YV12;
        } else if (formatStr.equals("yv16")) {
            fileParser = new YCbCrFileParser();
            head.format = FORMAT_YV16;
        } else if (formatStr.equals("yv24")) {
            fileParser = new YCbCrFileParser();
            head.format = FORMAT_YV24;
        } else if (formatStr.equals("nv21")) {
            fileParser = new YCbCrFileParser();
            head.format = FORMAT_NV21;
        } else if (formatStr.equals("nv61")) {
            fileParser = new YCbCrFileParser();
            head.format = FORMAT_NV61;
        } else if (formatStr.equals("nv42")) {
            fileParser = new YCbCrFileParser();
            head.format = FORMAT_NV42;
        } else if (formatStr.equals("yuv")) {
            fileParser = new YCbCrFileParser();
            head.format = FORMAT_YUV420P;
        } else if (formatStr.equals("y") || formatStr.equals("u") || formatStr.equals("v")) {
            fileParser = new ChannelFileParser();
            head.format = FORMAT_YUV;
        } else if (formatStr.equals("r")) {
            fileParser = new ChannelFileParser();
            head.format = FORMAT_R;
        } else if (formatStr.equals("g")) {
            fileParser = new ChannelFileParser();
            head.format = FORMAT_G;
        } else if (formatStr.equals("b")) {
            fileParser = new ChannelFileParser();
            head.format = FORMAT_B;
        } else if (formatStr.equals("a") || formatStr.equals("grey")) {
            fileParser = new ChannelFileParser();
            head.format = FORMAT_A;
        } else {
            throw new IllegalArgumentException("Unknown format!");
        }
        int j = name.lastIndexOf('x', i - 1);
        head.height = Integer.parseInt(name.substring(j + 1, i));
        int x = name.lastIndexOf('_', j - 1);
        head.width = Integer.parseInt(name.substring(x + 1, j));
        boolean isLittleEndianHost = ByteOrder.LITTLE_ENDIAN.equals(ByteOrder.nativeOrder());
        int y = name.lastIndexOf('_', x - 1);
        String endian = name.substring(y + 1, x);
        boolean isLittleEndianTarget = "le".equals(endian);
        if (!isLittleEndianTarget && !"be".equals(endian)) {
            throw new IllegalArgumentException("Unknown endian");
        }
        head.endianMatches = isLittleEndianHost == isLittleEndianTarget;
        fileParser.head = head;
        return fileParser;
    }

    public static int getFrameCount(FileHead head) {
        final long length = head.file.length();
        final int format = head.format;
        final int width = head.width;
        final int height = head.height;
        final int frameSize;
        switch (format) {
            case FORMAT_ARGB8888:
            case FORMAT_RGBA8888:
                frameSize = ((width * height) * 4/* unsigned int */);
                break;
            case FORMAT_RGB565:
                frameSize = (width * height) * 2/* unsigned short */;
                break;
            case FORMAT_YUV420P:
            case FORMAT_YV12:
            case FORMAT_YUV420SP:
            case FORMAT_NV21:
                frameSize = width * height + (((width + 1) / 2) * ((height + 1) / 2)) * 2;
                break;
            case FORMAT_YUV422P:
            case FORMAT_YV16:
            case FORMAT_YUV422SP:
            case FORMAT_NV61:
                frameSize = width * height + (((width + 1) / 2) * height) * 2;
                break;
            case FORMAT_YUV444P:
            case FORMAT_YV24:
            case FORMAT_YUV444SP:
            case FORMAT_NV42:
            case FORMAT_YUV444:
            case FORMAT_YVU444:
                frameSize = width * height * 3;
                break;
            case FORMAT_YUYV422:
            case FORMAT_UYVY422:
            case FORMAT_YVYU422:
            case FORMAT_VYUY422:
                frameSize = ((width + width % 2) * height) * 2;
                break;
            case FORMAT_YUV:
            case FORMAT_R:
            case FORMAT_G:
            case FORMAT_B:
            case FORMAT_A:
                frameSize = width * height;
                break;
            default:
                return -1;
        }
        if (length % frameSize != 0) {
            return -1;
        }
        return (int) (length / frameSize);
    }

    private FileHead head;

    public final FileHead getHead() {
        return head;
    }

    public abstract void fillToBuffer(FileHead head, int frameIndex, ColorSetter setter);
}
