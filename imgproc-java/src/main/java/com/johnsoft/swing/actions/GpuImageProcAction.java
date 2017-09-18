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
package com.johnsoft.swing.actions;

import java.awt.image.BufferedImage;

import javax.swing.JFrame;

import com.johnsoft.UiFace;
import com.johnsoft.alg.SimpleGpuProc;
import com.johnsoft.swing.SwingImageView;

/**
 * @author John Kenrinus Lee
 * @version 2017-09-18
 */
public class GpuImageProcAction implements UiFace.Action {
    @Override
    public void action(UiFace uiFace, UiFace.Control control) {
        final UiFace.TabPane tabPane = uiFace.getTabPane();
        final BufferedImage image = (BufferedImage) tabPane.getActiveTabData();
        if (image != null) {
            final int w = image.getWidth();
            final int h = image.getHeight();
            int[] data = new int[w * h];
            image.getRGB(0, 0, w, h, data, 0, w);
            data = subAction((JFrame) uiFace.getMainWindow().getPeer(), data, w, h);
            if (data != null) {
                final SwingImageView resultView = new SwingImageView(w, h, data);
                final String title = tabPane.getActiveTabIdentifier();
                tabPane.addTabWithExistTitle(title, resultView);
            }
        }
    }

    protected int[] subAction(JFrame frame, int[] data, int w, int h) {
        return SimpleGpuProc.getDefault().imageProc(data, w, h, 0, null);
    }
}
