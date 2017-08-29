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

import javax.swing.SwingUtilities;

/**
 * @author John Kenrinus Lee
 * @version 2017-04-11
 */
public class Test {
    public static void main(String[] args) {
        test1();
        test2();
    }

    private static final File resDir = new File("~/github/RawARGBYCbCrViewer/src/main/resources/images");

    private static int index;
    private static File[] res;
    private static Runnable task;

    private static void test1() {
        index = 0;
        res = resDir.listFiles();
        task = new Runnable() {
            @Override
            public void run() {
                SwingUtilities.invokeLater(new Runnable() {
                    @Override
                    public void run() {
                        if (res.length > index) {
                            File file = res[index++];
                            System.out.println("current file: " + file);
                            new ImageViewer(file, 0, task);
                        }
                    }
                });
            }
        };
        task.run();
    }

    private static void test2() {
        for (File file : resDir.listFiles()) {
            int frames = FileParser.getFrameCount(FileParser.parseFormat(file).getHead());
            System.err.println(file + " frame count: " + frames);
        }
    }
}
