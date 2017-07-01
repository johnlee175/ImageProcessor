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

import java.awt.Color;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.Image;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.MouseWheelEvent;
import java.awt.event.MouseWheelListener;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.util.concurrent.LinkedBlockingDeque;
import java.util.concurrent.TimeUnit;

import javax.imageio.ImageIO;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JViewport;
import javax.swing.SwingUtilities;
import javax.swing.WindowConstants;

import com.johnsoft.UiFace;

/**
 * @author John Kenrinus Lee
 * @version 2017-06-03
 */
public class SwingImageView implements UiFace.ImageView {
    private final ImageLabel imageLabel;

    public SwingImageView(File imageFile) {
        try {
            imageLabel = new ImageLabel(ImageIO.read(imageFile));
        } catch (IOException e) {
            throw new IllegalArgumentException(e);
        }
    }

    public SwingImageView(URL imageUrl) {
        try {
            imageLabel = new ImageLabel(ImageIO.read(imageUrl));
        } catch (IOException e) {
            throw new IllegalArgumentException(e);
        }
    }

    public SwingImageView(String imageFilePath) {
        try {
            imageLabel = new ImageLabel(ImageIO.read(new File(imageFilePath)));
        } catch (IOException e) {
            throw new IllegalArgumentException(e);
        }
    }

    public SwingImageView(InputStream imageSource) {
        try {
            imageLabel = new ImageLabel(ImageIO.read(imageSource));
        } catch (IOException e) {
            throw new IllegalArgumentException(e);
        }
    }

    public SwingImageView(int w, int h, int[] rgbArray) {
        this(0, 0, w, h, rgbArray, 0, w);
    }

    public SwingImageView(int startX, int startY, int w, int h,
                          int[] rgbArray, int offset, int scansize) {
        BufferedImage bufferedImage = new BufferedImage(w, h, BufferedImage.TYPE_INT_ARGB);
        bufferedImage.setRGB(startX, startY, w, h, rgbArray, offset, scansize);
        imageLabel = new ImageLabel(bufferedImage);
    }

    public int[] getRGB(int startX, int startY, int w, int h,
                 int[] rgbArray, int offset, int scansize) {
        return getImage().getRGB(startX, startY, w, h, rgbArray, offset, scansize);
    }

    public void setRGB(int startX, int startY, int w, int h,
                       int[] rgbArray, int offset, int scansize) {
        getImage().setRGB(startX, startY, w, h, rgbArray, offset, scansize);
    }

    private BufferedImage getImage() {
        return (BufferedImage) imageLabel.getImagePaintInfo().image;
    }

    @Override
    public UiFace.Peer getPeer() {
        return imageLabel;
    }

    @Override
    public Object getClientData() {
        return getImage();
    }

    private static final class ImageLabel extends JImageView implements UiFace.Peer {
        private MouseSupport mouseSupport;
        private WheelEventSupport wheelSupport;

        ImageLabel(Image image) {
            JImageView.ImagePaintInfo info = new JImageView.ImagePaintInfo();
            info.image = image;
            info.scaleType = JImageView.ImagePaintInfo.TYPE_ORIGINAL_CENTER;
            info.usePreferredSize = true;
            info.fillBlankBackground = Color.BLACK;
            setImagePaintInfo(info);
            mouseSupport = new MouseSupport(this);
            wheelSupport = new WheelEventSupport(this);
            addMouseListener(mouseSupport);
            addMouseMotionListener(mouseSupport);
            addMouseWheelListener(wheelSupport);
            addComponentListener(new ComponentAdapter() {
                @Override
                public void componentResized(ComponentEvent e) {
                    final Container container = getParent();
                    if (container instanceof JViewport) {
                        mouseSupport.setViewport((JViewport) container);
                        wheelSupport.setViewport((JViewport) container);
                    }
                }
            });
        }
    }

    private static final class MouseSupport extends MouseAdapter {
        private JImageView imageView;
        private JViewport viewport;
        private Point pressPoint;

        public MouseSupport(JImageView imageView) {
            this.imageView = imageView;
        }

        public void setViewport(JViewport viewport) {
            this.viewport = viewport;
        }

        @Override
        public void mouseClicked(MouseEvent e) {
            if (e.getButton() == MouseEvent.BUTTON1 && e.getClickCount() == 2) {
                Color color = imageView.colorAtPoint(e.getPoint());
                JDialog dialog = new JDialog((JFrame) null, true);
                JPanel content = new JPanel();
                JPanel colorPane = new JPanel();
                colorPane.setBackground(color);
                colorPane.setPreferredSize(new Dimension(30, 30));
                String description = '#' + Integer.toHexString(color.getRGB())
                        + ", a=" + color.getAlpha()
                        + ", r=" + color.getRed()
                        + ", g=" + color.getGreen()
                        + ", b=" + color.getBlue();
                content.add(colorPane);
                content.add(new JLabel(description));
                dialog.setContentPane(content);
                dialog.setTitle("Current Color On Click Pointer");
                dialog.setAlwaysOnTop(true);
                dialog.setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
                dialog.pack();
                dialog.setResizable(false);
                dialog.setLocationRelativeTo(null);
                dialog.setVisible(true);
            }
        }

        @Override
        public void mousePressed(final MouseEvent e) {
            pressPoint = e.getPoint();

        }

        @Override
        public void mouseReleased(final MouseEvent e) {
            pressPoint = null;
        }

        @Override
        public void mouseDragged(final MouseEvent e) {
            if (pressPoint != null) {
                final Point dragPoint = e.getPoint();
                final int xdiff = pressPoint.x - dragPoint.x;
                final int ydiff = pressPoint.y - dragPoint.y;

//                Point origin = viewport.getViewPosition();
//                origin.x += xdiff;
//                origin.y += ydiff;
//                if (origin.x < 0) {
//                    origin.x = 0;
//                }
//                if (origin.y < 0) {
//                    origin.y = 0;
//                }
//                viewport.setViewPosition(origin);
//                pressPoint = dragPoint;

                Rectangle view = viewport.getViewRect();
                view.x += xdiff;
                view.y += ydiff;

                imageView.scrollRectToVisible(view);
            }
        }

        @Override
        public void mouseWheelMoved(MouseWheelEvent e) {
            if (e.getScrollType() == MouseWheelEvent.WHEEL_UNIT_SCROLL) {
//                final Rectangle view = viewport.getViewRect();
                final Point view = viewport.getViewPosition();
                final boolean horizontal = (e.getModifiers() & MouseWheelEvent.SHIFT_MASK) != 0;
                final int delta = e.getUnitsToScroll() * 8;
                if (delta > 0) {
                    for (int i = 1; i <= delta; ++i) {
                        if (horizontal) {
                            view.x += 1;
                        } else {
                            view.y += 1;
                        }
//                        imageView.scrollRectToVisible(view);
                        viewport.setViewPosition(view);
                    }
                } else if (delta < 0) {
                    for (int i = -1; i >= delta; --i) {
                        if (horizontal) {
                            view.x -= 1;
                        } else {
                            view.y -= 1;
                        }
//                        imageView.scrollRectToVisible(view);
                        viewport.setViewPosition(view);
                    }
                }
            } else {
                System.out.println("wheel block scroll event ignored");
            }
        }
    }


    private static final class WheelEventSupport implements MouseWheelListener {
        private final LinkedBlockingDeque<WheelData> datas = new LinkedBlockingDeque<>();
        private final JImageView imageView;
        private final EventPeeker eventPeeker;
        private JViewport viewport;

        public WheelEventSupport(JImageView imageView) {
            this.imageView = imageView;
            this.eventPeeker = new EventPeeker();
        }

        public void setViewport(JViewport viewport) {
            this.viewport = viewport;
        }

        public void quit() {
            datas.clear();
            eventPeeker.isQuit = true;
            eventPeeker.interrupt();
        }

        @Override
        public void mouseWheelMoved(MouseWheelEvent e) {
            final boolean horizontal = (e.getModifiers() & MouseWheelEvent.SHIFT_MASK) != 0;
            final int delta = e.getUnitsToScroll();
            datas.offerLast(new WheelData(horizontal, delta));
        }

        private final class WheelData {
            boolean horizontal;
            int delta;

            public WheelData(boolean horizontal, int delta) {
                this.horizontal = horizontal;
                this.delta = delta;
            }
        }

        private final class EventPeeker extends Thread {
            boolean isQuit;

            public EventPeeker() {
                start();
            }

            @Override
            public void run() {
                try {
                    while (!isQuit) {
                        WheelData firstData = datas.takeFirst();
                        WheelData secondData = datas.pollFirst(100L, TimeUnit.MILLISECONDS);
                        if (secondData == null) {
                            move(firstData);
                        } else if (firstData.horizontal == secondData.horizontal) {
                            move(firstData);
                            datas.offerFirst(secondData);
                        } else {
                            final int deltaX, deltaY;
                            if (firstData.horizontal) {
                                deltaX = firstData.delta;
                                deltaY = secondData.delta;
                            } else {
                                deltaY = firstData.delta;
                                deltaX = secondData.delta;
                            }
                            move(deltaX, deltaY);
                        }
                    }
                } catch (InterruptedException ex) {
                    ex.printStackTrace();
                }
            }

            public void move(final WheelData data) {
                SwingUtilities.invokeLater(new Runnable() {
                    @Override
                    public void run() {
//                        final Rectangle view = viewport.getViewRect();
//                        if (data.horizontal) {
//                            view.x += data.delta;
//                        } else {
//                            view.y += data.delta;
//                        }
//                        imageView.scrollRectToVisible(view);
                        data.delta *= 6;
                        if (data.horizontal) {
                            smoothScroll(data.delta, 0);
                        } else {
                            smoothScroll(0, data.delta);
                        }
                    }
                });
            }

            public void move(int pDeltaX, int pDeltaY) {
                final int deltaX = pDeltaX * 2;
                final int deltaY = pDeltaY * 2;
                SwingUtilities.invokeLater(new Runnable() {
                    @Override
                    public void run() {
//                        final Rectangle view = viewport.getViewRect();
//                        view.x += deltaX;
//                        view.y += deltaY;
//                        imageView.scrollRectToVisible(view);
                        smoothScroll(deltaX, deltaY);
                    }
                });
            }

            public void smoothScroll(int deltaX, int deltaY) {
                final Rectangle view = viewport.getViewRect();
                final int srcX = view.x;
                final int srcY = view.y;
                final int dstX = srcX + deltaX;
                final int dstY = srcY + deltaY;
                for (int i = 0; i < 10; i++) {
                    view.x = srcX + Math.round(((dstX - srcX) / 10.0F) * i);
                    view.y = srcY + Math.round(((dstY - srcY) / 10.0F) * i);
                }
                imageView.scrollRectToVisible(view);
            }
        }
    }
}
