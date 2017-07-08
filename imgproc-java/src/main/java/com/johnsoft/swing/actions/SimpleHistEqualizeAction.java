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

import com.johnsoft.UiFace;
import com.johnsoft.alg.BaseImageProc;
import com.johnsoft.swing.SwingImageView;

/**
 * 对彩色分量rgb分别做均衡化, 会产生奇异的点, 图像不和谐. 后续采用yuv空间进行亮度的均衡即可, 以后实现.
 * @author John Kenrinus Lee
 * @version 2017-07-08
 */
public class SimpleHistEqualizeAction implements UiFace.Action {
    private static volatile boolean sUsingPhotoshop;
    private static final UiFace.Action algAction = new UiFace.Action() {
        @Override
        public void action(UiFace uiFace, UiFace.Control control) {
            if (control.isCheckedOrSelected()) {
                sUsingPhotoshop = true;
            } else {
                sUsingPhotoshop = false;
            }
        }
    };

    public static UiFace.Action algAction() {
        return algAction;
    }

    private final int type;

    public SimpleHistEqualizeAction(int type) {
        this.type = type;
    }

    @Override
    public void action(UiFace uiFace, UiFace.Control control) {
        final UiFace.TabPane tabPane = uiFace.getTabPane();
        final BufferedImage image = (BufferedImage) tabPane.getActiveTabData();
        if (image != null) {
            final int w = image.getWidth();
            final int h = image.getHeight();
            int[] data = new int[w * h];
            image.getRGB(0, 0, w, h, data, 0, w);
            if (type == BaseImageProc.TYPE_ARGB_COLOR) {
                data = histogramEqualization(data, BaseImageProc.COMPONENT_RED);
                data = histogramEqualization(data, BaseImageProc.COMPONENT_GREEN);
                data = histogramEqualization(data, BaseImageProc.COMPONENT_BLUE);
            } else {
                data = histogramEqualization(data, type);
            }
            final SwingImageView resultView = new SwingImageView(w, h, data);
            final String title = tabPane.getActiveTabIdentifier();
            tabPane.addTabWithExistTitle(title, resultView);
        }
    }

    protected int[] histogramEqualization(int[] data, int type) {
        return BaseImageProc.simpleHistogramEqualization(data, type, !sUsingPhotoshop);
    }
}
