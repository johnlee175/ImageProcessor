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
package com.johnsoft;


import java.awt.event.ActionEvent;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import java.util.Arrays;
import java.util.List;
import java.util.Locale;

import javax.imageio.ImageIO;
import javax.imageio.spi.IIORegistry;
import javax.imageio.spi.ImageReaderSpi;
import javax.imageio.spi.ImageWriterSpi;
import javax.swing.AbstractAction;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;
import javax.swing.UIManager;

import com.johnsoft.swing.DialogUtilities;
import com.johnsoft.swing.JImageView;
import com.johnsoft.swing.SwingImageView;
import com.johnsoft.swing.SwingMainWindow;
import com.johnsoft.swing.SwingMenuManager;
import com.johnsoft.swing.SwingTabPane;
import com.twelvemonkeys.imageio.plugins.tiff.TIFFImageReaderSpi;
import com.twelvemonkeys.imageio.plugins.tiff.TIFFImageWriterSpi;

import net.sf.javavp8decoder.imageio.WebPImageReaderSpi;

/**
 * @author John Kenrinus Lee
 * @version 2017-06-03
 */
public class SwingUiFace implements UiFace {
    private static SwingUiFace swingUiFace = new SwingUiFace();

    public static SwingUiFace getDefault() {
        return swingUiFace;
    }

    private MainWindow mainWindow;
    private TabPane tabPane;
    private MenuManager menuManager;

    public SwingUiFace() {
        Locale.setDefault(Locale.US);
        try {
            UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
        } catch (Exception e) {
            e.printStackTrace();
        }
        IIORegistry.getDefaultInstance()
                .registerServiceProvider(new WebPImageReaderSpi(), ImageReaderSpi.class);
        IIORegistry.getDefaultInstance()
                .registerServiceProvider(new TIFFImageReaderSpi(), ImageReaderSpi.class); // slowly
        IIORegistry.getDefaultInstance()
                .registerServiceProvider(new TIFFImageWriterSpi(), ImageWriterSpi.class); // slowly
    }

    public MainWindow getMainWindow() {
        if (mainWindow == null) {
            mainWindow = new SwingMainWindow(this);
            mainWindow.setMenu(getMenuManager());
            mainWindow.setContent(getTabPane());
        }
        return mainWindow;
    }

    public TabPane getTabPane() {
        if (tabPane == null) {
            tabPane = new SwingTabPane(this);
        }
        return tabPane;
    }

    public MenuManager getMenuManager() {
        if (menuManager == null) {
            menuManager = new SwingMenuManager(this);
        }
        return menuManager;
    }

    public Action getAction(String command) {
        if ("Open-File".equals(command)) {
            return new Action() {
                public void action(UiFace uiFace, Control control) {
                    File[] files = DialogUtilities.showOpenImageFileDialog();
                    if (files != null) {
                        onReceiveFiles(Arrays.asList(files));
                    }
                }
            };
        } else if ("Save-File".equals(command)) {
            return new Action() {
                public void action(UiFace uiFace, Control control) {
                    final BufferedImage image = (BufferedImage) getTabPane().getActiveTabData();
                    if (image != null) {
                        try {
                            final File saveFile = DialogUtilities.showSaveImageFileDialog();
                            if (saveFile == null) {
                                return;
                            }
                            String fileName = saveFile.getName();
                            if (fileName.endsWith(".png")) {
                                ImageIO.write(image, "png", saveFile);
                            } else if (fileName.endsWith(".jpg") || fileName.endsWith(".jpeg")) {
                                ImageIO.write(image, "jpg", saveFile);
                            } else {
                                ImageIO.write(image, "png", new File(saveFile.getAbsolutePath() + ".png"));
                            }
                        } catch (IOException e) {
                            DialogUtilities.showErrorMessageBox(e);
                        }
                    }
                }
            };
        }
        return  EMPTY_ACTION;
    }

    @Override
    public void onReceiveFiles(final List<File> files) {
        for (final File file : files) {
            SwingImageView swingImageView = new SwingImageView(file);
            JComponent component = (JComponent) swingImageView.getPeer();
            JPopupMenu menu = new JPopupMenu();
            menu.add(new JMenuItem(new AbstractAction("show scale view") {
                @Override
                public void actionPerformed(final ActionEvent e) {
                    final BufferedImage image;
                    try {
                        image = ImageIO.read(file);
                    } catch (IOException ex) {
                        DialogUtilities.showErrorMessageBox(ex);
                        return;
                    }
                    JDialog dialog = new JDialog((JFrame) getMainWindow().getPeer());
                    JImageView imageView = new JImageView();
                    JImageView.ImagePaintInfo info = new JImageView.ImagePaintInfo();
                    info.image = image;
                    info.scaleType = JImageView.ImagePaintInfo.TYPE_RATIO_CENTER;
                    imageView.setImagePaintInfo(info);
                    dialog.setContentPane(imageView);
                    dialog.setSize(600, 600);
                    dialog.setVisible(true);
                }
            }));
            component.setComponentPopupMenu(menu);
            getTabPane().addTab(file.getAbsolutePath(), swingImageView);
        }
    }

    private static final Action EMPTY_ACTION = new Action() {
        public void action(UiFace uiFace, Control control) {
        }
    };
}
