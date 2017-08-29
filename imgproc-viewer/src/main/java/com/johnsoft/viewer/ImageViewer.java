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
package com.johnsoft.viewer;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Image;
import java.awt.Point;
import java.awt.RenderingHints;
import java.awt.Window;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.image.BufferedImage;
import java.io.File;
import java.util.Locale;

import javax.swing.Icon;
import javax.swing.ImageIcon;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.SwingUtilities;
import javax.swing.UIManager;
import javax.swing.WindowConstants;

/**
 * @author John Kenrinus Lee
 * @version 2017-03-30
 */
public class ImageViewer {
    public static void main(final String[] args) {
        Locale.setDefault(Locale.ENGLISH);
        SwingUtilities.invokeLater(new Runnable() {
            @Override
            public void run() {
                try {
                    int frameIndex = 0;
                    if (args.length > 1) {
                        frameIndex = Integer.parseInt(args[1]);
                    }
                    UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
                    new ImageViewer(new File(args[0]), frameIndex, null);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
    }

    public ImageViewer(File file, int frameIndex, Runnable onClosed) {
        FileParser parser = FileParser.parseFormat(file);
        FileParser.FileHead head = parser.getHead();
        final BufferedImage image = new BufferedImage(head.width, head.height, BufferedImage.TYPE_INT_ARGB);
        parser.fillToBuffer(head, frameIndex, new FileParser.SafeColorSetter() {
            @Override
            public void onColor(int x, int y, int w, int h, int alpha, int rOrY, int gOrCb, int bOrCr) {
                image.setRGB(x, y, ARGBFileParser.convertArgbToColor(alpha, rOrY, gOrCb, bOrCr));
            }
        });
        prepareWindow(file.getName(), image, onClosed);
    }

    private void prepareWindow(final String fileName, final BufferedImage image, final Runnable onClosed) {
        JFrame frame = new JFrame("ImageViewer[" + fileName + "]");
        JLabel imageView = new JLabel(new ImageIcon(image)) {
            @Override
            public void paint(Graphics g) {
                Graphics2D g2 = (Graphics2D) g;
                g2.setRenderingHint(RenderingHints.KEY_INTERPOLATION, RenderingHints.VALUE_INTERPOLATION_BILINEAR);
                ImageIcon imageIcon = (ImageIcon) getIcon();
                if (imageIcon != null) {
                    g2.drawImage(imageIcon.getImage(), 0, 0, getWidth(), getHeight(), null);
                }
            }
        };
        imageView.addMouseListener(new MouseAdapter() {
            @Override
            public void mouseClicked(MouseEvent e) {
                if (e.getButton() != MouseEvent.BUTTON1 || e.getClickCount() > 2) {
                    return;
                }
                final Object source = e.getSource();
                if (source != null && source instanceof JLabel) {
                    final JLabel imageView = (JLabel) source;
                    final Dimension preferredSize = imageView.getPreferredSize();
                    if (e.getClickCount() == 2) {
                        imageView.setSize((int) preferredSize.getWidth(), (int) preferredSize.getHeight());
                        ((Window) imageView.getTopLevelAncestor()).pack();
                        return;
                    }
                    if (imageView.getWidth() != (int) preferredSize.getWidth()
                            || imageView.getHeight() != (int) preferredSize.getHeight()) {
                        System.err.println("Please double-click to resize view to image size");
                        return;
                    }
                    final Icon icon = imageView.getIcon();
                    if (icon != null && icon instanceof ImageIcon) {
                        final ImageIcon imageIcon = (ImageIcon) icon;
                        final Image imageIconImage = imageIcon.getImage();
                        if (imageIconImage != null && imageIconImage instanceof BufferedImage) {
                            final BufferedImage bufferedImage = (BufferedImage) imageIconImage;
                            final int pixelX = e.getX();
                            final int pixelY = e.getY();
                            if (pixelX > 0 && pixelX < bufferedImage.getWidth()
                                    && pixelY > 0 && pixelY < bufferedImage.getHeight()) {
                                final int color = bufferedImage.getRGB(pixelX, pixelY);
                                System.err.println("Click - " + (new Point(pixelX, pixelY)) + (new Color(color)));
                            }
                        }
                    }
                }
            }
        });
        frame.setContentPane(imageView);
        frame.addWindowListener(new WindowAdapter() {
            @Override
            public void windowClosed(WindowEvent e) {
                if (onClosed != null) {
                    onClosed.run();
                } else {
                    System.exit(0);
                }
            }
        });
        frame.setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
        frame.pack();
        frame.setLocationRelativeTo(null);
        frame.setVisible(true);
    }
}
