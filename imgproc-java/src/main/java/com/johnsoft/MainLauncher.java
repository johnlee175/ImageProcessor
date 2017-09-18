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

import java.awt.image.BufferedImage;
import java.io.IOException;

import javax.imageio.ImageIO;

import com.johnsoft.alg.BaseImageProc;
import com.johnsoft.swing.SwingImageView;

/**
 * Support jpg/jpeg, png, gif, bmp, webp, tiff
 * more format see https://github.com/haraldk/TwelveMonkeys/tree/master/imageio
 *
 * @author John Kenrinus Lee
 * @version 2017-06-03
 */
public class MainLauncher {
    public static void main(String[] args) throws Exception {
//        mainTestSwing();
//        DebugUtils.enableSwingAllDebugLog();
        UiFace face = SwingUiFace.getDefault();
        face.getMenuManager()
                .addMenus("File", "Filter", "Chart")
                .addMenuItem("File", "Open", face.getAction("Open-File"))
                .addMenuItem("File", "Save", face.getAction("Save-File"))
                .addMenuItem("Filter", "Grey Picture",
                        face.getAction("Filter-Grey-Picture"))
                .addMenuItem("Filter", "Color Picture",
                        face.getAction("Filter-Color-Picture"))
                .addMenuItem("Filter", "Grey Bit Plane",
                        face.getAction("Filter-Grey-Bit-Plane"))
                .addMenuItem("Filter", "Color Bit Plane",
                        face.getAction("Filter-Color-Bit-Plane"))
                .addMenuItem("Filter", "Color Component Plane",
                        face.getAction("Filter-Color-Component-Plane"))
                .addMenuItem("Filter", "Combine Bit Plane",
                        face.getAction("Filter-Combine-Bit-Plane"))
                .addSubMenu("Filter", "Simple Histogram Equalization")
                .addCheckedMenuItem("Simple Histogram Equalization", "use photoshop alg",
                        false, face.getAction("Filter-Simple-Histogram-Equalization-Photoshop"))
                .addMenuItem("Simple Histogram Equalization", "Grey",
                        face.getAction("Filter-Simple-Histogram-Equalization-Grey"))
                .addMenuItem("Simple Histogram Equalization", "Red",
                        face.getAction("Filter-Simple-Histogram-Equalization-Red"))
                .addMenuItem("Simple Histogram Equalization", "Green",
                        face.getAction("Filter-Simple-Histogram-Equalization-Green"))
                .addMenuItem("Simple Histogram Equalization", "Blue",
                        face.getAction("Filter-Simple-Histogram-Equalization-Blue"))
                .addMenuItem("Simple Histogram Equalization", "All",
                        face.getAction("Filter-Simple-Histogram-Equalization-Color"))
                .addSubMenu("Filter", "GPU")
                .addMenuItem("GPU", "normal",
                        face.getAction("Filter-GPU-normal"))
                .addMenuItem("Chart", "Histogram",
                        face.getAction("Chart-Histogram"))
                .assemble();
        face.getMainWindow().show("ImageProcessor", 800, 600);
    }

    private static void mainTestSwing() {
        UiFace.ImageView image1 = new SwingImageView(MainLauncher.class.getResource("/mm_a.png"));
        UiFace.ImageView image2 = new SwingImageView(MainLauncher.class.getResource("/mm_b.png"));
        BufferedImage image;
        try {
            image = ImageIO.read(MainLauncher.class.getResource("/mm_j1.jpg"));
        } catch (IOException e) {
            throw new IllegalStateException(e);
        }
        int[] ints = new int[200 * 200];
        image.getRGB(100, 50, 200, 200, ints, 0, 200);

//        for (int i = 0; i < ints.length; i++) {
//            int avg = (int) (((ints[i] & 0xFF) + ((ints[i] >>> 8) & 0xFF) + ((ints[i] >>> 16) & 0xFF)) / 3.0F);
//            ints[i] = (255 << 24) + (avg << 16) + (avg << 8) + avg;
//        }

//        ints = BaseImageProc.greyFilter(ints, 200, 200);
        ints = BaseImageProc.colorFilter(ints, 200, 200);

        UiFace.ImageView image3 = new SwingImageView(200, 200, ints);

        UiFace face = new SwingUiFace();
        face.getTabPane().addTab("Hello", image1).addTab("World", image2).addTab("Welcom", image3);
        face.getMenuManager()
                .addMenus("File", "Edit", "View", "Tools", "Window", "Help")
                .addMenuItem("File", "Open", new UiFace.Action() {
                    public void action(UiFace uiFace, UiFace.Control control) {
                        System.out.println("Click");
                    }
                })
                .addMenuItem("File", "Save", null)
                .addMenuItem("Edit", "Copy", null)
                .addMenuItem("Edit", "Delete", null)
                .addSubMenu("Edit", "More")
                .addMenuItem("More", "Find", null)
                .addMenuItem("More", "Replace", null)
                .addCheckedMenuItem("Edit", "Check", false, new UiFace.Action() {
                    public void action(UiFace uiFace, UiFace.Control control) {
                        System.out.println(control.isCheckedOrSelected());
                    }
                }).assemble();
        face.getMenuManager().setEnabled("Edit/More/Replace", false);
        face.getMainWindow().show("ImageProcessor", 800, 600);
    }
}
