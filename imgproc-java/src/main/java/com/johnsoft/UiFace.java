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
package com.johnsoft;


import java.io.File;
import java.util.List;

/**
 * @author John Kenrinus Lee
 * @version 2017-06-03
 */
public interface UiFace {
    MainWindow getMainWindow();
    TabPane getTabPane();
    MenuManager getMenuManager();
    Action getAction(String command);
    void onReceiveFiles(List<File> files);

    interface Peer {
    }

    interface Control extends Peer {
        void setEnabled(boolean enabled);
        void setCheckedOrSelected(boolean checkedOrSelected);
        void setFocused(boolean focused);
        boolean isEnabled();
        boolean isCheckedOrSelected();
        boolean isFocused();
    }

    interface Ui {
        Peer getPeer();
        Object getClientData();
    }

    interface MainWindow extends Ui {
        MainWindow setContent(TabPane tabPane);
        MainWindow setMenu(MenuManager menuManager);
        MainWindow show(String title, int w, int h);
    }

    interface TabPane extends Ui {
        TabPane addTab(String title, Ui ui);
        Object getActiveTabData();
    }

    interface MenuManager extends Ui {
        MenuManager addMenus(String...menus);
        MenuManager addMenuItem(String targetMenu, String itemLabel, Action action);
        MenuManager addCheckedMenuItem(String targetMenu, String itemLabel, boolean checked, Action action);
        MenuManager addSubMenu(String targetMenu, String itemLabel);
        MenuManager assemble();
        MenuManager setEnabled(String menuPath, boolean enabled);
    }

    interface ImageView extends Ui {
        int[] getRGB(int startX, int startY, int w, int h,
                            int[] rgbArray, int offset, int scansize);

        void setRGB(int startX, int startY, int w, int h,
                           int[] rgbArray, int offset, int scansize);
    }

    interface Action {
        void action(UiFace uiFace, Control control);
    }
}
