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

import static com.johnsoft.viewer.FileParser.FileHead.FORMAT_ARGB8888;
import static com.johnsoft.viewer.FileParser.FileHead.FORMAT_RGB565;
import static com.johnsoft.viewer.FileParser.FileHead.FORMAT_RGBA8888;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteOrder;

/**
 * @author John Kenrinus Lee
 * @version 2017-03-31
 */
public final class ARGBFileParser extends FileParser {
    @Override
    public void fillToBuffer(FileParser.FileHead head, int frameIndex, ColorSetter setter) {
        final int width = head.width;
        final int height = head.height;
        final int format = head.format;
        final boolean endianMatches = head.endianMatches;

        DataInputStream input = null;
        try {
            input = new DataInputStream(new BufferedInputStream(new FileInputStream(head.file)));
            int skipLength = 0;
            if (format == FORMAT_RGB565) {
                skipLength = frameIndex * (width * height) * 2/* unsigned short */;
            } else if (format == FORMAT_RGBA8888 || format == FORMAT_ARGB8888) {
                skipLength = frameIndex * (width * height) * 4/* unsigned int */;
            } else {
                throw new IllegalStateException("Logical error!");
            }
            final byte[] bytes = new byte[8192];
            while (skipLength > 0) {
                skipLength -= input.read(bytes, 0, Math.min(skipLength, 8192));
            }

            int raw;
            for (int i = 0; i < height; ++i) {
                for (int j = 0; j < width; ++j) {
                    final int color;
                    switch (format) {
                        case FORMAT_RGB565: {
                            raw = input.readUnsignedShort();
                            if (!endianMatches) {
                                raw = convertByteOrderUnsignedShort(raw);
                            }
                            color = rgb565ToArgb8888(raw);
                        }
                            break;
                        case FORMAT_RGBA8888: {
                            raw = input.readInt();
                            if (!endianMatches) {
                                raw = convertByteOrderUnsignedInteger(raw);
                            }
                            color = rgba8888ToArgb8888(raw);
                        }
                            break;
                        case FORMAT_ARGB8888: {
                            raw = input.readInt();
                            if (!endianMatches) {
                                raw = convertByteOrderUnsignedInteger(raw);
                            }
                            color = raw;
                        }
                            break;
                        default:
                            throw new IllegalStateException("Logical error!");
                    }
                    final int[] argb = convertColorToArgb(color);
                    setter.set(j, i, width, height, argb[0], argb[1], argb[2], argb[3]);
                }
            }
        } catch (Exception e) {
            throw new IllegalStateException(e);
        } finally {
            if (input != null) {
                try {
                    input.close();
                } catch (IOException ignored) {
                    // do noting
                }
            }
        }
    }

    public static /*unsigned*/int convertByteOrderUnsignedShort(int unsignedShort) {
        unsignedShort = unsignedShort & 0x0000FFFF;
        int a = unsignedShort & 0x0000FF00;
        int b = unsignedShort & 0x000000FF;
        return (a >>> 8) | (b << 8);
    }

    public static /*unsigned*/int convertByteOrderUnsignedInteger(int unsignedInt) {
        int a = unsignedInt & 0xFF000000;
        int b = unsignedInt & 0x00FF0000;
        int c = unsignedInt & 0x0000FF00;
        int d = unsignedInt & 0x000000FF;
        return (a >>> 24) | (b >>> 8) | (c << 8) | (d << 24);
    }

    public static /*unsigned*/int rgb565ToArgb8888(/*unsigned short*/int n565Color) {
        n565Color = n565Color & 0x0000FFFF;
        // 获取RGB单色，并填充低位
        int cAlpha = 0x000000FF;
        int cRed = ((n565Color & 0x0000f800) >>> 8) & 0x000000FF;
        int cGreen = ((n565Color & 0x000007e0) >>> 3) & 0x000000FF;
        int cBlue = ((n565Color & 0x0000001f) << 3) & 0x000000FF;
        // 连接
        return (cAlpha << 24) | (cRed << 16) | (cGreen << 8) | cBlue;
    }

    public static /*unsigned*/int rgb565ToArgb8888X(/*unsigned short*/int n565Color) {
        n565Color = n565Color & 0x0000FFFF;
        // 获取RGB单色，并填充低位
        int cAlpha = 0x000000FF;
        int cRed = ((n565Color & 0x0000f800) >>> 8) & 0x000000FF;
        cRed = ((cRed & 0x00000038) >>> 3) | cRed;
        int cGreen = ((n565Color & 0x000007e0) >>> 3) & 0x000000FF;
        cGreen = ((cGreen & 0x0000000c) >>> 2) | cGreen;
        int cBlue = ((n565Color & 0x0000001f) << 3) & 0x000000FF;
        cBlue = ((cBlue & 0x00000038) >>> 3) | cBlue;
        // 连接
        return (cAlpha << 24) | (cRed << 16) | (cGreen << 8) | cBlue;
    }

    public static /*unsigned*/int rgba8888ToArgb8888(/*unsigned*/int color) {
        int rgb = color >>> 8;
        int alpha = color & 0x000000FF;
        return rgb | (alpha << 24);
    }

    public static int convertArgbToColor(int a, int r, int g, int b) {
        return ((a & 0xFF) << 24) | ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | ((b & 0xFF) << 0);
    }

    public static int[] convertColorToArgb(int color) {
        final int a = (color >> 24) & 0xFF;
        final int r = (color >> 16) & 0xFF;
        final int g = (color >> 8) & 0xFF;
        final int b = (color >> 0) & 0xFF;
        return new int[] {a, r, g, b};
    }

    public static int convertArgbToGrey(int a, int r, int g, int b) {
        return convertArgbToGreyXXX(a, r, g, b);
    }

    public static int convertArgbToGreyX(int a, int r, int g, int b) {
        a = a & 0xFF;
        r = r & 0xFF;
        g = g & 0xFF;
        b = b & 0xFF;
        return (int) (0.299 * r + 0.587 * g + 0.114 * b);
    }

    public static int convertArgbToGreyXX(int a, int r, int g, int b) {
        a = a & 0xFF;
        r = r & 0xFF;
        g = g & 0xFF;
        b = b & 0xFF;
        return (r * 30 + g * 59 + b * 11) / 100;
    }

    public static int convertArgbToGreyXXX(int a, int r, int g, int b) {
        a = a & 0xFF;
        r = r & 0xFF;
        g = g & 0xFF;
        b = b & 0xFF;
        return (r * 76 + g * 151 + b * 28) >> 8;
    }

    public static void convertRgb888FileToArgb8888File(File file, int width, int height, boolean isLittleEndian) {
        DataInputStream channelIn = null;
        DataOutputStream channelOut = null;
        try {
            final int length = width * height * 3;
            final boolean isLittleEndianHost = ByteOrder.LITTLE_ENDIAN.equals(ByteOrder.nativeOrder());
            final byte[] buffer = new byte[length];
            channelIn = new DataInputStream(new BufferedInputStream(new FileInputStream(file)));
            channelOut = new DataOutputStream(new BufferedOutputStream(new FileOutputStream(file.getAbsolutePath()
                    + ".argb")));
            while (channelIn.read(buffer) == buffer.length) {
                for (int i = 0; i < length; i += 3) {
                    int color = convertArgbToColor(255, buffer[i], buffer[i + 1], buffer[i + 2]);
                    if (isLittleEndian != isLittleEndianHost) {
                        color = convertByteOrderUnsignedInteger(color);
                    }
                    channelOut.writeInt(color);
                }
            }
        } catch (Exception e) {
            throw new IllegalStateException(e);
        } finally {
            if (channelIn != null) {
                try {
                    channelIn.close();
                } catch (IOException ignored) {
                    // do nothing
                }
            }
            if (channelOut != null) {
                try {
                    channelOut.close();
                } catch (IOException ignored) {
                    // do nothing
                }
            }
        }
    }

    public static void main(String[] args) {
        convertRgb888FileToArgb8888File(new File("convert/susheview_le_640x480.rgb24"), 640, 480, true);
    }
}
