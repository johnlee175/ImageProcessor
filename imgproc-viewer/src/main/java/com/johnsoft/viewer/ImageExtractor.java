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

import java.io.BufferedOutputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

/**
 * @author John Kenrinus Lee
 * @version 2017-04-11
 */
public class ImageExtractor {
    public static void main(String[] args) {
        try {
            int frameIndex = 0;
            if (args.length > 2) {
                frameIndex = Integer.parseInt(args[2]);
            }
            new ImageExtractor(new File(args[0]), args[1], frameIndex);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private DataOutputStream output;

    public ImageExtractor(File file, final String weight, int frameIndex) {
        try {
            output = new DataOutputStream(new BufferedOutputStream(new FileOutputStream(file.getPath()
                    + "." + weight)));
            FileParser parser = FileParser.parseFormat(file);
            FileParser.FileHead head = parser.getHead();
            if (parser instanceof ARGBFileParser) {
                ((ARGBFileParser) parser).fillToBuffer(head, frameIndex, new FileParser.SafeColorSetter() {
                    @Override
                    public void onColor(int x, int y, int w, int h, int alpha, int r, int g, int b) {
                        try {
                            if ("a".equals(weight)) {
                                output.writeByte(alpha);
                            } else if ("r".equals(weight)) {
                                output.writeByte(r);
                            } else if ("g".equals(weight)) {
                                output.writeByte(g);
                            } else if ("b".equals(weight)) {
                                output.writeByte(b);
                            } else if ("grey".equals(weight)) {
                                output.writeByte(ARGBFileParser.convertArgbToGrey(alpha, r, g, b));
                            } else {
                                throw new IllegalArgumentException("Unknown weight parameter!");
                            }
                        } catch (IOException e) {
                            throw new IllegalStateException(e);
                        }
                    }
                });
            } else if (parser instanceof YCbCrFileParser) {
                ((YCbCrFileParser) parser).fillToBufferRaw(head, frameIndex, new FileParser.SafeColorSetter() {
                    @Override
                    public void onColor(int x, int y, int w, int h, int alpha, int Y, int Cb, int Cr) {
                        try {
                            if ("y".equals(weight)) {
                                output.writeByte(Y);
                            } else if ("u".equals(weight)) {
                                output.writeByte(Cb);
                            } else if ("v".equals(weight)) {
                                output.writeByte(Cr);
                            } else {
                                throw new IllegalStateException("Unknown weight parameter!");
                            }
                        } catch (IOException e) {
                            throw new IllegalStateException(e);
                        }
                    }
                });
            } else {
                System.err.println("Can't extract any thing!");
            }
        } catch (Exception e) {
            throw new IllegalStateException(e);
        } finally {
            if (output != null) {
                try {
                    output.close();
                } catch (IOException ignored) {
                    // do nothing
                }
            }
        }
    }
}
