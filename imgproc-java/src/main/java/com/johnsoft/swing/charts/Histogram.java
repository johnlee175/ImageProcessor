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
package com.johnsoft.swing.charts;

import java.awt.Color;
import java.awt.ComponentOrientation;
import java.awt.Dimension;
import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.geom.RectangularShape;
import java.util.HashMap;
import java.util.Map;

import javax.swing.JComponent;
import javax.swing.JPanel;
import javax.swing.JScrollPane;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.plot.CategoryPlot;
import org.jfree.chart.plot.PlotOrientation;
import org.jfree.chart.renderer.category.BarPainter;
import org.jfree.chart.renderer.category.BarRenderer;
import org.jfree.chart.renderer.category.StandardBarPainter;
import org.jfree.data.category.CategoryDataset;
import org.jfree.data.category.DefaultCategoryDataset;
import org.jfree.ui.RectangleEdge;

import com.johnsoft.swing.GridBagAssembler;

/**
 * @author John Kenrinus Lee
 * @version 2017-07-06
 */
public class Histogram {
    public static JComponent newAllTypeViews(int[] data) {
        final JPanel panel = new JPanel();
        final GridBagAssembler assembler = new GridBagAssembler(panel, ComponentOrientation.LEFT_TO_RIGHT);
        assembler.fill(GridBagAssembler.FILL_NONE).insets(5, 5, 5, 5);
        assembler.wrapLine().add(newView(data, Color.GRAY, false))
                .nextColumn().add(newView(data, Color.RED, false))
                .wrapLine().add(newView(data, Color.GREEN, false))
                .nextColumn().add(newView(data, Color.BLUE, false))
                .wrapLine().add(newView(data, Color.GRAY, true))
                .nextColumn().add(newView(data, Color.RED, true))
                .wrapLine().add(newView(data, Color.GREEN, true))
                .nextColumn().add(newView(data, Color.BLUE, true));
        final JScrollPane scrollPane = new JScrollPane(panel);
        scrollPane.getVerticalScrollBar().setUnitIncrement(4);
        scrollPane.getHorizontalScrollBar().setUnitIncrement(4);
        return scrollPane;
    }

    public static JComponent newView(int[] data, Color color, boolean useColorBar) {
        final HashMap<Integer, Integer> map = new HashMap<>();
        makeData(map, data, color);
        final JFreeChart chart = makeChart(makeCategoryDataset(map), useColorBar, color);
        final ChartPanel panel = new ChartPanel(chart, false, true,
                false, true, true);
        panel.setPreferredSize(new Dimension(480, 320));
        return panel;
    }

    private static JFreeChart makeChart(CategoryDataset dataset, boolean useColorBar, final Color color) {
        final JFreeChart chart = ChartFactory.createBarChart(null,
                "Color", "Count",
                dataset, PlotOrientation.VERTICAL, false, true, false);
        chart.setNotify(false);
        final CategoryPlot plot = (CategoryPlot) chart.getPlot();
        plot.setNotify(false);
        plot.getDomainAxis().setVisible(false);
        plot.getRangeAxis().setLabel(null);
        plot.setRangeGridlinesVisible(true);

        final BarRenderer renderer = ((BarRenderer) plot.getRenderer());
        if (useColorBar) {
            makeColorBar(color, plot, renderer);
        } else {
            makeStandardBar(color, plot, renderer);
        }

        return chart;
    }

    private static void makeColorBar(final Color color, CategoryPlot plot, BarRenderer renderer) {
        plot.setBackgroundPaint(Color.YELLOW.darker());
        plot.setRangeGridlinePaint(Color.MAGENTA.brighter());
        renderer.setBarPainter(new BarPainter() {
            @Override
            public void paintBar(Graphics2D g2d, BarRenderer renderer,
                                 int row, int column, RectangularShape bar, RectangleEdge edge) {
                if (Color.RED.equals(color)) {
                    g2d.setPaint(new Color(column, 0, 0));
                } else if (Color.GREEN.equals(color)) {
                    g2d.setPaint(new Color(0, column, 0));
                } else if (Color.BLUE.equals(color)) {
                    g2d.setPaint(new Color(0, 0, column));
                } else if (Color.GRAY.equals(color)) {
                    g2d.setPaint(new Color(column, column, column));
                } else {
                    throw new IllegalArgumentException("Unknown color!");
                }
                final Rectangle rect = bar.getBounds();
                g2d.fillRect(rect.x, rect.y, rect.width, rect.height);
            }
            @Override
            public void paintBarShadow(Graphics2D graphics2D, BarRenderer barRenderer, int i1, int i2,
                                       RectangularShape rectangularShape, RectangleEdge rectangleEdge, boolean b) {
            }
        });
    }

    private static void makeStandardBar(Color color, CategoryPlot plot, BarRenderer renderer) {
        plot.setBackgroundPaint(Color.WHITE);
        plot.setRangeGridlinePaint(Color.BLACK);
        renderer.setBarPainter(new StandardBarPainter());
        renderer.setSeriesPaint(0, color, false);
    }

    private static CategoryDataset makeCategoryDataset(Map<Integer, Integer> map) {
        final DefaultCategoryDataset dataset = new DefaultCategoryDataset();
        dataset.setNotify(false);
        for (int i = 0; i < 256; ++i) {
            dataset.addValue((Number) (map.containsKey(i) ? map.get(i) : 0), "Color", i);
        }
        return dataset;
    }

    private static int makeData(Map<Integer, Integer> map, int[] data, Color color) {
        int max = 0;
        for (int i = 0; i < data.length; ++i) {
            final int value = extract(data[i], color);
            final int count;
            if (map.containsKey(value)) {
                count = map.get(value) + 1;
            } else {
                count = 1;
            }
            map.put(value, count);
            max = max < count ? count : max;
        }
        final int size = map.size();
        System.out.println("size: " + size + ", max: " + max);
        return max;
    }

    private static int extract(int data, Color color) {
        if (Color.RED.equals(color)) {
            return (data >> 16) & 0xFF;
        } else if (Color.GREEN.equals(color)) {
            return (data >> 8) & 0xFF;
        } else if (Color.BLUE.equals(color)) {
            return (data >> 0) & 0xFF;
        } else if (Color.GRAY.equals(color)) {
            final int r = (data >> 16) & 0xFF;
            final int g = (data >> 8) & 0xFF;
            final int b = (data >> 0) & 0xFF;
            return (r * 19595 + g * 38469 + b * 7472) >> 16;
        } else {
            throw new IllegalArgumentException("Unknown color!");
        }
    }
}
