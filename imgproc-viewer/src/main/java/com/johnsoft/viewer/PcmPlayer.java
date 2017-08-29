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

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.DataLine;
import javax.sound.sampled.LineUnavailableException;
import javax.sound.sampled.SourceDataLine;

/**
 * @author John Kenrinus Lee
 * @version 2017-04-09
 */
public class PcmPlayer {
    public static void main(String[] args) {
//        try {
//            play(new File("out.pcm"), SAMPLE_RATE_16K, SAMPLE_SIZE_16BIT_SHORT, CHANNEL_MONO, true, false);
//        } catch (Exception e) {
//            e.printStackTrace();
//        }

        parseAndPlay(args[0]);
    }

    public static final int SAMPLE_SIZE_8BIT_BYTE = 8;
    public static final int SAMPLE_SIZE_16BIT_SHORT = 16;
    public static final int SAMPLE_SIZE_32BIT_FLOAT = 32;

    public static final float SAMPLE_RATE_8K = 8000F;
    public static final float SAMPLE_RATE_11K = 11025F;
    public static final float SAMPLE_RATE_16K = 16000F;
    public static final float SAMPLE_RATE_22K = 22050F;
    public static final float SAMPLE_RATE_32K = 32000F;
    public static final float SAMPLE_RATE_44K = 44100F;
    public static final float SAMPLE_RATE_48K = 48000F;

    public static final int CHANNEL_MONO = 1;
    public static final int CHANNEL_STEREO = 2;

    /**
     * @param pcmFilePath like 'outname_unsigned_le_mono_short_16k.pcm', must not be start with '_'
     */
    public static void parseAndPlay(String pcmFilePath) {
        File file = new File(pcmFilePath);
        if (!file.exists()) {
            throw new IllegalArgumentException("input file not exists!");
        }
        String name = file.getName();
        if (!name.endsWith(".pcm")) {
            throw new IllegalArgumentException("not end with .pcm");
        }
        int i = name.lastIndexOf('.');
        int j = name.lastIndexOf('_', i - 1);
        if (j <= 0) {
            throw new IllegalArgumentException("not found sample rate");
        }
        final float sampleRate;
        String sampleRateStr = name.substring(j + 1, i);
        if ("8k".equals(sampleRateStr)) {
            sampleRate = SAMPLE_RATE_8K;
        } else if ("11k".equals(sampleRateStr)) {
            sampleRate = SAMPLE_RATE_11K;
        } else if ("16k".equals(sampleRateStr)) {
            sampleRate = SAMPLE_RATE_16K;
        } else if ("22k".equals(sampleRateStr)) {
            sampleRate = SAMPLE_RATE_22K;
        } else if ("32k".equals(sampleRateStr)) {
            sampleRate = SAMPLE_RATE_32K;
        } else if ("44k".equals(sampleRateStr)) {
            sampleRate = SAMPLE_RATE_44K;
        } else if ("48k".equals(sampleRateStr)) {
            sampleRate = SAMPLE_RATE_48K;
        } else {
            throw new IllegalArgumentException("unknown sample rate");
        }
        i = name.lastIndexOf('_', j - 1);
        if (i <= 0) {
            throw new IllegalArgumentException("not found sample size");
        }
        final int sampleSizeInBits;
        String sampleSizeStr = name.substring(i + 1, j);
        if ("byte".equals(sampleSizeStr)) {
            sampleSizeInBits = SAMPLE_SIZE_8BIT_BYTE;
        } else if ("short".equals(sampleSizeStr)) {
            sampleSizeInBits = SAMPLE_SIZE_16BIT_SHORT;
        } else if ("float".equals(sampleSizeStr)) {
            sampleSizeInBits = SAMPLE_SIZE_32BIT_FLOAT;
        } else {
            throw new IllegalArgumentException("unknown sample size");
        }
        j = name.lastIndexOf('_', i - 1);
        if (j <= 0) {
            throw new IllegalArgumentException("not found channels");
        }
        final int channels;
        String channelsStr = name.substring(j + 1, i);
        if ("mono".equals(channelsStr)) {
            channels = CHANNEL_MONO;
        } else if ("stereo".equals(channelsStr)) {
            channels = CHANNEL_STEREO;
        } else {
            throw new IllegalArgumentException("unknown channels");
        }
        i = name.lastIndexOf('_', j - 1);
        if (i <= 0) {
            throw new IllegalArgumentException("not found byte order");
        }
        final boolean bigEndian;
        String byteOrderStr = name.substring(i + 1, j);
        if ("be".equals(byteOrderStr)) {
            bigEndian = true;
        } else if ("le".equals(byteOrderStr)) {
            bigEndian = false;
        } else {
            throw new IllegalArgumentException("unknown byte order");
        }
        boolean signed = true;
        j = name.lastIndexOf('_', i - 1);
        if (j > 0) {
            String unsigned = name.substring(j + 1, i);
            if ("unsigned".equals(unsigned)) {
                signed = false;
            }
        }
        try {
            play(file, sampleRate, sampleSizeInBits, channels, signed, bigEndian);
        } catch (Exception e) {
            throw new IllegalStateException(e);
        }
    }

    /**
     * @param file PCM文件
     * @param sampleRate  采样率, see also {@link #SAMPLE_RATE_8K}, {@link #SAMPLE_RATE_11K}, {@link #SAMPLE_RATE_16K},
     *                    {@link #SAMPLE_RATE_22K}, {@link #SAMPLE_RATE_32K}, {@link #SAMPLE_RATE_44K},
     *                    {@link #SAMPLE_RATE_48K}
     * @param sampleSizeInBits 采样精度位数, see also {@link #SAMPLE_SIZE_8BIT}, {@link #SAMPLE_SIZE_16BIT_SHORT},
     *                         {@link #SAMPLE_SIZE_24BIT}, {@link #SAMPLE_SIZE_32BIT_FLOAT}
     * @param channels 采样声道数, see also {@link #CHANNEL_MONO}, {@link #CHANNEL_STEREO}
     * @param signed 指示数据是有符号还是无符号的, 一般设为true
     * @param bigEndian 指示数据是否以大端字节序存放, false表示little-endian小端字节序
     */
    public static void play(File file, float sampleRate,
                            int sampleSizeInBits,
                            int channels,
                            boolean signed,
                            boolean bigEndian) throws LineUnavailableException, IOException {
        if (sampleSizeInBits % 8 != 0) {
            throw new IOException("not support sample size which can't mod with 8");
        }
        final int sampleSizeInBytes = sampleSizeInBits / 8;
        final int frameBufferSize = (int) (sampleRate * sampleSizeInBytes * channels); // 1s data
        final AudioFormat audioFormat = new AudioFormat((signed
                ? AudioFormat.Encoding.PCM_SIGNED : AudioFormat.Encoding.PCM_UNSIGNED),
                sampleRate,
                sampleSizeInBits,
                channels,
                sampleSizeInBytes * channels,
                sampleRate,
                bigEndian);
        final SourceDataLine.Info info = new DataLine.Info(SourceDataLine.class, audioFormat, frameBufferSize);
        final SourceDataLine sdl = (SourceDataLine) AudioSystem.getLine(info);
        final byte[] buffer = new byte[frameBufferSize];
        BufferedInputStream bis = null;
        int readCount = 0, writeCount = 0;
        try {
            sdl.open(audioFormat);
            bis = new BufferedInputStream(new FileInputStream(file));
            sdl.start();
            while ((readCount = bis.read(buffer)) > 0) {
                while ((writeCount += sdl.write(buffer, 0, readCount)) < readCount);
                writeCount = 0;
            }
            sdl.stop();
        } finally {
            if (bis != null) {
                try {
                    bis.close();
                } catch (IOException ignored) {
                    /* do nothing */
                }
            }
            if (sdl != null && sdl.isOpen()) {
                sdl.close();
            }
        }
    }
}
