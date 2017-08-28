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

import java.nio.ByteBuffer;

/**
 * Define DirectMemory, and make factory
 * @author John Kenrinus Lee
 * @version 2017-08-18
 */
public class DirectByteBuffers {
    public interface DirectMemory {
        ByteBuffer mallocDirect(int capacity);
        void freeDirect(ByteBuffer byteBuffer);
    }

    public static class JavaDirectMemory implements DirectMemory {
        @Override
        public ByteBuffer mallocDirect(int capacity) {
            return ByteBuffer.allocateDirect(capacity);
        }

        @Override
        public void freeDirect(ByteBuffer byteBuffer) {
            // Do nothing
        }
    }

    public static class NativeDirectMemory implements DirectMemory {
        static {
            System.loadLibrary("imgproc_android_camera");
        }

        public native ByteBuffer mallocDirect(int capacity);
        public native void freeDirect(ByteBuffer byteBuffer);
    }

    public static DirectMemory createJavaDirectMemory() {
        return new JavaDirectMemory();
    }

    public static DirectMemory createNativeDirectMemory() {
        return new NativeDirectMemory();
    }
}
