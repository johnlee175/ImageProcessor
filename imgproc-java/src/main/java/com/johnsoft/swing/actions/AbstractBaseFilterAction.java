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
import java.awt.Dimension;
import java.awt.image.BufferedImage;
import java.util.regex.Pattern;

import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.SwingConstants;
import javax.swing.text.JTextComponent;

import com.johnsoft.UiFace;
import com.johnsoft.alg.BaseImageProc;
import com.johnsoft.swing.JImageView;
import com.johnsoft.swing.SwingImageView;

/**
 * @author John Kenrinus Lee
 * @version 2017-07-01
 */
public abstract class AbstractBaseFilterAction implements UiFace.Action {
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

    protected abstract int[] subAction(JFrame frame, int[] data, int w, int h);

    protected void handleDocument(BaseFilterDocument doc) {
        BaseImageProc.filterIndex(doc.type, doc.filterIndex);
        if (doc.filterIndex == BaseImageProc.INDEX_LINEAR_FILTER
                || doc.filterIndex == BaseImageProc.INDEX_LOG_FILTER
                || doc.filterIndex == BaseImageProc.INDEX_GAMMA_FILTER) {
            BaseImageProc.filterParam(doc.c, doc.l, doc.g);
        } else if (doc.filterIndex == BaseImageProc.INDEX_BINARY_FILTER) {
            BaseImageProc.filterParam2(doc.lowLevel, doc.lowPolicy, doc.highLevel, doc.highPolicy);
        }
    }

    protected static class BaseFilterDocument {
        int type = BaseImageProc.FILTER_TYPE_GREY;
        int filterIndex = BaseImageProc.INDEX_NO_FILTER;
        float c = 0.0F;
        float l = 1.0F;
        float g = 1.0F;
        int highLevel = 255;
        int highPolicy = BaseImageProc.BINARY_FILTER_POLICY_SAME;
        int lowLevel = 0;
        int lowPolicy = BaseImageProc.BINARY_FILTER_POLICY_SAME;
    }

    protected static String getDefaultFilterDescription(String prefix) {
        return prefix + "No Parameters required!";
    }

    protected static void updateFilterDescription(JComboBox<String> filter, JTextComponent description,
                                                  String prefix) {
        final int idx = filter.getSelectedIndex();
        switch (idx) {
            case 0:
                description.setText(prefix + "No Parameters required!");
                break;
            case 1:
                description.setText(prefix + "No Parameters required!\n"
                        + "result = 255 - value;\n"
                        + "value in [0, 255], result in [0, 255]");
                break;
            case 2:
                description.setText(prefix + "result = c + l * value;");
                break;
            case 3:
                description.setText(prefix + "result = c + l * log(value * g + 1) / (log(g + 1) + 0.001);\n"
                        + "value in [0, 1], result in [0, 1]");
                break;
            case 4:
                description.setText(prefix + "result = c + l * pow(value, g);\n"
                        + "value in [0, 1], result in [0, 1]");
                break;
            case 5:
                description.setText(prefix + "No Parameters required!\n"
                        + "x = 0.5 / (value + 0.05);\n"
                        + "e1 = log(1 / 0.05 - 1) / log(0.5 / 0.05);\n"
                        + "e2 = log(1 / 0.95 - 1) / log(0.5 / 0.95);\n"
                        + "e = ceil(min(e1, e2));\n"
                        + "result = 1 / (1 + pow(temp, e));\n"
                        + "value in [0, 1], result in [0, 1]");
                break;
            case 6:
                description.setText(prefix + "h-level >= 0 && value >= h-level; then apply h-policy to value;\n"
                        + "l-level >= 0 && value <= l-level; then apply l-policy to value;\n");
                break;
            default:
                break;
        }
    }

    protected static JLabel newLabel(String text, Color color) {
        JLabel label = new JLabel(text);
        label.setHorizontalAlignment(SwingConstants.RIGHT);
        int grey = (color.getRed() * 38 + color.getGreen() * 75 + color.getBlue() * 15) >>> 7;
        if (grey > 128) {
            label.setForeground(Color.BLACK);
        } else {
            label.setForeground(Color.WHITE);
        }
        return label;
    }

    protected static JTextField newTextField(String text) {
        return new JTextField(text, 5);
    }

    protected static JTextArea newTextArea(int row, int col) {
        JTextArea textArea = new JTextArea(row, col);
        textArea.setEditable(false);
        textArea.setLineWrap(true);
        textArea.setWrapStyleWord(true);
        return textArea;
    }

    protected static JComboBox<String> newFilterSelect() {
        return new JComboBox<>(new String[] {
                "no_filter",
                "reverse_filter",
                "linear_filter",
                "log_filter",
                "gamma_filter",
                "stretch_filter",
                "binary_filter",
        });
    }

    protected static JComboBox<String> newPolicySelect() {
        return new JComboBox<>(new String[] {
                "same", "0", "255"
        });
    }

    protected static JImageView newImageView(int[] data, int w, int h) {
        JImageView imageView = new JImageView();
        JImageView.ImagePaintInfo info = new JImageView.ImagePaintInfo();
        BufferedImage bufferedImage = new BufferedImage(w, h, BufferedImage.TYPE_INT_ARGB);
        bufferedImage.setRGB(0, 0, w, h, data, 0, w);
        info.image = bufferedImage;
        info.scaleType = JImageView.ImagePaintInfo.TYPE_RATIO_CENTER;
        imageView.setImagePaintInfo(info);
        imageView.setPreferredSize(new Dimension(500, 500));
        return imageView;
    }

    protected static boolean isFloat(JTextComponent text) {
        return floatPattern.matcher(text.getText().trim()).matches();
    }

    protected static boolean isUint(JTextComponent text) {
        return uintPattern.matcher(text.getText().trim()).matches();
    }

    protected static boolean isColorElement(JTextComponent text) {
        int value = Integer.valueOf(text.getText());
        return value >= 0 && value <= 255;
    }

    private static final Pattern uintPattern = Pattern.compile("0|(^[1-9][0-9]*$)");
    private static final Pattern floatPattern = Pattern.compile("^(-?\\d+)(\\.\\d+)?$");
}
