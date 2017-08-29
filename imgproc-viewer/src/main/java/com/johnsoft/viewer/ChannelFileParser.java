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
import static com.johnsoft.viewer.FileParser.FileHead.FORMAT_B;
import static com.johnsoft.viewer.FileParser.FileHead.FORMAT_G;
import static com.johnsoft.viewer.FileParser.FileHead.FORMAT_YUV;
import static com.johnsoft.viewer.FileParser.FileHead.FORMAT_R;

import java.io.FileInputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;

/**
 * @author John Kenrinus Lee
 * @version 2017-04-11
 */
public final class ChannelFileParser extends FileParser {
    @Override
    public void fillToBuffer(FileHead head, int frameIndex, ColorSetter setter) {
        final int width = head.width;
        final int height = head.height;
        final int format = head.format;
        final int length = width * height;
        final byte[] buffer = new byte[length];
        FileChannel channel = null;
        try {
            channel = new FileInputStream(head.file).getChannel();
            channel.position(frameIndex * length);

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
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                final int index = i * width + j;
                final int value = buffer[index];
                switch (format) {
                    case FORMAT_YUV:
                        final int[] argb = YCbCrFileParser.aycbcr2argb(255, value, 128, 128);
                        setter.set(j, i, width, height, argb[0], argb[1], argb[2], argb[3]);
                        break;
                    case FORMAT_R:
                        setter.set(j, i, width, height, 255, value, 0, 0);
                        break;
                    case FORMAT_G:
                        setter.set(j, i, width, height, 255, 0, value, 0);
                        break;
                    case FORMAT_B:
                        setter.set(j, i, width, height, 255, 0, 0, value);
                        break;
                    case FORMAT_A:
                        setter.set(j, i, width, height, 255, value, value, value);
                        break;
                    default:
                        throw new IllegalStateException("Logical error!");
                }
            }
        }
    }
}
