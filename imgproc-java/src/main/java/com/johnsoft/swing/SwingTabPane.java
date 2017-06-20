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

import javax.swing.JComponent;
import javax.swing.JScrollPane;
import javax.swing.JTabbedPane;
import javax.swing.JViewport;
import javax.swing.SwingConstants;

import com.johnsoft.SwingUiFace;
import com.johnsoft.UiFace;


/**
 * @author John Kenrinus Lee
 * @version 2017-06-03
 */
public class SwingTabPane implements UiFace.TabPane {
    private final TabbedPane tabbedPane;
    private final SwingUiFace uiface;

    public SwingTabPane(SwingUiFace uiFace) {
        tabbedPane = new TabbedPane();
        uiface = uiFace;
    }

    public UiFace.TabPane addTab(String title, UiFace.Ui ui) {
        JScrollPane scrollPane = new JScrollPane((JComponent) ui.getPeer());
//        scrollPane.getVerticalScrollBar().setUnitIncrement(10);
//        scrollPane.getHorizontalScrollBar().setUnitIncrement(10);
        scrollPane.setWheelScrollingEnabled(true);
        scrollPane.getViewport().setScrollMode(JViewport.BLIT_SCROLL_MODE);
        tabbedPane.putClientProperty(title, ui.getClientData());
        tabbedPane.addTab(title, scrollPane);
        return this;
    }

    @Override
    public Object getActiveTabData() {
        return tabbedPane.getClientProperty(tabbedPane.getTitleAt(tabbedPane.getSelectedIndex()));
    }

    @Override
    public UiFace.Peer getPeer() {
        return tabbedPane;
    }

    @Override
    public Object getClientData() {
        return null;
    }

    private static final class TabbedPane extends JTabbedPane implements UiFace.Peer {
        TabbedPane() {
            super(SwingConstants.TOP, JTabbedPane.SCROLL_TAB_LAYOUT);
        }
    }
}
