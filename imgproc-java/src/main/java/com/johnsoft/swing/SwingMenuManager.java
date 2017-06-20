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

import java.awt.CheckboxMenuItem;
import java.awt.Menu;
import java.awt.MenuBar;
import java.awt.MenuItem;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.util.HashMap;

import com.johnsoft.SwingUiFace;
import com.johnsoft.UiFace;

/**
 * @author John Kenrinus Lee
 * @version 2017-06-03
 */
public class SwingMenuManager implements UiFace.MenuManager {
    private final SwingUiFace uiface;
    private final MainMenuBar menuBar;
    private final HashMap<String, Menu> menuMap = new HashMap<String, Menu>();

    public SwingMenuManager(SwingUiFace uiFace) {
        uiface = uiFace;
        menuBar = new MainMenuBar();
    }

    public UiFace.MenuManager addMenus(String... menus) {
        for (String menuLabel : menus) {
            Menu menu = new Menu(menuLabel);
            menuBar.add(menu);
            menuMap.put(menuLabel, menu);
        }
        return this;
    }

    public UiFace.MenuManager addMenuItem(String targetMenu, String itemLabel, final UiFace.Action action) {
        Menu menu = menuMap.get(targetMenu);
        MenuItemPeer item = new MenuItemPeer(itemLabel);
        item.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (action != null) {
                    action.action(SwingUiFace.getDefault(), (UiFace.Control) e.getSource());
                }
            }
        });
        menu.add(item);
        return this;
    }

    public UiFace.MenuManager addCheckedMenuItem(String targetMenu, String itemLabel, boolean checked,
                                                 final UiFace.Action action) {
        Menu menu = menuMap.get(targetMenu);
        CheckedMenuItemPeer item = new CheckedMenuItemPeer(itemLabel, checked);
        item.addItemListener(new ItemListener() {
            public void itemStateChanged(ItemEvent e) {
                if (action != null) {
                    action.action(SwingUiFace.getDefault(), (UiFace.Control) e.getSource());
                }
            }
        });
        menu.add(item);
        return this;
    }

    public UiFace.MenuManager addSubMenu(String targetMenu, String itemLabel) {
        Menu menu = menuMap.get(targetMenu);
        Menu subMenu = new Menu(itemLabel);
        menuMap.put(itemLabel, subMenu);
        menu.add(subMenu);
        return this;
    }

    public UiFace.MenuManager assemble() {
        return this;
    }

    public UiFace.MenuManager setEnabled(final String menuPath, final boolean enabled) {
        return applyOpOnMenuPath(menuPath, new MenuItemOp() {
            public void operate(MenuItem menuItem) {
                menuItem.setEnabled(enabled);
            }
        });
    }

    private UiFace.MenuManager applyOpOnMenuPath(String menuPath, MenuItemOp menuItemOp) {
        if (menuPath == null || menuPath.trim().length() == 0) {
            return this;
        }
        if (menuPath.charAt(0) == '/') {
            menuPath = menuPath.substring(1);
        }
        String[] segments = menuPath.split("/");
        MenuItem target = menuMap.get(segments[0]);
        MenuItem menuItem;
        Menu menu;
        if (target != null) {
            int x = 1;
            while (segments.length > x && target instanceof Menu) {
                menu = (Menu) target;
                boolean found = false;
                for (int i = 0; i < menu.getItemCount(); i++) {
                    menuItem = menu.getItem(i);
                    if (segments[x].equals(menuItem.getLabel())) {
                        target = menuItem;
                        found = true;
                        break;
                    }
                }
                if (found) {
                    ++x;
                } else {
                    target = null;
                    break;
                }
            }
            if (target != null) {
                menuItemOp.operate(target);
            }
        }
        return this;
    }

    @Override
    public UiFace.Peer getPeer() {
        return menuBar;
    }

    @Override
    public Object getClientData() {
        return null;
    }

    private static final class MainMenuBar extends MenuBar implements UiFace.Peer {
    }

    private static final class MenuItemPeer extends MenuItem implements UiFace.Control {
        MenuItemPeer(String label) {
            super(label);
        }

        public void setCheckedOrSelected(boolean checkedOrSelected) {
        }

        public void setFocused(boolean focused) {
        }

        public boolean isCheckedOrSelected() {
            return false;
        }

        public boolean isFocused() {
            return false;
        }
    }

    private static final class CheckedMenuItemPeer extends CheckboxMenuItem implements UiFace.Control {
        CheckedMenuItemPeer(String label, boolean checked) {
            super(label, checked);
        }

        public void setCheckedOrSelected(boolean checkedOrSelected) {
            setState(checkedOrSelected);
        }

        public boolean isCheckedOrSelected() {
            return getState();
        }

        public boolean isFocused() {
            return false;
        }

        public void setFocused(boolean focused) {
        }
    }

    private interface MenuItemOp {
        void operate(MenuItem menuItem);
    }
}
