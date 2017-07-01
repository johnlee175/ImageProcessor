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

import java.awt.Component;
import java.awt.ComponentOrientation;
import java.awt.Container;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;

import javax.swing.BorderFactory;
import javax.swing.JComponent;

/**
 * @author John Kenrinus Lee
 * @version 2017-06-29
 */
public class GridBagAssembler {
    public static final int GRID_RELATIVE = -1;
    public static final int GRID_REMAINDER = 0;
    public static final int FILL_NONE = 0;
    public static final int FILL_BOTH = 1;
    public static final int FILL_HORIZONTAL = 2;
    public static final int FILL_VERTICAL = 3;
    public static final int ANCHOR_CENTER = 10;
    public static final int ANCHOR_NORTH = 11;
    public static final int ANCHOR_NORTHEAST = 12;
    public static final int ANCHOR_EAST = 13;
    public static final int ANCHOR_SOUTHEAST = 14;
    public static final int ANCHOR_SOUTH = 15;
    public static final int ANCHOR_SOUTHWEST = 16;
    public static final int ANCHOR_WEST = 17;
    public static final int ANCHOR_NORTHWEST = 18;
    public static final int ANCHOR_PAGE_START = 19;
    public static final int ANCHOR_PAGE_END = 20;
    public static final int ANCHOR_LINE_START = 21;
    public static final int ANCHOR_LINE_END = 22;
    public static final int ANCHOR_FIRST_LINE_START = 23;
    public static final int ANCHOR_FIRST_LINE_END = 24;
    public static final int ANCHOR_LAST_LINE_START = 25;
    public static final int ANCHOR_LAST_LINE_END = 26;
    public static final int ANCHOR_BASELINE = 0x100;
    public static final int ANCHOR_BASELINE_LEADING = 0x200;
    public static final int ANCHOR_BASELINE_TRAILING = 0x300;
    public static final int ANCHOR_ABOVE_BASELINE = 0x400;
    public static final int ANCHOR_ABOVE_BASELINE_LEADING = 0x500;
    public static final int ANCHOR_ABOVE_BASELINE_TRAILING = 0x600;
    public static final int ANCHOR_BELOW_BASELINE = 0x700;
    public static final int ANCHOR_BELOW_BASELINE_LEADING = 0x800;
    public static final int ANCHOR_BELOW_BASELINE_TRAILING = 0x900;

    private static final Insets ZERO_INSETS = new Insets(0, 0, 0, 0);

    private final Container container;
    private final GridBagConstraints constraints = new GridBagConstraints();

    /**
     * @param container 要安装GridBagLayout布局的面板
     * @param o         面板中组件的布局方向
     */
    public GridBagAssembler(Container container, ComponentOrientation o) {
        this.container = container;
        container.setLayout(new GridBagLayout());
        container.setComponentOrientation(o);
    }

    /**
     * gridx+1
     */
    public GridBagAssembler nextColumn() {
        constraints.gridx = constraints.gridx + 1;
        return this;
    }

    /**
     * gridy+1
     */
    public GridBagAssembler nextRow() {
        constraints.gridy = constraints.gridy + 1;
        return this;
    }

    /**
     * gridy+1, gridx=0
     */
    public GridBagAssembler wrapLine() {
        constraints.gridy = constraints.gridy + 1;
        constraints.gridx = 0;
        return this;
    }

    /**
     * 设置container的padding, 前提是container是JComponent的子类
     */
    public GridBagAssembler padding(int top, int left, int bottom, int right) {
        if (container instanceof JComponent) {
            JComponent component = (JComponent) container;
            component.setBorder(BorderFactory.createEmptyBorder(top, left, bottom, right));
        }
        return this;
    }

    /**
     * 添加完组件后可以继续使用, swing会clone使用而不是引用内部的constraints
     */
    public GridBagAssembler add(Component comp) {
        if (comp != null) {
            container.add(comp, constraints);
        }
        return this;
    }

    /**
     * @return 返回一个内部constraints的clone
     */
    public GridBagConstraints getConstraints() {
        return (GridBagConstraints) constraints.clone();
    }

    public GridBagAssembler bound(int gridx, int gridy, int gridwidth, int gridheight) {
        constraints.gridx = gridx;
        constraints.gridy = gridy;
        constraints.gridwidth = gridwidth;
        constraints.gridheight = gridheight;
        return this;
    }

    public GridBagAssembler position(int gridx, int gridy) {
        constraints.gridx = gridx;
        constraints.gridy = gridy;
        return this;
    }

    public GridBagAssembler size(int gridwidth, int gridheight) {
        constraints.gridwidth = gridwidth;
        constraints.gridheight = gridheight;
        return this;
    }

    public GridBagAssembler weight(double weightx, double weighty) {
        constraints.weightx = weightx;
        constraints.weighty = weighty;
        return this;
    }

    public GridBagAssembler ipad(int ipadx, int ipady) {
        constraints.ipadx = ipadx;
        constraints.ipady = ipady;
        return this;
    }

    public GridBagAssembler fill(int fill) {
        constraints.fill = fill;
        return this;
    }

    public GridBagAssembler anchor(int anchor) {
        constraints.anchor = anchor;
        return this;
    }

    public GridBagAssembler insets(int top, int left, int bottom, int right) {
        constraints.insets = new Insets(top, left, bottom, right);
        return this;
    }

    public GridBagAssembler zeroInsets() {
        constraints.insets = ZERO_INSETS;
        return this;
    }

    public GridBagAssembler ipadx(int ipadx) {
        constraints.ipadx = ipadx;
        return this;
    }

    public GridBagAssembler ipady(int ipady) {
        constraints.ipady = ipady;
        return this;
    }

    public GridBagAssembler weighty(double weighty) {
        constraints.weighty = weighty;
        return this;
    }

    public GridBagAssembler weightx(double weightx) {
        constraints.weightx = weightx;
        return this;
    }

    public GridBagAssembler gridx(int gridx) {
        constraints.gridx = gridx;
        return this;
    }

    public GridBagAssembler gridy(int gridy) {
        constraints.gridy = gridy;
        return this;
    }

    public GridBagAssembler gridwidth(int gridwidth) {
        constraints.gridwidth = gridwidth;
        return this;
    }

    public GridBagAssembler gridheight(int gridheight) {
        constraints.gridheight = gridheight;
        return this;
    }
}
