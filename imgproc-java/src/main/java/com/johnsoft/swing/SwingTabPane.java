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

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import java.util.concurrent.atomic.AtomicInteger;

import javax.imageio.ImageIO;
import javax.swing.Icon;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.JTabbedPane;
import javax.swing.JViewport;
import javax.swing.SwingConstants;

import com.johnsoft.Log;
import com.johnsoft.SwingUiFace;
import com.johnsoft.ToolUtilities;
import com.johnsoft.UiFace;


/**
 * @author John Kenrinus Lee
 * @version 2017-06-03
 */
public class SwingTabPane implements UiFace.TabPane {
    private final AtomicInteger intGenerator = new AtomicInteger(1);

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
        tabbedPane.setTabComponentAt(tabbedPane.getTabCount() - 1, tabComponent(title, null, title));
        return this;
    }

    @Override
    public UiFace.TabPane closeTab(String title) {
        final int count = tabbedPane.getTabCount();
        for (int i = count - 1; i >= 0; --i) {
            if (title.equals(tabbedPane.getTitleAt(i))) {
                tabbedPane.removeTabAt(i);
                break;
            }
        }
        return this;
    }

    @Override
    public UiFace.TabPane addTabWithExistTitle(String title, UiFace.Ui ui) {
        final File file = ToolUtilities.validImagePath(title);
        if (file == null) {
            DialogUtilities.showInfoMessageBox("Unknow file name or path!");
            return this;
        }
        final int dotIdx = title.lastIndexOf('.');
        final String newTitle = title.substring(0, dotIdx) + '_'
                + intGenerator.getAndIncrement() + title.substring(dotIdx);
        addTab(newTitle, ui);
        return this;
    }

    @Override
    public UiFace.TabPane closeTabWithRegexTitle(String titleRegex) {
        final int count = tabbedPane.getTabCount();
        for (int i = count - 1; i >= 0; --i) {
            if (tabbedPane.getTitleAt(i).matches(titleRegex)) {
                tabbedPane.removeTabAt(i);
                break;
            }
        }
        return this;
    }

    public void closeAllTabs() {
        final int count = tabbedPane.getTabCount();
        for (int i = count - 1; i >= 0; --i) {
            tabbedPane.removeTabAt(i);
        }
    }

    @Override
    public Object getActiveTabData() {
        final int selectIndex = tabbedPane.getSelectedIndex();
        if (!checkIndex(selectIndex)) {
            return null;
        }
        return tabbedPane.getClientProperty(tabbedPane.getTitleAt(selectIndex));
    }

    @Override
    public String getActiveTabIdentifier() {
        final int selectIndex = tabbedPane.getSelectedIndex();
        if (!checkIndex(selectIndex)) {
            return null;
        }
        return tabbedPane.getTitleAt(selectIndex);
    }

    private boolean checkIndex(int selectIndex) {
        if (selectIndex < 0) {
            if (tabbedPane.isShowing()) {
                DialogUtilities.showInfoMessageBox("Show select one tab first!");
            } else {
                Log.w("SwingTabPane", "Show select one tab first!");
            }
            return false;
        }
        return true;
    }

    @Override
    public UiFace.Peer getPeer() {
        return tabbedPane;
    }

    @Override
    public Object getClientData() {
        return null;
    }

    private JComponent tabComponent(final String title, Icon icon, String tip) {
        JPanel panel = new JPanel(new BorderLayout());
        panel.setOpaque(false);
        panel.addMouseListener(new MouseAdapter() {
            @Override
            public void mouseClicked(MouseEvent e) {
                if (e.getClickCount() == 1) {
                    if (e.getButton() == MouseEvent.BUTTON1) {
                        final int count = tabbedPane.getTabCount();
                        for (int i = 0; i < count; ++i) {
                            if (title.equals(tabbedPane.getTitleAt(i))) {
                                tabbedPane.setSelectedIndex(i);
                                break;
                            }
                        }
                    } else if (e.getButton() == MouseEvent.BUTTON3) {
                        JMenuItem closeCurrentTab = new JMenuItem("Close Current Tab");
                        closeCurrentTab.addActionListener(new ActionListener() {
                            @Override
                            public void actionPerformed(ActionEvent e) {
                                closeTab(title);
                            }
                        });
                        JMenuItem closeOtherTabs = new JMenuItem("Close Other Tabs");
                        closeOtherTabs.addActionListener(new ActionListener() {
                            @Override
                            public void actionPerformed(ActionEvent e) {
                                final int count = tabbedPane.getTabCount();
                                for (int i = count - 1; i >= 0; --i) {
                                    if (!title.equals(tabbedPane.getTitleAt(i))) {
                                        tabbedPane.removeTabAt(i);
                                    }
                                }
                            }
                        });
                        JMenuItem closeAllTabs = new JMenuItem("Close All Tabs");
                        closeAllTabs.addActionListener(new ActionListener() {
                            @Override
                            public void actionPerformed(ActionEvent e) {
                                closeAllTabs();
                            }
                        });
                        JPopupMenu popupMenu = new JPopupMenu();
                        popupMenu.add(closeCurrentTab);
                        popupMenu.add(closeOtherTabs);
                        popupMenu.add(closeAllTabs);
                        popupMenu.show(e.getComponent(), e.getX(), e.getY());
                    }
                }
            }
        });
        panel.setToolTipText(tip);
        File file = ToolUtilities.validImagePath(title);
        if (file != null) {
            panel.add(new JLabel(file.getName()));
        } else {
            panel.add(new JLabel(title));
        }
        if (icon != null) {
            panel.add(new JLabel(icon), BorderLayout.WEST);
        }
        JImageView xClose = new JImageView();
        xClose.setImagePaintInfo(xCloseIconInfo);
        xClose.setPreferredSize(new Dimension(xCloseIconInfo.dstW, xCloseIconInfo.dstH));
        xClose.addMouseListener(new MouseAdapter() {
            @Override
            public void mouseClicked(MouseEvent e) {
                if (e.getClickCount() == 1 && e.getButton() == MouseEvent.BUTTON1) {
                    closeTab(title);
                }
            }
        });
        panel.add(xClose, BorderLayout.EAST);
        return panel;
    }

    private static final JImageView.ImagePaintInfo xCloseIconInfo = getCloseIcon();

    private static JImageView.ImagePaintInfo getCloseIcon() {
        final BufferedImage image;
        try {
            image = ImageIO.read(SwingTabPane.class.getResource("/close_button.png"));
        } catch (IOException e) {
            throw new IllegalStateException(e);
        }
        JImageView.ImagePaintInfo info = new JImageView.ImagePaintInfo();
        info.image = image;
        info.scaleType = JImageView.ImagePaintInfo.TYPE_RATIO_CENTER;
        info.usePreferredSize = false;
        info.dstW = 12;
        info.dstH = 12;
        return info;
    }

    private static final class TabbedPane extends JTabbedPane implements UiFace.Peer {
        TabbedPane() {
            super(SwingConstants.TOP, JTabbedPane.SCROLL_TAB_LAYOUT);
        }
    }
}
