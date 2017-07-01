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
import java.awt.Component;
import java.awt.ComponentOrientation;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.util.LinkedList;
import java.util.Random;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JFrame;
import javax.swing.JPanel;

/**
 * @author John Kenrinus Lee
 * @version 2017-06-29
 */
public class BoxAssembler {
    private final Container container;
    private final LinkedList<Component> listLeaf = new LinkedList<>();
    private final LinkedList<Component> listBranch = new LinkedList<>();

    private Container wrapper(Component component) {
        Container container = new Container();
        container.setLayout(new FlowLayout());
        container.add(component);
        return container;
    }

    public BoxAssembler(Container container, int axis, ComponentOrientation o) {
        this.container = container;
        container.setLayout(new BoxLayout(container, axis));
        container.setComponentOrientation(o);
    }

    public BoxAssembler appendHGlue() {
        listLeaf.addLast(Box.createHorizontalGlue());
        return this;
    }

    public BoxAssembler appendVGlue() {
        listLeaf.addLast(Box.createVerticalGlue());
        return this;
    }

    public BoxAssembler prependHGlue() {
        listLeaf.addFirst(Box.createHorizontalGlue());
        return this;
    }

    public BoxAssembler prependVGlue() {
        listLeaf.addFirst(Box.createVerticalGlue());
        return this;
    }

    public BoxAssembler appendHStrut(int width) {
        listLeaf.addLast(Box.createHorizontalStrut(width));
        return this;
    }

    public BoxAssembler appendVStrut(int height) {
        listLeaf.addLast(Box.createVerticalStrut(height));
        return this;
    }

    public BoxAssembler prependHStrut(int width) {
        listLeaf.addFirst(Box.createHorizontalStrut(width));
        return this;
    }

    public BoxAssembler prependVStrut(int height) {
        listLeaf.addFirst(Box.createVerticalStrut(height));
        return this;
    }

    public BoxAssembler apendGlue() {
        listLeaf.addLast(Box.createGlue());
        return this;
    }

    public BoxAssembler prependGlue() {
        listLeaf.addFirst(Box.createGlue());
        return this;
    }

    public BoxAssembler apendRigidArea(int width, int height) {
        listLeaf.addLast(Box.createRigidArea(new Dimension(width, height)));
        return this;
    }

    public BoxAssembler prependRigidArea(int width, int height) {
        listLeaf.addFirst(Box.createRigidArea(new Dimension(width, height)));
        return this;
    }

    public BoxAssembler append(Component...components) {
        for (Component component : components) {
            listLeaf.addLast(component);
        }
        return this;
    }

    public BoxAssembler prepend(Component...components) {
        for (Component component : components) {
            listLeaf.addFirst(component);
        }
        return this;
    }

    public BoxAssembler appendWithWrapper(Component...components) {
        for (Component component : components) {
            listLeaf.addLast(wrapper(component));
        }
        return this;
    }

    public BoxAssembler prependWithWrapper(Component...components) {
        for (Component component : components) {
            listLeaf.addFirst(wrapper(component));
        }
        return this;
    }

    public BoxAssembler popAllToHBox() {
        Box box = Box.createHorizontalBox();
        for (Component component : listLeaf) {
            box.add(component);
        }
        listLeaf.clear();
        listLeaf.add(box);
        return this;
    }

    public BoxAssembler popAllToVBox() {
        Box box = Box.createVerticalBox();
        for (Component component : listLeaf) {
            box.add(component);
        }
        listLeaf.clear();
        listLeaf.add(box);
        return this;
    }

    public BoxAssembler storeToFirst() {
        for (Component component : listLeaf) {
            listBranch.addFirst(component);
        }
        listLeaf.clear();
        return this;
    }

    public BoxAssembler storeToLast() {
        for (Component component : listLeaf) {
            listBranch.addLast(component);
        }
        listLeaf.clear();
        return this;
    }

    public BoxAssembler clearAndLoad() {
        listLeaf.clear();
        return load();
    }

    public BoxAssembler load() {
        listLeaf.addAll(listBranch);
        listBranch.clear();
        return this;
    }

    public void assemble() {
        for (Component component : listLeaf) {
            container.add(component);
        }
    }

    public static void main(String[] args) {
        JFrame frame = new JFrame("Test");
        JPanel panel = new JPanel();
        panel.setBackground(Color.BLACK);

        BoxAssembler ba = new BoxAssembler(panel, BoxLayout.X_AXIS, ComponentOrientation.LEFT_TO_RIGHT);

        ba.append(randomColorBlock(20, 20), randomColorBlock(20, 20)).popAllToHBox()
                .prepend(randomColorBlock(40, 40)).popAllToVBox()
                .prepend(randomColorBlock(200, 80)).prepend(randomColorBlock(100, 80)).prependHGlue()
                .popAllToHBox().storeToLast();

        ba.append(randomColorBlock(100, 80), randomColorBlock(80, 80)).popAllToHBox()
                .append(randomColorBlock(100, 100)).appendVGlue().popAllToVBox()
                .appendHStrut(40).append(randomColorBlock(200, 200)).popAllToHBox()
                .storeToLast();

        ba.load().popAllToVBox().storeToLast();

        ba.append(randomColorBlock(30, 30), randomColorBlock(30, 30)).popAllToHBox()
                .prepend(randomColorBlock(60, 300)).popAllToVBox().storeToFirst();

        ba.load().popAllToHBox().prepend(randomColorBlock(400, 50)).prependVGlue().popAllToVBox();

        ba.assemble();

        frame.setContentPane(panel);
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setSize(800, 600);
        frame.setLocationRelativeTo(null);
        frame.setVisible(true);
    }

    private static final Random rand = new Random();

    private static JPanel randomColorBlock(int w, int h) {
        JPanel panel = new JPanel();
        panel.setBackground(new Color(rand.nextInt(255), rand.nextInt(255), rand.nextInt(255)));
        panel.setPreferredSize(new Dimension(w, h));
        return panel;
    }
}
