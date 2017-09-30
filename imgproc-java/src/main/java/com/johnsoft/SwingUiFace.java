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
import javax.swing.WindowConstants;

import com.johnsoft.alg.BaseImageProc;
import com.johnsoft.swing.DialogUtilities;
import com.johnsoft.swing.JImageView;
import com.johnsoft.swing.SwingImageView;
import com.johnsoft.swing.SwingMainWindow;
import com.johnsoft.swing.SwingMenuManager;
import com.johnsoft.swing.SwingTabPane;
import com.johnsoft.swing.actions.ChartHistogramAction;
import com.johnsoft.swing.actions.DelegateAction;
import com.johnsoft.swing.actions.FilterColorPictureAction;
import com.johnsoft.swing.actions.FilterGreyPictureAction;
import com.johnsoft.swing.actions.GpuImageProcAction;
import com.johnsoft.swing.actions.PlaneCombineBitAction;
import com.johnsoft.swing.actions.PlaneSliceColorBitAction;
import com.johnsoft.swing.actions.PlaneSliceColorCompAction;
import com.johnsoft.swing.actions.PlaneSliceGreyBitAction;
import com.johnsoft.swing.actions.SimpleHistEqualizeAction;
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
        } else if ("Filter-Grey-Picture".equals(command)) {
            return new FilterGreyPictureAction();
        } else if ("Filter-Color-Picture".equals(command)) {
            return new DelegateAction(new DelegateAction.Delegate() {
                @Override
                public Action getInstance() {
                    return new FilterColorPictureAction();
                }
            });
        } else if ("Filter-Grey-Bit-Plane".equals(command)) {
            return new PlaneSliceGreyBitAction();
        } else if ("Filter-Color-Bit-Plane".equals(command)) {
            return new PlaneSliceColorBitAction();
        } else if ("Filter-Color-Component-Plane".equals(command)) {
            return new PlaneSliceColorCompAction();
        } else if ("Filter-Combine-Bit-Plane".equals(command)) {
            return new PlaneCombineBitAction();
        } else if ("Filter-Simple-Histogram-Equalization-Photoshop".equals(command)) {
            return SimpleHistEqualizeAction.algAction();
        } else if ("Filter-Simple-Histogram-Equalization-Grey".equals(command)) {
            return new SimpleHistEqualizeAction(BaseImageProc.TYPE_GREY_COLOR);
        } else if ("Filter-Simple-Histogram-Equalization-Red".equals(command)) {
            return new SimpleHistEqualizeAction(BaseImageProc.COMPONENT_RED);
        } else if ("Filter-Simple-Histogram-Equalization-Green".equals(command)) {
            return new SimpleHistEqualizeAction(BaseImageProc.COMPONENT_GREEN);
        } else if ("Filter-Simple-Histogram-Equalization-Blue".equals(command)) {
            return new SimpleHistEqualizeAction(BaseImageProc.COMPONENT_BLUE);
        } else if ("Filter-Simple-Histogram-Equalization-Color".equals(command)) {
            return new SimpleHistEqualizeAction(BaseImageProc.TYPE_ARGB_COLOR);
        } else if ("Filter-GPU-normal".equals(command)) {
            return new GpuImageProcAction("normal");
        } else if ("Filter-GPU-negative".equals(command)) {
            return new GpuImageProcAction("negative");
        } else if ("Filter-GPU-sketch".equals(command)) {
            return new GpuImageProcAction("sketch");
        } else if ("Filter-GPU-white_skin".equals(command)) {
            return new GpuImageProcAction("white_skin");
        } else if ("Chart-Histogram".equals(command)) {
            return new ChartHistogramAction();
        }
        return EMPTY_ACTION;
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
                    dialog.setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
                    dialog.setContentPane(imageView);
                    dialog.setSize(600, 600);
                    dialog.setLocationRelativeTo(null);
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
