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

import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.WindowConstants;

import com.johnsoft.UiFace;
import com.johnsoft.swing.charts.Histogram;

/**
 * 重要备注: 目前只实现灰度图的均衡化.
 * 对彩色分量rgb分别做均衡化, 会产生奇异的点, 图像不和谐. 一般采用的是用yuv空间进行亮度的均衡即可.
 * @author John Kenrinus Lee
 * @version 2017-07-06
 */
public class ChartHistogramAction implements UiFace.Action {
    @Override
    public void action(UiFace uiFace, UiFace.Control control) {
        final UiFace.TabPane tabPane = uiFace.getTabPane();
        final BufferedImage image = (BufferedImage) tabPane.getActiveTabData();
        if (image != null) {
            final int w = image.getWidth();
            final int h = image.getHeight();
            int[] data = new int[w * h];
            image.getRGB(0, 0, w, h, data, 0, w);

            JDialog dialog = new JDialog((JFrame) null, false);
            dialog.setContentPane(Histogram.newAllTypeViews(data));
            dialog.setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
            dialog.setSize(1000, 680);
            dialog.setLocationRelativeTo(null);
            dialog.setVisible(true);
        }
    }
}
