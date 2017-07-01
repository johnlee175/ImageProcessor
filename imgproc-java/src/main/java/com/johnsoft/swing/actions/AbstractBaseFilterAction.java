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
import java.util.regex.Pattern;

import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JTextArea;
import javax.swing.SwingConstants;
import javax.swing.text.JTextComponent;

import com.johnsoft.UiFace;
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

    protected static class BaseFilterDocument {
        int type;
        int filterIndex;
        float c;
        float l;
        float g;
        int highLevel;
        int highPolicy;
        int lowLevel;
        int lowPolicy;
    }

    protected static JLabel newLabel(String text) {
        JLabel label = new JLabel(text);
        label.setHorizontalAlignment(SwingConstants.RIGHT);
        return label;
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
