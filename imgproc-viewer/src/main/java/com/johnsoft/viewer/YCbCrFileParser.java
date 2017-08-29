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

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;

/**
 * ffmpeg -i aixiao.mp4 -an -f rawvideo -vframes 1 -vcodec rawvideo -ss "0:0:6" -s 1024x768 -pix_fmt nv12
 * output_le_1024x768.yuv420sp
 *
 * @author John Kenrinus Lee
 * @version 2017-04-06
 */
public final class YCbCrFileParser extends FileParser {
    @Override
    public void fillToBuffer(FileParser.FileHead head, int frameIndex, final ColorSetter setter) {
        fillYuvToBuffer(head, frameIndex, new SafeColorSetter() {
            @Override
            public void onColor(int x, int y, int w, int h, int alpha, int Y, int Cb, int Cr) {
                final int[] argb = aycbcr2argb(alpha, Y, Cb, Cr);
                setter.set(x, y, w, h, argb[0], argb[1], argb[2], argb[3]);
            }
        });
    }

    public void fillToBufferRaw(FileHead head, int frameIndex, ColorSetter setter) {
        fillYuvToBuffer(head, frameIndex, setter);
    }

    private static void fillYuvToBuffer(FileHead head, int frameIndex, ColorSetter setter) {
        final File file = head.file;
        final int width = head.width;
        final int height = head.height;
        final int format = head.format;
        if (format == FileHead.FORMAT_YUV420P || format == FileHead.FORMAT_YV12
                || format == FileHead.FORMAT_YUV420SP || format == FileHead.FORMAT_NV21) {
            fillYuv420ToBuffer(head, setter, file, width, height, format, frameIndex);
        } else if (format == FileHead.FORMAT_YUV422P || format == FileHead.FORMAT_YV16
                || format == FileHead.FORMAT_YUV422SP || format == FileHead.FORMAT_NV61) {
            fillYuv422ToBuffer(head, setter, file, width, height, format, frameIndex);
        } else if (format == FileHead.FORMAT_YUV444P || format == FileHead.FORMAT_YV24
                || format == FileHead.FORMAT_YUV444SP || format == FileHead.FORMAT_NV42) {
            fillYuv444ToBuffer(head, setter, file, width, height, format, frameIndex);
        } else if (format == FileHead.FORMAT_YUV444 || format == FileHead.FORMAT_YVU444) {
            fillYuv444PackedToBuffer(head, setter, file, width, height, format, frameIndex);
        } else if (format == FileHead.FORMAT_YUYV422 || format == FileHead.FORMAT_YVYU422
                || format == FileHead.FORMAT_UYVY422 || format == FileHead.FORMAT_VYUY422) {
            fillYuv422PackedToBuffer(head, setter, file, width, height, format, frameIndex);
        }
    }

    /**
     *  odd resolution, like 3x3 image:
     *
     *  Y1 Y2 Y3
     *  Y4 Y5 Y6
     *  Y7 Y8 Y9
     *  U1 U2
     *  U3 U4
     *  V1 V2
     *  V3 V4
     *
     *  | Y1 U1 V1 | Y2 U1 V1 | Y3 U2 V2 | Y4 U1 V1 | Y5 U1 V1 | Y6 U2 V2 |
     *  | Y7 U3 V3 | Y8 U3 V3 | Y9 U4 V4 |
     */
    private static void fillYuv420ToBuffer(FileHead head, ColorSetter setter, File file, int width, int height,
                                           int format, int frameIndex) {
        final int uvRowBytes = (width + 1) / 2;
        final int halfHeight = (height + 1) / 2;
        final int uvSize = uvRowBytes * halfHeight;
        final int numOfPixels = width * height;
        final int length = numOfPixels + uvSize * 2;
        final byte[] buffer = new byte[length];
        readFileToBuffer(file, frameIndex * length, buffer);

        final int posOfU;
        final int posOfV;
        if (format == FileHead.FORMAT_YUV420P || format == FileHead.FORMAT_YUV420SP) {
            posOfU = numOfPixels;
            posOfV = uvSize + numOfPixels;
        } else if (format == FileHead.FORMAT_YV12 || format == FileHead.FORMAT_NV21) {
            posOfU = uvSize + numOfPixels;
            posOfV = numOfPixels;
        } else {
            throw new IllegalStateException("Logical error!");
        }
        final int base;
        if (format == FileHead.FORMAT_YUV420P || format == FileHead.FORMAT_YV12) {
            base = uvRowBytes;
        } else if (format == FileHead.FORMAT_YUV420SP || format == FileHead.FORMAT_NV21) {
            base = uvRowBytes * 2;
        } else {
            throw new IllegalStateException("Logical error!");
        }

        for (int i = 0; i < height; ++i) {
            final int step = (i / 2) * base;
            final int startY = i * width;
            final int startU = posOfU + step;
            final int startV = posOfV + step;
            for (int j = 0; j < width; ++j) {
                final int indexY = startY + j;
                final int indexU, indexV;
                if (format == FileHead.FORMAT_YUV420SP) {
                    indexU = startU + j - j % 2;
                    indexV = indexU + 1;
                } else if (format == FileHead.FORMAT_NV21) {
                    indexV = startV + j - j % 2;
                    indexU = indexV + 1;
                } else {
                    indexU = startU + j / 2;
                    indexV = startV + j / 2;
                }
                setter.set(j, i, width, height, 255, buffer[indexY], buffer[indexU], buffer[indexV]);
            }
        }
    }

    private static void fillYuv422ToBuffer(FileHead head, ColorSetter setter, File file, int width, int height,
                                           int format, int frameIndex) {
        final int uvRowBytes = (width + 1) / 2;
        final int uvSize = uvRowBytes * height;
        final int numOfPixels = width * height;
        final int length = numOfPixels + uvSize * 2;
        final byte[] buffer = new byte[length];
        readFileToBuffer(file, frameIndex * length, buffer);

        final int posOfU;
        final int posOfV;
        if (format == FileHead.FORMAT_YUV422P || format == FileHead.FORMAT_YUV422SP) {
            posOfU = numOfPixels;
            posOfV = uvSize + numOfPixels;
        } else if (format == FileHead.FORMAT_YV16 || format == FileHead.FORMAT_NV61) {
            posOfU = uvSize + numOfPixels;
            posOfV = numOfPixels;
        } else {
            throw new IllegalStateException("Logical error!");
        }
        final int base;
        if (format == FileHead.FORMAT_YUV422P || format == FileHead.FORMAT_YV16) {
            base = uvRowBytes;
        } else if (format == FileHead.FORMAT_YUV422SP || format == FileHead.FORMAT_NV61) {
            base = uvRowBytes * 2;
        } else {
            throw new IllegalStateException("Logical error!");
        }

        for (int i = 0; i < height; ++i) {
            final int step = i * base;
            final int startY = i * width;
            final int startU = posOfU + step;
            final int startV = posOfV + step;
            for (int j = 0; j < width; ++j) {
                final int indexY = startY + j;
                final int indexU, indexV;
                if (format == FileHead.FORMAT_YUV422SP) {
                    indexU = startU + j - j % 2;
                    indexV = indexU + 1;
                } else if (format == FileHead.FORMAT_NV61) {
                    indexV = startV + j - j % 2;
                    indexU = indexV + 1;
                } else {
                    indexU = startU + j / 2;
                    indexV = startV + j / 2;
                }
                setter.set(j, i, width, height, 255, buffer[indexY], buffer[indexU], buffer[indexV]);
            }
        }
    }

    private static void fillYuv444ToBuffer(FileHead head, ColorSetter setter, File file, int width, int height,
                                           int format, int frameIndex) {
        final int numOfPixels = width * height;
        final int length = numOfPixels * 3;
        final byte[] buffer = new byte[length];
        readFileToBuffer(file, frameIndex * length, buffer);

        final int posOfU;
        final int posOfV;
        if (format == FileHead.FORMAT_YUV444P || format == FileHead.FORMAT_YUV444SP) {
            posOfU = numOfPixels;
            posOfV = posOfU + numOfPixels;
        } else if (format == FileHead.FORMAT_YV24 || format == FileHead.FORMAT_NV42) {
            posOfV = numOfPixels;
            posOfU = posOfV + numOfPixels;
        } else {
            throw new IllegalStateException("Logical error!");
        }
        final int base;
        if (format == FileHead.FORMAT_YUV444P || format == FileHead.FORMAT_YV24) {
            base = width;
        } else if (format == FileHead.FORMAT_YUV444SP || format == FileHead.FORMAT_NV42) {
            base = width * 2;
        } else {
            throw new IllegalStateException("Logical error!");
        }

        for (int i = 0; i < height; ++i) {
            final int step = i * base;
            final int startY = i * width;
            final int startU = posOfU + step;
            final int startV = posOfV + step;
            for (int j = 0; j < width; ++j) {
                final int indexY = startY + j;
                final int indexU, indexV;
                if (format == FileHead.FORMAT_YUV444SP) {
                    indexU = startU + j * 2;
                    indexV = indexU + 1;
                } else if (format == FileHead.FORMAT_NV42) {
                    indexV = startV + j * 2;
                    indexU = indexV + 1;
                } else {
                    indexU = startU + j;
                    indexV = startV + j;
                }
                setter.set(j, i, width, height, 255, buffer[indexY], buffer[indexU], buffer[indexV]);
            }
        }
    }

    private static void fillYuv444PackedToBuffer(FileHead head, ColorSetter setter, File file, int width, int height,
                                                 int format, int frameIndex) {
        final int numOfPixels = width * height;
        final int length = numOfPixels * 3;
        final byte[] buffer = new byte[length];
        readFileToBuffer(file, frameIndex * length, buffer);

        final int stepU;
        final int stepV;
        if (format == FileHead.FORMAT_YUV444) {
            stepU = 1;
            stepV = 2;
        } else if (format == FileHead.FORMAT_YVU444) {
            stepV = 1;
            stepU = 2;
        } else {
            throw new IllegalStateException("Logical error!");
        }

        final int line = width * 3;
        for (int i = 0; i < height; ++i) {
            final int startY = i * line;
            for (int j = 0; j < width; ++j) {
                final int indexY = startY + j * 3;
                final int indexU = indexY + stepU;
                final int indexV = indexY + stepV;
                setter.set(j, i, width, height, 255, buffer[indexY], buffer[indexU], buffer[indexV]);
            }
        }
    }

    /**
     *  6x3 image:
     *
     *  Y0  U0  Y1  V0  Y2  U1  Y3  V1  Y4  U2  Y5  V2
     *  Y6  U3  Y7  V3  Y8  U4  Y9  V4  Y10 U5  Y11 V5
     *  Y12 U6  Y13 V6  Y14 U7  Y15 V7  Y16 U8  Y17 V8
     *
     *  5x3 image:
     *  U0  Y0  V0  Y1  U1  Y2  V1  Y3  U2  Y4  V2  Y4
     *  U3  Y5  V3  Y6  U4  Y7  V4  Y8  U5  Y9  V5  Y9
     *  U6  Y10 V6  Y11 U7  Y12 V7  Y13 U8  Y14 V8  Y14
     *
     *  |Y0 U0 V0 | Y1 U0 V0 | Y2 U1 V1 | Y3 U1 V1 | Y4 U2 V2 | Y5 U2 V2 |
     */
    private static void fillYuv422PackedToBuffer(FileHead head, ColorSetter setter, File file, int width, int height,
                                                 int format, int frameIndex) {
        final int length = ((width + width % 2) * height) * 2;
        final byte[] buffer = new byte[length];
        readFileToBuffer(file, frameIndex * length, buffer);

        final int yBase;
        final int stepOddU, stepEvenU;
        final int stepOddV, stepEvenV;
        if (format == FileHead.FORMAT_YUYV422) {
            yBase = 0;
            stepEvenU = 1;
            stepOddU = -1;
            stepEvenV = 3;
            stepOddV = 1;
        } else if (format == FileHead.FORMAT_YVYU422) {
            yBase = 0;
            stepEvenU = 3;
            stepOddU = 1;
            stepEvenV = 1;
            stepOddV = -1;
        } else if (format == FileHead.FORMAT_UYVY422) {
            yBase = 1;
            stepEvenU = -1;
            stepOddU = -3;
            stepEvenV = 1;
            stepOddV = -1;
        } else if (format == FileHead.FORMAT_VYUY422) {
            yBase = 1;
            stepEvenU = 1;
            stepOddU = -1;
            stepEvenV = -1;
            stepOddV = -3;
        } else {
            throw new IllegalStateException("Logical error!");
        }

        final int line = (width + width % 2) * 2;
        for (int i = 0; i < height; ++i) {
            final int startY = i * line + yBase;
            for (int j = 0; j < width; ++j) {
                final int indexY = startY + j * 2;
                final int indexU, indexV;
                if (j % 2 == 0) {
                    indexU = indexY + stepEvenU;
                    indexV = indexY + stepEvenV;
                } else {
                    indexU = indexY + stepOddU;
                    indexV = indexY + stepOddV;
                }
                setter.set(j, i, width, height, 255, buffer[indexY], buffer[indexU], buffer[indexV]);
            }
        }
    }

    private static void readFileToBuffer(File file, long skipOffset, byte[] buffer) {
        FileChannel channel = null;
        try {
            channel = new FileInputStream(file).getChannel();
            channel.position(skipOffset);
            if (channel.read(ByteBuffer.wrap(buffer)) != buffer.length) {
                throw new IllegalArgumentException("Read size not equals calculate size!");
            }
        } catch (Exception e) {
            throw new IllegalStateException(e);
        } finally {
            if (channel != null) {
                try {
                    channel.close();
                } catch (IOException ignored) {
                    // do nothing
                }
            }
        }
    }

    public static final int[] aycbcr2argb(/*uint_8*/int alpha, /*uint_8*/int Y, /*uint_8*/int Cb, /*uint_8*/int Cr) {
        return aycbcr2argbXXX(alpha, Y, Cb, Cr);
    }

    private static int[] clampArgb(int a, int r, int g, int b) {
        a = a > 255 ? 255 : a < 0 ? 0 : a;
        r = r > 255 ? 255 : r < 0 ? 0 : r;
        g = g > 255 ? 255 : g < 0 ? 0 : g;
        b = b > 255 ? 255 : b < 0 ? 0 : b;
        return new int[] { a, r, g, b };
    }

    public static int[] aycbcr2argbX(/*uint_8*/int alpha, /*uint_8*/int Y, /*uint_8*/int Cb, /*uint_8*/int Cr) {
        Y = 0xFF & Y;
        Cb = (0xFF & Cb) - 128;
        Cr = (0xFF & Cr) - 128;
        // origin:
        int R = (int) (Y + 1.4 * Cr);
        int G = (int) (Y - 0.34 * Cb - 0.71 * Cr);
        int B = (int) (Y + 1.77 * Cb);
        return clampArgb(alpha, R, G, B);
    }

    public static int[] aycbcr2argbXX(/*uint_8*/int alpha, /*uint_8*/int Y, /*uint_8*/int Cb, /*uint_8*/int Cr) {
        Y = 0xFF & Y;
        Cb = (0xFF & Cb) - 128;
        Cr = (0xFF & Cr) - 128;
        // optimization:
        int RDiff = Cr + ((Cr * 103) >> 8);
        int negativeGDiff = ((Cb * 88) >> 8) + ((Cr * 183) >> 8);
        int BDiff = Cb + ((Cb * 198) >> 8);
        int R = Y + RDiff;
        int G = Y - negativeGDiff;
        int B = Y + BDiff;
        return clampArgb(alpha, R, G, B);
    }

    public static int[] aycbcr2argbXXX(/*uint_8*/int alpha, /*uint_8*/int Y, /*uint_8*/int Cb, /*uint_8*/int Cr) {
        Y = 0xFF & Y;
        Cb = 0xFF & Cb;
        Cr = 0xFF & Cr;
        // optimization:
        int R = Y + RDiffTable[Cr];
        int G = Y - GCbDiffTable[Cb] - GCrDiffTable[Cr];
        int B = Y + BDiffTable[Cb];
        return clampArgb(alpha, R, G, B);
    }

    private static final int[] RDiffTable = new int[] {
            -180, -179, -177, -176, -174, -173, -172, -170, -169, -167, -166, -165, -163, -162, -160, -159, -158,
            -156, -155, -153, -152, -151, -149, -148, -146, -145, -144, -142, -141, -139, -138, -137, -135,
            -134, -132, -131, -130, -128, -127, -125, -124, -123, -121, -120, -118, -117, -115, -114, -113,
            -111, -110, -108, -107, -106, -104, -103, -101, -100, -99, -97, -96, -94, -93, -92, -90,
            -89, -87, -86, -85, -83, -82, -80, -79, -78, -76, -75, -73, -72, -71, -69, -68,
            -66, -65, -64, -62, -61, -59, -58, -57, -55, -54, -52, -51, -50, -48, -47, -45,
            -44, -43, -41, -40, -38, -37, -36, -34, -33, -31, -30, -29, -27, -26, -24, -23,
            -22, -20, -19, -17, -16, -15, -13, -12, -10, -9, -8, -6, -5, -3, -2, 0,
            1, 2, 4, 5, 7, 8, 9, 11, 12, 14, 15, 16, 18, 19, 21, 22,
            23, 25, 26, 28, 29, 30, 32, 33, 35, 36, 37, 39, 40, 42, 43, 44,
            46, 47, 49, 50, 51, 53, 54, 56, 57, 58, 60, 61, 63, 64, 65, 67,
            68, 70, 71, 72, 74, 75, 77, 78, 79, 81, 82, 84, 85, 86, 88, 89,
            91, 92, 93, 95, 96, 98, 99, 100, 102, 103, 105, 106, 107, 109, 110, 112,
            113, 114, 116, 117, 119, 120, 122, 123, 124, 126, 127, 129, 130, 131, 133, 134,
            136, 137, 138, 140, 141, 143, 144, 145, 147, 148, 150, 151, 152, 154, 155, 157,
            158, 159, 161, 162, 164, 165, 166, 168, 169, 171, 172, 173, 175, 176, 178,
    };
    private static final int[] GCbDiffTable = new int[] {
            -44, -44, -44, -43, -43, -43, -42, -42, -42, -41, -41, -41, -40, -40, -40, -39, -39,
            -39, -38, -38, -38, -37, -37, -37, -36, -36, -36, -35, -35, -35, -34, -34, -33,
            -33, -33, -32, -32, -32, -31, -31, -31, -30, -30, -30, -29, -29, -29, -28, -28,
            -28, -27, -27, -27, -26, -26, -26, -25, -25, -25, -24, -24, -24, -23, -23, -22,
            -22, -22, -21, -21, -21, -20, -20, -20, -19, -19, -19, -18, -18, -18, -17, -17,
            -17, -16, -16, -16, -15, -15, -15, -14, -14, -14, -13, -13, -13, -12, -12, -11,
            -11, -11, -10, -10, -10, -9, -9, -9, -8, -8, -8, -7, -7, -7, -6, -6,
            -6, -5, -5, -5, -4, -4, -4, -3, -3, -3, -2, -2, -2, -1, -1, 0,
            0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5,
            5, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 11,
            11, 11, 12, 12, 12, 13, 13, 13, 14, 14, 14, 15, 15, 15, 16, 16,
            16, 17, 17, 17, 18, 18, 18, 19, 19, 19, 20, 20, 20, 21, 21, 22,
            22, 22, 23, 23, 23, 24, 24, 24, 25, 25, 25, 26, 26, 26, 27, 27,
            27, 28, 28, 28, 29, 29, 29, 30, 30, 30, 31, 31, 31, 32, 32, 33,
            33, 33, 34, 34, 34, 35, 35, 35, 36, 36, 36, 37, 37, 37, 38, 38,
            38, 39, 39, 39, 40, 40, 40, 41, 41, 41, 42, 42, 42, 43, 43,
    };
    private static final int[] GCrDiffTable = new int[] {
            -92, -91, -91, -90, -89, -88, -88, -87, -86, -86, -85, -84, -83, -83, -82, -81, -81,
            -80, -79, -78, -78, -77, -76, -76, -75, -74, -73, -73, -72, -71, -71, -70, -69,
            -68, -68, -67, -66, -66, -65, -64, -63, -63, -62, -61, -61, -60, -59, -58, -58,
            -57, -56, -56, -55, -54, -53, -53, -52, -51, -51, -50, -49, -48, -48, -47, -46,
            -46, -45, -44, -43, -43, -42, -41, -41, -40, -39, -38, -38, -37, -36, -36, -35,
            -34, -33, -33, -32, -31, -31, -30, -29, -28, -28, -27, -26, -26, -25, -24, -23,
            -23, -22, -21, -21, -20, -19, -18, -18, -17, -16, -16, -15, -14, -13, -13, -12,
            -11, -11, -10, -9, -8, -8, -7, -6, -6, -5, -4, -3, -3, -2, -1, 0,
            0, 1, 2, 2, 3, 4, 5, 5, 6, 7, 7, 8, 9, 10, 10, 11,
            12, 12, 13, 14, 15, 15, 16, 17, 17, 18, 19, 20, 20, 21, 22, 22,
            23, 24, 25, 25, 26, 27, 27, 28, 29, 30, 30, 31, 32, 32, 33, 34,
            35, 35, 36, 37, 37, 38, 39, 40, 40, 41, 42, 42, 43, 44, 45, 45,
            46, 47, 47, 48, 49, 50, 50, 51, 52, 52, 53, 54, 55, 55, 56, 57,
            57, 58, 59, 60, 60, 61, 62, 62, 63, 64, 65, 65, 66, 67, 67, 68,
            69, 70, 70, 71, 72, 72, 73, 74, 75, 75, 76, 77, 77, 78, 79, 80,
            80, 81, 82, 82, 83, 84, 85, 85, 86, 87, 87, 88, 89, 90, 90,
    };
    private static final int[] BDiffTable = new int[] {
            -227, -226, -224, -222, -220, -219, -217, -215, -213, -212, -210, -208, -206, -204, -203, -201, -199,
            -197, -196, -194, -192, -190, -188, -187, -185, -183, -181, -180, -178, -176, -174, -173, -171,
            -169, -167, -165, -164, -162, -160, -158, -157, -155, -153, -151, -149, -148, -146, -144, -142,
            -141, -139, -137, -135, -134, -132, -130, -128, -126, -125, -123, -121, -119, -118, -116, -114,
            -112, -110, -109, -107, -105, -103, -102, -100, -98, -96, -94, -93, -91, -89, -87, -86,
            -84, -82, -80, -79, -77, -75, -73, -71, -70, -68, -66, -64, -63, -61, -59, -57,
            -55, -54, -52, -50, -48, -47, -45, -43, -41, -40, -38, -36, -34, -32, -31, -29,
            -27, -25, -24, -22, -20, -18, -16, -15, -13, -11, -9, -8, -6, -4, -2, 0,
            1, 3, 5, 7, 8, 10, 12, 14, 15, 17, 19, 21, 23, 24, 26, 28,
            30, 31, 33, 35, 37, 39, 40, 42, 44, 46, 47, 49, 51, 53, 54, 56,
            58, 60, 62, 63, 65, 67, 69, 70, 72, 74, 76, 78, 79, 81, 83, 85,
            86, 88, 90, 92, 93, 95, 97, 99, 101, 102, 104, 106, 108, 109, 111, 113,
            115, 117, 118, 120, 122, 124, 125, 127, 129, 131, 133, 134, 136, 138, 140, 141,
            143, 145, 147, 148, 150, 152, 154, 156, 157, 159, 161, 163, 164, 166, 168, 170,
            172, 173, 175, 177, 179, 180, 182, 184, 186, 187, 189, 191, 193, 195, 196, 198,
            200, 202, 203, 205, 207, 209, 211, 212, 214, 216, 218, 219, 221, 223, 225,
    };

    public static void main(String[] args) {
        generateTables();
    }

    private static void generateTables() {
        final int min = -128, max = 128; // final int min = 0, max = 256;
        // RDiff
        final StringBuilder RDiff = new StringBuilder("private static final int[] RDiffTable = new int[] {\n");
        for (int Cr = min; Cr < max; ++Cr) {
            RDiff.append(Cr + ((Cr * 103) >> 8)).append(',').append((Cr != min && Cr % 16 == 0) ? '\n' : ' ');
        }
        System.out.println(RDiff.append("\n};").toString());

        // negativeGDiff
        final StringBuilder GCbDiff = new StringBuilder("private static final int[] GCbDiffTable = new int[] {\n");
        for (int Cb = min; Cb < max; ++Cb) {
            GCbDiff.append((Cb * 88) >> 8).append(',').append((Cb != min && Cb % 16 == 0) ? '\n' : ' ');
        }
        System.out.println(GCbDiff.append("\n};").toString());
        final StringBuilder GCrDiff = new StringBuilder("private static final int[] GCrDiffTable = new int[] {\n");
        for (int Cr = min; Cr < max; ++Cr) {
            GCrDiff.append((Cr * 183) >> 8).append(',').append((Cr != min && Cr % 16 == 0) ? '\n' : ' ');
        }
        System.out.println(GCrDiff.append("\n};").toString());

        // BDiff
        final StringBuilder BDiff = new StringBuilder("private static final int[] BDiffTable = new int[] {\n");
        for (int Cb = min; Cb < max; ++Cb) {
            BDiff.append(Cb + ((Cb * 198) >> 8)).append(',').append((Cb != min && Cb % 16 == 0) ? '\n' : ' ');
        }
        System.out.println(BDiff.append("\n};").toString());
    }
}
