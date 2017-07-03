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

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.ComponentOrientation;
import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.FocusAdapter;
import java.awt.event.FocusEvent;
import java.awt.image.BufferedImage;

import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.WindowConstants;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;

import com.johnsoft.OnObject;
import com.johnsoft.alg.BaseImageProc;
import com.johnsoft.swing.GridBagAssembler;
import com.johnsoft.swing.JImageView;
import com.johnsoft.swing.MainThreadUtils;

/**
 * @author John Kenrinus Lee
 * @version 2017-07-01
 */
public class FilterColorPictureAction extends AbstractBaseFilterAction {
    private static final Color RED = new Color(175, 0, 0);
    private static final Color GREEN = new Color(0, 175, 0);
    private static final Color BLUE = new Color(0, 0, 175);

    private final BaseFilterGroup redGroup = new BaseFilterGroup(BaseImageProc.FILTER_TYPE_RED);
    private final BaseFilterGroup greenGroup = new BaseFilterGroup(BaseImageProc.FILTER_TYPE_GREEN);
    private final BaseFilterGroup blueGroup = new BaseFilterGroup(BaseImageProc.FILTER_TYPE_BLUE);
    private boolean applyToOrigin;
    private JImageView imageView;
    private JTextArea description;
    private int[] data;
    private int width;
    private int height;

    @Override
    protected int[] subAction(JFrame frame, int[] data, int w, int h) {
        this.data = data;
        this.width = w;
        this.height = h;
        final int openOption = JOptionPane.showConfirmDialog(null,
                "Apply to origin image? [Default overlay effect]",
                "Effect Tips", JOptionPane.YES_NO_OPTION, JOptionPane.QUESTION_MESSAGE);
        applyToOrigin = openOption == JOptionPane.YES_OPTION;
        imageView = newImageView(data, w, h);
        description = newTextArea(3, 3);
        JPanel tools = new JPanel(new GridLayout(3, 1));
        tools.add(redGroup.getView(RED));
        tools.add(greenGroup.getView(GREEN));
        tools.add(blueGroup.getView(BLUE));
        JPanel content = new JPanel(new BorderLayout());
        content.add(imageView, BorderLayout.CENTER);
        content.add(new JScrollPane(description), BorderLayout.SOUTH);
        content.add(tools, BorderLayout.EAST);
        showDialog(frame, content);
        final int saveOption = JOptionPane.showConfirmDialog(null,
                "Add result image to tabbed panel for save?",
                "Result Tips", JOptionPane.YES_NO_OPTION, JOptionPane.QUESTION_MESSAGE);
        if (saveOption == JOptionPane.YES_OPTION) {
            final BufferedImage bufferedImage = (BufferedImage) imageView.getImagePaintInfo().image;
            return bufferedImage.getRGB(0, 0, w, h, data, 0, w);
        }
        return null;
    }

    void doAction(long delayed) {
        MainThreadUtils.postDelay(new Runnable() {
            @Override
            public void run() {
                handleDocument(redGroup.getDocument());
                handleDocument(greenGroup.getDocument());
                handleDocument(blueGroup.getDocument());

                if (applyToOrigin) {
                    // state in jni
                    final int[] result = BaseImageProc.colorFilter(data, width, height);
                    final BufferedImage bufferedImage = new BufferedImage(width, height, BufferedImage.TYPE_INT_ARGB);
                    bufferedImage.setRGB(0, 0, width, height, result, 0, width);
                    imageView.getImagePaintInfo().image = bufferedImage;
                } else {
                    data = BaseImageProc.colorFilter(data, width, height);
                    final BufferedImage bufferedImage = (BufferedImage) imageView.getImagePaintInfo().image;
                    bufferedImage.setRGB(0, 0, width, height, data, 0, width);
                }

                imageView.repaint();
            }
        }, delayed);
    }

    private static void showDialog(JFrame frame, JComponent content) {
        JDialog dialog = new JDialog(frame, true);
        dialog.setContentPane(content);
        dialog.setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
        dialog.setSize(800, 600);
        dialog.setLocationRelativeTo(null);
        dialog.setVisible(true);
    }

    private class BaseFilterGroup {
        BaseFilterDocument document = new BaseFilterDocument();

        JComboBox<String> filter = newFilterSelect();
        JTextField cField = newTextField("0.0");
        JTextField lField = newTextField("1.0");
        JTextField gField = newTextField("1.0");

        JTextField hlvField = newTextField("255");
        JComboBox<String> hpField = newPolicySelect();
        JTextField llvField = newTextField("0");
        JComboBox<String> lpField = newPolicySelect();

        private long editDelayed = 2000L;
        private long changeSelectDelayed = 1000L;
        private long focusLostDelayed = 100L;

        BaseFilterGroup(int type) {
            document.type = type;
        }

        BaseFilterDocument getDocument() {
            return document;
        }

        JComponent getView(Color color) {
            JPanel panel = new JPanel();
            panel.setBackground(color);

            GridBagAssembler assembler = new GridBagAssembler(panel, ComponentOrientation.LEFT_TO_RIGHT);
            assembler.fill(GridBagAssembler.FILL_HORIZONTAL)
                    .wrapLine().gridwidth(2).add(filter).gridwidth(1)
                    .wrapLine().add(newLabel("c", color)).nextColumn().add(cField)
                    .wrapLine().add(newLabel("l", color)).nextColumn().add(lField)
                    .wrapLine().add(newLabel("g", color)).nextColumn().add(gField)
                    .wrapLine().add(newLabel("h-level", color)).nextColumn().add(hlvField)
                    .wrapLine().add(newLabel("h-policy", color)).nextColumn().add(hpField)
                    .wrapLine().add(newLabel("l-level", color)).nextColumn().add(llvField)
                    .wrapLine().add(newLabel("l-policy", color)).nextColumn().add(lpField);

            driverField(cField, new OnObject<JTextField>() {
                @Override
                public void on(JTextField textField) {
                    if (isFloat(textField)) {
                        document.c = Float.valueOf(textField.getText());
                    }
                }
            });
            driverField(lField, new OnObject<JTextField>() {
                @Override
                public void on(JTextField textField) {
                    if (isFloat(textField)) {
                        document.l = Float.valueOf(textField.getText());
                    }
                }
            });
            driverField(gField, new OnObject<JTextField>() {
                @Override
                public void on(JTextField textField) {
                    if (isFloat(textField)) {
                        document.g = Float.valueOf(textField.getText());
                    }
                }
            });
            driverField(hlvField, new OnObject<JTextField>() {
                @Override
                public void on(JTextField textField) {
                    if (isUint(textField)) {
                        document.highLevel = Integer.valueOf(textField.getText());
                    }
                }
            });
            driverField(llvField, new OnObject<JTextField>() {
                @Override
                public void on(JTextField textField) {
                    if (isUint(textField)) {
                        document.lowLevel = Integer.valueOf(textField.getText());
                    }
                }
            });

            driverComboBox(hpField, new OnObject<JComboBox>() {
                @Override
                public void on(JComboBox comboBox) {
                    final int idx = comboBox.getSelectedIndex();
                    document.highPolicy = idx < 0 ? 0 : idx;
                }
            });
            driverComboBox(lpField, new OnObject<JComboBox>() {
                @Override
                public void on(JComboBox comboBox) {
                    final int idx = comboBox.getSelectedIndex();
                    document.lowPolicy = idx < 0 ? 0 : idx;
                }
            });

            filter.setSelectedIndex(0);
            filter.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    updateFilterDescription(filter, description, getPrefix(document.type));
                    final int index = filter.getSelectedIndex();
                    if (index < 0) {
                        return;
                    }
                    document.filterIndex = index;
                    switch (index) {
                        case BaseImageProc.INDEX_NO_FILTER:
                            doAction(changeSelectDelayed);
                            break;
                        case BaseImageProc.INDEX_REVERSE_FILTER:
                            doAction(changeSelectDelayed);
                            break;
                        case BaseImageProc.INDEX_STRETCH_FILTER:
                            doAction(changeSelectDelayed);
                            break;
                        case BaseImageProc.INDEX_LINEAR_FILTER:
                            cField.setEnabled(true);
                            lField.setEnabled(true);
                            gField.setEnabled(false);
                            hlvField.setEnabled(false);
                            hpField.setEnabled(false);
                            llvField.setEnabled(false);
                            lpField.setEnabled(false);
                            break;
                        case BaseImageProc.INDEX_LOG_FILTER:
                            cField.setEnabled(true);
                            lField.setEnabled(true);
                            gField.setEnabled(true);
                            hlvField.setEnabled(false);
                            hpField.setEnabled(false);
                            llvField.setEnabled(false);
                            lpField.setEnabled(false);
                            break;
                        case BaseImageProc.INDEX_GAMMA_FILTER:
                            cField.setEnabled(true);
                            lField.setEnabled(true);
                            gField.setEnabled(true);
                            hlvField.setEnabled(false);
                            hpField.setEnabled(false);
                            llvField.setEnabled(false);
                            lpField.setEnabled(false);
                            break;
                        case BaseImageProc.INDEX_BINARY_FILTER:
                            cField.setEnabled(false);
                            lField.setEnabled(false);
                            gField.setEnabled(false);
                            hlvField.setEnabled(true);
                            hpField.setEnabled(true);
                            llvField.setEnabled(true);
                            lpField.setEnabled(true);
                            break;
                        default:
                            break;
                    }
                }
            });

            JScrollPane scrollPane = new JScrollPane(panel);
            scrollPane.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
            scrollPane.setPreferredSize(new Dimension(panel.getPreferredSize().width + 30, 0));
            return scrollPane;
        }

        private void driverField(final JTextField textField, final OnObject<JTextField> assign) {
            textField.getDocument().addDocumentListener(new DocumentListener() {
                @Override
                public void insertUpdate(DocumentEvent e) {
                    assign.on(textField);
                    if (isFieldsValid(document.filterIndex)) {
                        doAction(editDelayed);
                    }
                }
                @Override
                public void removeUpdate(DocumentEvent e) {
                    assign.on(textField);
                    if (isFieldsValid(document.filterIndex)) {
                        doAction(editDelayed);
                    }
                }
                @Override
                public void changedUpdate(DocumentEvent e) {
                    System.out.println("changedUpdate");
                }
            });
            textField.addFocusListener(new FocusAdapter() {
                @Override
                public void focusLost(FocusEvent e) {
                    assign.on(textField);
                    if (isFieldsValid(document.filterIndex)) {
                        doAction(focusLostDelayed);
                    }
                }
            });
        }

        private void driverComboBox(final JComboBox comboBox, final OnObject<JComboBox> assign) {
            comboBox.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    assign.on(comboBox);
                    if (isFieldsValid(document.filterIndex)) {
                        doAction(changeSelectDelayed);
                    }
                }
            });
        }

        private boolean isFieldsValid(int filterIndex) {
            if (filterIndex == BaseImageProc.INDEX_LINEAR_FILTER) {
                return isFloat(cField) && isFloat(lField);
            }
            if (filterIndex == BaseImageProc.INDEX_LOG_FILTER
                    || filterIndex == BaseImageProc.INDEX_GAMMA_FILTER) {
                return isFloat(cField) && isFloat(lField) && isFloat(gField);
            }
            if (filterIndex == BaseImageProc.INDEX_BINARY_FILTER) {
                return isUint(hlvField) && isUint(llvField) && isColorElement(hlvField) && isColorElement(llvField);
            }
            return true;
        }
    }

    private static String getPrefix(int type) {
        switch (type) {
            case BaseImageProc.FILTER_TYPE_RED:
                return "RED:\n";
            case BaseImageProc.FILTER_TYPE_GREEN:
                return "GREEN:\n";
            case BaseImageProc.FILTER_TYPE_BLUE:
                return "BLUE:\n";
            default:
                return "";
        }
    }

}
