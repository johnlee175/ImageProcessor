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

import java.awt.image.BufferedImage;
import java.io.BufferedOutputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Collections;
import java.util.List;

import javax.imageio.ImageIO;

import com.google.devtools.common.options.Option;
import com.google.devtools.common.options.OptionsBase;
import com.google.devtools.common.options.OptionsParser;

/**
 * @author John Kenrinus Lee
 * @version 2017-09-17
 */
public class RGBAWriter {
    public static void write(File src, String format) {
        String name = src.getName();
        if (name.endsWith(".png") || name.endsWith(".jpg")) {
            try {
                BufferedImage image = ImageIO.read(src);
                File target = new File(src.getParent(), name.substring(0, name.length() - 4)
                        + "." + format);
                DataOutputStream output = new DataOutputStream(new BufferedOutputStream(new
                        FileOutputStream(target)));
                for (int i = 0; i < image.getHeight(); ++i) {
                    for (int j = 0; j < image.getWidth(); ++j) {
                        final int color = image.getRGB(j, i);
                        if ("rgb".equals(format)) {
                            output.writeByte((color >> 16) & 0xFF);
                            output.writeByte((color >> 16) & 0xFF);
                            output.writeByte((color >> 16) & 0xFF);
                        } else {
                            output.writeInt(color << 8 | 0xFF);
                        }
                    }
                }
                output.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        } else {
            System.out.println("Skip File: " + src.getAbsolutePath());
        }
    }

    public static class ArgOptions extends OptionsBase {
        @Option(name = "path",
                abbrev = 'p',
                help = "the source image path, default value is ''",
                defaultValue = "",
                allowMultiple = true)
        public List<String> paths;

        @Option(name = "format",
                abbrev = 'f',
                help = "the target image format, options include 'rgba', 'rgb'",
                defaultValue = "rgba")
        public String format;

        public void printUsage(OptionsParser parser) {
            System.out.println("Usage: java -cp imgproc-viewer-1.0.0.jar com.johnsoft.viewer.RGBAWriter OPTIONS");
            System.out.println(parser.describeOptions(Collections.<String, String>emptyMap(),
                    OptionsParser.HelpVerbosity.LONG));
        }
    }

    public static void main(String[] args) {
        OptionsParser parser = OptionsParser.newOptionsParser(ArgOptions.class);
        parser.parseAndExitUponError(args);
        ArgOptions options = parser.getOptions(ArgOptions.class);
        if (options != null) {
            if (options.paths.isEmpty()) {
                System.out.println("Arg Require: please add with --path or -p");
                options.printUsage(parser);
                return;
            }
            if (!"rgba".equals(options.format) && !"rgb".equals(options.format)) {
                System.out.println("Arg Error: --format[-f] only include 'rgba', 'rgb'");
                options.printUsage(parser);
                return;
            }
            for (int i = 0; i < options.paths.size(); i++) {
                String path = options.paths.get(i);
                File file;
                if (path.trim().isEmpty() || !(file = new File(path)).exists()) {
                    System.out.println("Arg Error: --path[-p] should use real file path");
                    options.printUsage(parser);
                    return;
                }
                if (file.isDirectory()) {
                    File[] files = file.listFiles();
                    if (files != null) {
                        for (File subFile : files) {
                            RGBAWriter.write(subFile, options.format);
                        }
                    }
                } else {
                    RGBAWriter.write(file, options.format);
                }
            }
        }
    }
}
