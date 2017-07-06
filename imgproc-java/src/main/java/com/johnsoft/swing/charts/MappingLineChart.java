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

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.plot.PlotOrientation;
import org.jfree.chart.plot.XYPlot;
import org.jfree.data.xy.DefaultXYDataset;

import com.johnsoft.alg.BaseImageProc;

/**
 * @author John Kenrinus Lee
 * @version 2017-07-06
 */
public class MappingLineChart {
    private static final double[] normalValueArray = createNormalValueArray();
    private static final double[][] baseArray = new double[][] { normalValueArray, normalValueArray };

    private static double[] createNormalValueArray() {
        final double[] arr = new double[256];
        final int len = arr.length;
        for (int i = 0; i < len; ++i) {
            arr[i] = i / 255.0;
        }
        return arr;
    }

    public static ChartPanel newView() {
        final double[][] data = new double[2][256];
        data[0] = normalValueArray;
        data[1] = normalValueArray;
        final DefaultXYDataset dataset = new DefaultXYDataset();
        dataset.addSeries("Base", baseArray);
        dataset.addSeries("Result", data);
        final JFreeChart chart = ChartFactory.createXYLineChart(null,
                null, null,
                dataset, PlotOrientation.VERTICAL, false, true, false);
        final XYPlot plot = chart.getXYPlot();
        plot.getDomainAxis().setUpperBound(1);
        plot.getDomainAxis().setLowerBound(0);
        plot.getRangeAxis().setUpperBound(1);
        plot.getRangeAxis().setLowerBound(0);
        final ChartPanel panel = new ChartPanel(chart, false, true,
                false, true, true);
        panel.putClientProperty("data", data);
        return panel;
    }

    /** If call, then should call after newView() called first */
    public static void updateData(ChartPanel panel) {
        final double[][] data = (double[][]) panel.getClientProperty("data");
        data[1] = BaseImageProc.calcGreyFilter(data[0]);
        final JFreeChart chart = panel.getChart();
        final XYPlot plot = chart.getXYPlot();
        final DefaultXYDataset dataset = (DefaultXYDataset) plot.getDataset();
        dataset.removeSeries("Result");
        dataset.addSeries("Result", data);
    }
}
