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
package com.johnsoft.swing;

import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;

import javax.swing.JDialog;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.filechooser.FileFilter;

/**
 * @author John Kenrinus Lee
 * @version 2017-06-09
 */
public class DialogUtilities {
    public static void showInfoMessageBox(String message) {
        JOptionPane.showMessageDialog(null, message, "Info Tips", JOptionPane.INFORMATION_MESSAGE);
    }

    public static void showErrorMessageBox(Throwable thr) {
        JOptionPane.showMessageDialog(null, thr.toString(), "Error Tips", JOptionPane.ERROR_MESSAGE);
    }

    public static void showErrorMessageBox(String message) {
        JOptionPane.showMessageDialog(null, message, "Error Tips", JOptionPane.ERROR_MESSAGE);
    }

    public static File[] showOpenImageFileDialog() {
        return new SimpleOpenFileDialog("Open Image File Dialog",
                new FileFilter() {
                    @Override
                    public boolean accept(File f) {
                        final String fileName = f.getName();
                        return f.isDirectory() || fileName.endsWith(".png") || fileName.endsWith("webp")
                                || fileName.endsWith(".jpg") || fileName.endsWith(".jpeg");
                    }

                    @Override
                    public String getDescription() {
                        return "*.png, *.jpg, *.webp";
                    }
                }).showWindow().getSelectFiles();
    }

    public static File showSaveImageFileDialog() {
        return new SimpleSaveFileDialog("Save Image File Dialog",
                new FileFilter() {
                    @Override
                    public boolean accept(File f) {
                        final String fileName = f.getName();
                        return f.isDirectory()
                                || fileName.endsWith(".png")
                                || fileName.endsWith(".jpg");
                    }

                    @Override
                    public String getDescription() {
                        return "*.png, *.jpg";
                    }
                }).showWindow().getSelectFile();
    }

    private static final class SimpleOpenFileDialog extends JDialog {
        private File[] selectedFiles;

        SimpleOpenFileDialog(String title, FileFilter fileFilter) {
            super((JFrame)null, title, true);
            setAlwaysOnTop(true);
            final JFileChooser fileChooser = new JFileChooser();
            fileChooser.setDialogType(JFileChooser.OPEN_DIALOG);
            fileChooser.setDragEnabled(true);
            fileChooser.setFileHidingEnabled(false);
            fileChooser.setMultiSelectionEnabled(true);
            fileChooser.setFileSelectionMode(JFileChooser.FILES_ONLY);
            fileChooser.setFileFilter(fileFilter);
            fileChooser.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    if (JFileChooser.APPROVE_SELECTION.equals(e.getActionCommand())) {
                        selectedFiles = fileChooser.getSelectedFiles();
                    }
                    dispose();
                }
            });
            setContentPane(fileChooser);
            setSize(fileChooser.getPreferredSize());
            setMinimumSize(new Dimension(600, 200));
        }

        SimpleOpenFileDialog showWindow() {
            setLocationRelativeTo(null);
            setVisible(true);
            return this;
        }

        File[] getSelectFiles() {
            return selectedFiles;
        }
    }

    private static final class SimpleSaveFileDialog extends JDialog {
        private File selectedFile;

        SimpleSaveFileDialog(String title, FileFilter fileFilter) {
            super((JFrame)null, title, true);
            setAlwaysOnTop(true);
            final JFileChooser fileChooser = new JFileChooser();
            fileChooser.setDialogType(JFileChooser.SAVE_DIALOG);
            fileChooser.setDragEnabled(true);
            fileChooser.setFileHidingEnabled(false);
            fileChooser.setMultiSelectionEnabled(false);
            fileChooser.setFileSelectionMode(JFileChooser.FILES_ONLY);
            fileChooser.setAcceptAllFileFilterUsed(false);
            fileChooser.addChoosableFileFilter(fileFilter);
            fileChooser.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    if (JFileChooser.APPROVE_SELECTION.equals(e.getActionCommand())) {
                        selectedFile = fileChooser.getSelectedFile();
                    }
                    dispose();
                }
            });
            setContentPane(fileChooser);
            setSize(fileChooser.getPreferredSize());
            setMinimumSize(new Dimension(600, 200));
        }

        SimpleSaveFileDialog showWindow() {
            setLocationRelativeTo(null);
            setVisible(true);
            return this;
        }

        File getSelectFile() {
            return selectedFile;
        }
    }
}
