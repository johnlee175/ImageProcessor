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

import java.awt.ComponentOrientation;
import java.awt.image.BufferedImage;

import javax.swing.JCheckBox;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.WindowConstants;

import com.johnsoft.UiFace;
import com.johnsoft.alg.BaseImageProc;
import com.johnsoft.swing.DialogUtilities;
import com.johnsoft.swing.GridBagAssembler;
import com.johnsoft.swing.SwingImageView;

/**
 * @author John Kenrinus Lee
 * @version 2017-07-04
 */
public class PlaneCombineBitAction implements UiFace.Action {
    @Override
    public void action(UiFace uiFace, UiFace.Control control) {
        final UiFace.TabPane tabPane = uiFace.getTabPane();
        final BufferedImage image = (BufferedImage) tabPane.getActiveTabData();
        if (image != null) {
            final int w = image.getWidth();
            final int h = image.getHeight();
            final int[] data = new int[w * h];
            image.getRGB(0, 0, w, h, data, 0, w);

            final JTextField maskField = new JTextField(10);
            final JCheckBox checkAsGrey = new JCheckBox("Using grey picture");
            final JTextArea help = new JTextArea("Hex Letter -> Bin (4 bits)\n"
                    + "1 -> 0001;\t\t2 -> 0010;\t\t3 -> 0011;\t\t4 -> 0100;\n"
                    + "5 -> 0101;\t\t6 -> 0110;\t\t7 -> 0111;\t\t8 -> 1000;\n"
                    + "9 -> 1001;\t\tA -> 1010;\t\tB -> 1011;\t\tC -> 1100;\n"
                    + "D -> 1101;\t\tE -> 1110;\t\tF -> 1111;\t\t0 -> 0000;\n");
            help.setEnabled(false);
            final JLabel label = new JLabel("Input Hex Mask: ");
            final JPanel content = new JPanel();
            final GridBagAssembler assembler = new GridBagAssembler(content, ComponentOrientation.LEFT_TO_RIGHT);
            assembler.fill(GridBagAssembler.FILL_HORIZONTAL)
                    .wrapLine().gridwidth(3).add(help).gridwidth(1)
                    .wrapLine().add(label)
                    .nextColumn().add(maskField)
                    .nextColumn().add(checkAsGrey);

            final JDialog dialog = new JDialog((JFrame) uiFace.getMainWindow().getPeer(), true);
            dialog.setContentPane(content);
            dialog.setAlwaysOnTop(true);
            dialog.setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
            dialog.pack();
            dialog.setResizable(false);
            dialog.setLocationRelativeTo(null);
            dialog.setVisible(true);

            final int type;
            if (checkAsGrey.isSelected()) {
                type = BaseImageProc.TYPE_GREY_COLOR;
            } else {
                type = BaseImageProc.TYPE_ARGB_COLOR;
            }

            int mask;
            try {
                mask = Long.valueOf(maskField.getText().trim(), 16).intValue();
                if (type == BaseImageProc.TYPE_GREY_COLOR) {
                    mask = mask & 0xFF;
                } else {
                    mask = mask | 0xFF000000;
                }
            } catch (NumberFormatException ex) {
                DialogUtilities.showErrorMessageBox("Need hex int ( > 0) for text input");
                return;
            }

            final int[] result = BaseImageProc.combineBitsPlane(data, w, h, type, mask);
            final SwingImageView resultView = new SwingImageView(w, h, result);
            final String title = tabPane.getActiveTabIdentifier();
            tabPane.addTabWithExistTitle(title, resultView);
        }
    }
}
