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
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Image;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.image.BufferedImage;

import javax.swing.JLabel;

/**
 * @author John Kenrinus Lee
 * @version 2017-06-04
 */
public class JImageView extends JLabel {
    /** Not thread safe, should change value in ui thread */
    public static class ImagePaintInfo implements Cloneable {
        public static final int TYPE_CUSTOM = 0;
        public static final int TYPE_FIT_IMAGE_SIZE = 1;
        public static final int TYPE_FIT_VIEW_SIZE = 2;
        public static final int TYPE_FILL_IMAGE_VIEW = 3;
        public static final int TYPE_RATIO_CENTER = 4;
        public static final int TYPE_RATIO_START = 5;
        public static final int TYPE_ORIGINAL_CENTER = 6;
        public static final int TYPE_ORIGINAL_START = 7;
        public static final int TYPE_FULL_WIDTH_CENTER = 8;
        public static final int TYPE_FULL_WIDTH_START = 9;
        public static final int TYPE_FULL_HEIGHT_CENTER = 10;
        public static final int TYPE_FULL_HEIGHT_START = 11;

        /** you can create {@link java.awt.image.BufferedImage} or using {@link javax.imageio.ImageIO} */
        public Image image;
        /** default no fill */
        public Color fillBlankBackground;
        /** see source code */
        public int scaleType;
        /** default use clip bounds */
        public boolean usePreferredSize;
        /** normalize clipping image area, only valid if scaleType in {@link #TYPE_CUSTOM},
         * {@link #TYPE_FILL_IMAGE_VIEW}, {@link #TYPE_FIT_IMAGE_SIZE}, {@link #TYPE_FIT_VIEW_SIZE} */
        public int srcX, srcY, srcW, srcH;
        /** normalize mapping view area */
        public int dstX, dstY, dstW, dstH;

        @Override
        public ImagePaintInfo clone() {
            try {
                return (ImagePaintInfo) super.clone();
            } catch (CloneNotSupportedException e) {
                ImagePaintInfo cloneOne = new ImagePaintInfo();
                cloneOne.image = image;
                cloneOne.fillBlankBackground = fillBlankBackground;
                cloneOne.scaleType = scaleType;
                cloneOne.usePreferredSize = usePreferredSize;
                cloneOne.srcX = srcX;
                cloneOne.srcY = srcY;
                cloneOne.srcW = srcW;
                cloneOne.srcH = srcH;
                cloneOne.dstX = dstX;
                cloneOne.dstY = dstY;
                cloneOne.dstW = dstW;
                cloneOne.dstH = dstH;
                return cloneOne;
            }
        }

        @Override
        public String toString() {
            return "ImagePaintInfo{" +
                    "image=" + image +
                    ", fillBlankBackground=" + fillBlankBackground +
                    ", scaleType=" + scaleType +
                    ", usePreferredSize=" + usePreferredSize +
                    ", srcX=" + srcX +
                    ", srcY=" + srcY +
                    ", srcW=" + srcW +
                    ", srcH=" + srcH +
                    ", dstX=" + dstX +
                    ", dstY=" + dstY +
                    ", dstW=" + dstW +
                    ", dstH=" + dstH +
                    '}';
        }
    }

    private ImagePaintInfo imagePaintInfo;

    public ImagePaintInfo getImagePaintInfo() {
        return imagePaintInfo;
    }

    public void setImagePaintInfo(ImagePaintInfo imagePaintInfo) {
        this.imagePaintInfo = imagePaintInfo;
    }

    public Color colorAtPoint(Point point) {
        BufferedImage bufferedImage = new BufferedImage(getWidth(), getHeight(), BufferedImage.TYPE_INT_ARGB);
        paint(bufferedImage.getGraphics());
        return new Color(bufferedImage.getRGB(point.x, point.y));
    }

    @Override
    public void doLayout() {
        ImagePaintInfo pi = getImagePaintInfo();
        if (pi.usePreferredSize && pi.image != null) {
            setPreferredSize(new Dimension(Math.max(getWidth(), pi.image.getWidth(null)),
                    Math.max(getHeight(), pi.image.getHeight(null))));
        }
        super.doLayout();
    }

    @Override
    protected void paintComponent(final Graphics g) {
        paintImageFromInfo(this, g, getImagePaintInfo());
    }

    private static void paintImageFromInfo(JImageView view, Graphics g, ImagePaintInfo pi) {
        if (pi.image == null) {
            return;
        }

        Rectangle clipBounds = g.getClipBounds();
        Rectangle preferredSize = new Rectangle(view.getPreferredSize());
        if (clipBounds == null) {
            clipBounds = preferredSize;
        }
        Rectangle rectangle = pi.usePreferredSize ? preferredSize : clipBounds;
        int imageWidth = pi.image.getWidth(null);
        int imageHeight = pi.image.getHeight(null);

        if (pi.fillBlankBackground != null) {
            g.setColor(pi.fillBlankBackground);
            g.fillRect(clipBounds.x, clipBounds.y, clipBounds.width, clipBounds.height);
        }

        switch (pi.scaleType) {
            case ImagePaintInfo.TYPE_CUSTOM: {
                g.drawImage(pi.image, pi.dstX, pi.dstY, (pi.dstX + pi.dstW), (pi.dstY + pi.dstH),
                        pi.srcX, pi.srcY, (pi.srcX + pi.srcW), (pi.srcY + pi.srcH), null);
            }
                break;
            case ImagePaintInfo.TYPE_FIT_IMAGE_SIZE: {
                g.drawImage(pi.image, pi.dstX, pi.dstY, (pi.dstX + pi.dstW), (pi.dstY + pi.dstH),
                        0, 0, imageWidth, imageHeight, null);
            }
                break;
            case ImagePaintInfo.TYPE_FIT_VIEW_SIZE: {
                g.drawImage(pi.image, rectangle.x, rectangle.y,
                        (rectangle.x + rectangle.width), (rectangle.y + rectangle.height),
                        pi.srcX, pi.srcY, (pi.srcX + pi.srcW), (pi.srcY + pi.srcH), null);
            }
                break;
            case ImagePaintInfo.TYPE_FILL_IMAGE_VIEW: { // scale base view
                g.drawImage(pi.image, rectangle.x, rectangle.y,
                        (rectangle.x + rectangle.width), (rectangle.y + rectangle.height),
                        0, 0, imageWidth, imageHeight, null);
            }
                break;
            case ImagePaintInfo.TYPE_RATIO_CENTER: { // scale base ratio of long side or both
                Dimension scaledSize = getScaledSizeByRatio(rectangle.width, rectangle.height,
                        imageWidth, imageHeight);
                g.drawImage(pi.image,
                        rectangle.x + (rectangle.width - scaledSize.width) / 2,
                        rectangle.y + (rectangle.height - scaledSize.height) / 2,
                        scaledSize.width, scaledSize.height, null);
            }
                break;
            case ImagePaintInfo.TYPE_RATIO_START: { // scale base ratio of long side or both
                Dimension scaledSize = getScaledSizeByRatio(rectangle.width, rectangle.height,
                        imageWidth, imageHeight);
                g.drawImage(pi.image, rectangle.x, rectangle.y, scaledSize.width, scaledSize.height, null);
            }
                break;
            case ImagePaintInfo.TYPE_ORIGINAL_CENTER: { // no scale
                g.drawImage(pi.image,
                        rectangle.x + (rectangle.width - imageWidth) / 2,
                        rectangle.y + (rectangle.height - imageHeight) / 2,
                        imageWidth, imageHeight, null);
            }
                break;
            case ImagePaintInfo.TYPE_ORIGINAL_START: { // no scale
                g.drawImage(pi.image, rectangle.x, rectangle.y, imageWidth, imageHeight, null);
            }
                break;
            case ImagePaintInfo.TYPE_FULL_WIDTH_CENTER: {
                g.drawImage(pi.image,
                        rectangle.x + (rectangle.width - imageWidth) / 2,
                        rectangle.y + (rectangle.height - imageHeight) / 2,
                        rectangle.width, (int) (rectangle.width * (imageHeight / (float) imageWidth)), null);
            }
                break;
            case ImagePaintInfo.TYPE_FULL_WIDTH_START: {
                g.drawImage(pi.image, rectangle.x, rectangle.y,
                        rectangle.width, (int) (rectangle.width * (imageHeight / (float) imageWidth)), null);
            }
                break;
            case ImagePaintInfo.TYPE_FULL_HEIGHT_CENTER: {
                g.drawImage(pi.image,
                        rectangle.x + (rectangle.width - imageWidth) / 2,
                        rectangle.y + (rectangle.height - imageHeight) / 2,
                        (int) (rectangle.height * (imageWidth / (float) imageHeight)), rectangle.height, null);
            }
                break;
            case ImagePaintInfo.TYPE_FULL_HEIGHT_START: {
                g.drawImage(pi.image, rectangle.x, rectangle.y,
                        (int) (rectangle.height * (imageWidth / (float) imageHeight)), rectangle.height, null);
            }
                break;
            default:
                throw new IllegalArgumentException("Unknown scale type!");
        }
    }

    private static Dimension getScaledSizeByRatio(int viewWidth, int viewHeight, int imageWidth, int imageHeight) {
        int scaledWidth, scaledHeight;
        if (viewWidth >= imageWidth && viewHeight >= imageHeight) {
            scaledWidth = imageWidth;
            scaledHeight = imageHeight;
        } else if (viewWidth >= imageWidth && viewHeight < imageHeight) {
            scaledHeight = viewHeight;
            scaledWidth = (int) (scaledHeight * (imageWidth / (float) imageHeight));
        } else if (viewWidth < imageWidth && viewHeight >= imageHeight) {
            scaledWidth = viewWidth;
            scaledHeight = (int) (scaledWidth * (imageHeight / (float) imageWidth));
        } else {
            float imageRatio = imageWidth / (float) imageHeight;
            float viewRatio = viewWidth / (float) viewHeight;
            if (viewRatio <= imageRatio) {
                scaledWidth = viewWidth;
                scaledHeight = (int) (scaledWidth * (imageHeight / (float) imageWidth));
            } else {
                scaledHeight = viewHeight;
                scaledWidth = (int) (scaledHeight * (imageWidth / (float) imageHeight));
            }
        }
        return new Dimension(scaledWidth, scaledHeight);
    }
}
