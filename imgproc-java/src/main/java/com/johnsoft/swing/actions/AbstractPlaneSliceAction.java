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

import java.awt.Color;
import java.awt.ComponentOrientation;
import java.awt.Dimension;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.image.BufferedImage;
import java.util.ArrayList;

import javax.swing.BorderFactory;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.WindowConstants;

import com.johnsoft.UiFace;
import com.johnsoft.alg.BaseImageProc;
import com.johnsoft.swing.GridBagAssembler;
import com.johnsoft.swing.JImageView;
import com.johnsoft.swing.SwingImageView;

/**
 * @author John Kenrinus Lee
 * @version 2017-07-04
 */
public abstract class AbstractPlaneSliceAction implements UiFace.Action {
    @Override
    public void action(UiFace uiFace, UiFace.Control control) {
        final UiFace.TabPane tabPane = uiFace.getTabPane();
        final BufferedImage image = (BufferedImage) tabPane.getActiveTabData();
        if (image != null) {
            final int w = image.getWidth();
            final int h = image.getHeight();
            final int[] data = new int[w * h];
            image.getRGB(0, 0, w, h, data, 0, w);

            final SelectableImageView[] imageViews = getImageViews(data, w, h);
            final int count = imageViews.length;
            final int rowCount = getRowCount();
            if (count <= 0 || rowCount <= 0) {
                return;
            }
            final int columnCount = count / rowCount;

            final JPanel panel = new JPanel();
            GridBagAssembler assembler = new GridBagAssembler(panel, ComponentOrientation.LEFT_TO_RIGHT);
            assembler.fill(GridBagAssembler.FILL_NONE).insets(5, 5, 5, 5);
            for (int i = 0; i < rowCount; ++i) {
                assembler.wrapLine();
                for (int j = 0; j < columnCount; ++j) {
                    assembler.nextColumn().add(imageViews[i * columnCount + j]);
                }
            }

            final JScrollPane scrollPane = new JScrollPane(panel);
            scrollPane.getVerticalScrollBar().setUnitIncrement(4);
            scrollPane.getHorizontalScrollBar().setUnitIncrement(4);
            final JDialog dialog = new JDialog((JFrame) null, "Plane Slice");
            dialog.setContentPane(scrollPane);
            dialog.setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
            if (shouldCombineSave()) {
                dialog.addWindowListener(new WindowAdapter() {
                    @Override
                    public void windowClosed(WindowEvent e) {
                        ArrayList<int[]> list = new ArrayList<>(count);
                        SelectableImageView iv;
                        BufferedImage image;
                        for (int i = 0; i < count; ++i) {
                            iv = imageViews[i];
                            if (iv.isSelected()) {
                                image = (BufferedImage) iv.getImagePaintInfo().image;
                                final int w = image.getWidth();
                                final int h = image.getHeight();
                                list.add(image.getRGB(0, 0, w, h, null, 0, w));
                            }
                        }
                        if (!list.isEmpty()) {
                            final int[] argb;
                            if (list.size() == 1) {
                                argb = list.get(0);
                            } else {
                                final int[][] datas = new int[list.size()][];
                                list.toArray(datas);
                                argb = BaseImageProc.combineSimplePlane(datas, w, h);
                            }
                            final SwingImageView resultView = new SwingImageView(w, h, argb);
                            final String title = tabPane.getActiveTabIdentifier();
                            tabPane.addTabWithExistTitle(title, resultView);
                        }
                    }
                });
            }
            dialog.setSize(800, 600);
            dialog.setLocationRelativeTo(null);
            dialog.setVisible(true);
        }
    }

    protected abstract SelectableImageView[] getImageViews(int[] data, int w, int h);

    protected abstract int getRowCount();

    protected abstract boolean shouldCombineSave();

    protected static SelectableImageView newImageView(int[] data, int w, int h) {
        SelectableImageView imageView = new SelectableImageView();
        JImageView.ImagePaintInfo info = new JImageView.ImagePaintInfo();
        BufferedImage bufferedImage = new BufferedImage(w, h, BufferedImage.TYPE_INT_ARGB);
        bufferedImage.setRGB(0, 0, w, h, data, 0, w);
        info.image = bufferedImage;
        info.usePreferredSize = true;
        info.noResize = true;
        info.scaleType = JImageView.ImagePaintInfo.TYPE_RATIO_CENTER;
        imageView.setImagePaintInfo(info);
        imageView.setPreferredSize(new Dimension(300, 300));
        return imageView;
    }

    protected static final class SelectableImageView extends JImageView {
        private boolean selected;

        public SelectableImageView() {
            addMouseListener(new MouseAdapter() {
                @Override
                public void mouseClicked(MouseEvent e) {
                    if (e.getButton() == MouseEvent.BUTTON1 && e.getClickCount() == 1) {
                        setSelected(!isSelected());
                    }
                }
            });
        }

        public void setSelected(boolean selected) {
            this.selected = selected;
            if (selected) {
                setBorder(BorderFactory.createLineBorder(Color.CYAN, 4));
            } else {
                setBorder(BorderFactory.createEmptyBorder());
            }
        }

        public boolean isSelected() {
            return selected;
        }
    }
}
