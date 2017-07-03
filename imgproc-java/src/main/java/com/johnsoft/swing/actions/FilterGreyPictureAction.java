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
import java.awt.Window;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.WindowConstants;

import com.johnsoft.alg.BaseImageProc;
import com.johnsoft.swing.DialogUtilities;
import com.johnsoft.swing.GridBagAssembler;

/**
 * @author John Kenrinus Lee
 * @version 2017-07-01
 */
public class FilterGreyPictureAction extends AbstractBaseFilterAction {
    @Override
    protected int[] subAction(JFrame frame, int[] data, int w, int h) {
        JDialog dialog = new JDialog(frame, true);
        BaseFilterView view = new BaseFilterView(0, dialog);
        dialog.setContentPane(view.getView(true));
        dialog.setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
        dialog.pack();
        dialog.setMinimumSize(dialog.getPreferredSize());
        dialog.setLocationRelativeTo(null);
        dialog.setVisible(true);
        if (!view.isApply()) {
            return null;
        }
        if (view.isFieldsInvalid()) {
            DialogUtilities.showInfoMessageBox("Parameters invalid");
            return null;
        }
        handleDocument(view.getDocument());
        return BaseImageProc.greyFilter(data, w, h);
    }

    private static class BaseFilterView {
        private JComboBox<String> filter = newFilterSelect();
        private JTextArea description = newTextArea(3, 30);

        private JTextField cField = newTextField("0.0");
        private JTextField lField = newTextField("1.0");
        private JTextField gField = newTextField("1.0");

        private JTextField hlvField = newTextField("255");
        private JComboBox<String> hpField = newPolicySelect();
        private JTextField llvField = newTextField("0");
        private JComboBox<String> lpField = newPolicySelect();

        private final int type;
        private Window window;
        private boolean apply;

        private BaseFilterView(int type, final Window window) {
            this.type = type;
            this.window = window;
            window.addWindowListener(new WindowAdapter() {
                @Override
                public void windowClosed(WindowEvent e) {
                    apply = false;
                    window.dispose();
                }
            });
        }

        JComponent getView(boolean showButtons) {
            JPanel panel = new JPanel();
            BoxLayout boxLayout = new BoxLayout(panel, BoxLayout.X_AXIS);
            panel.setLayout(boxLayout);

            JButton applyBtn = new JButton("Apply");
            JButton cancelBtn = new JButton("Cancel");
            if (!showButtons) {
                applyBtn.setVisible(false);
                cancelBtn.setVisible(false);
            } else {
                applyBtn.addActionListener(new ActionListener() {
                    @Override
                    public void actionPerformed(ActionEvent e) {
                        apply = true;
                        window.dispose();
                    }
                });
                cancelBtn.addActionListener(new ActionListener() {
                    @Override
                    public void actionPerformed(ActionEvent e) {
                        apply = false;
                        window.dispose();
                    }
                });
            }

            description.setText(getDefaultFilterDescription("GREY:\n"));
            filter.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    updateFilterDescription(filter, description, "GREY:\n");
                }
            });
            filter.setSelectedIndex(0);

            int gap = 20;
            Color color = panel.getBackground();
            new GridBagAssembler(panel, ComponentOrientation.LEFT_TO_RIGHT)
                    .fill(GridBagAssembler.FILL_HORIZONTAL)
                    .padding(30, 30, 30, 30)
                    .wrapLine().gridwidth(7).add(filter).gridwidth(1)
                    .wrapLine().add(newLabel("c", color))
                    .nextColumn().insets(0, 0, 0, gap).add(cField).zeroInsets()
                    .nextColumn().add(newLabel("l", color))
                    .nextColumn().insets(0, 0, 0, gap).add(lField).zeroInsets()
                    .nextColumn().add(newLabel("g", color))
                    .nextColumn().insets(0, 0, 0, gap).add(gField).zeroInsets()
                    .wrapLine().add(newLabel("h-level", color))
                    .nextColumn().insets(0, 0, 0, gap).add(hlvField).zeroInsets()
                    .nextColumn().add(newLabel("h-policy", color))
                    .nextColumn().insets(0, 0, 0, gap).add(hpField).zeroInsets()
                    .wrapLine().add(newLabel("l-level", color))
                    .nextColumn().insets(0, 0, 0, gap).add(llvField).zeroInsets()
                    .nextColumn().add(newLabel("l-policy", color))
                    .nextColumn().insets(0, 0, 0, gap).add(lpField).zeroInsets()
                    .wrapLine().size(6, 2).add(new JScrollPane(description))
                    .size(1, 1).gridx(6).add(applyBtn).nextRow().add(cancelBtn);

            return panel;
        }

        boolean isFieldsInvalid() {
            final int filterIndex = filter.getSelectedIndex();
            if (filterIndex == BaseImageProc.INDEX_LINEAR_FILTER) {
                return !isFloat(cField) || !isFloat(lField);
            }
            if (filterIndex == BaseImageProc.INDEX_LOG_FILTER
                    || filterIndex == BaseImageProc.INDEX_GAMMA_FILTER) {
                return !isFloat(cField) || !isFloat(lField) || !isFloat(gField);
            }
            if (filterIndex == BaseImageProc.INDEX_BINARY_FILTER) {
                return !isUint(hlvField) || !isUint(llvField) || !isColorElement(hlvField) || !isColorElement(llvField);
            }
            return false;
        }

        boolean isApply() {
            return apply;
        }

        BaseFilterDocument getDocument() {
            BaseFilterDocument doc = new BaseFilterDocument();
            doc.type = type;
            doc.filterIndex = Math.max(filter.getSelectedIndex(), 0);
            doc.c = Float.valueOf(cField.getText());
            doc.l = Float.valueOf(lField.getText());
            doc.g = Float.valueOf(gField.getText());
            doc.highLevel = Integer.valueOf(hlvField.getText());
            doc.highPolicy = Math.max(hpField.getSelectedIndex(), 0);
            doc.lowLevel = Integer.valueOf(llvField.getText());
            doc.lowPolicy = Math.max(lpField.getSelectedIndex(), 0);
            return doc;
        }
    }
}
