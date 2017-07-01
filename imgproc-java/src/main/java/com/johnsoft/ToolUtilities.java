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
package com.johnsoft;

import java.io.File;
import java.util.Arrays;
import java.util.List;

/**
 * @author John Kenrinus Lee
 * @version 2017-06-30
 */
public class ToolUtilities {
    private static final List<File> ROOTS_FILE = Arrays.asList(File.listRoots());
    private static final List<String> IMAGE_EXTENDS = Arrays.asList(
            ".png", ".jpg", ".jpeg", "gif", "webp", "bmp", "tiff"
    );
    public static File validImagePath(String path) {
        File file = new File(path);
        File originFile = file;
        boolean endsWithImageType = false;
        for (String extend : IMAGE_EXTENDS) {
            if (file.getName().endsWith(extend)) {
                endsWithImageType = true;
                break;
            }
        }
        if (!endsWithImageType) {
            return null;
        }
        if (file.exists()) {
            return originFile;
        }
        while ((file = file.getParentFile()) != null) {
            if (file.exists() && !ROOTS_FILE.contains(file)) {
                return originFile;
            }
        }
        return null;
    }
}
