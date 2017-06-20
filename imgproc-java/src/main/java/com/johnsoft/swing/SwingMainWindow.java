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

import java.awt.KeyEventDispatcher;
import java.awt.MenuBar;
import java.awt.Toolkit;
import java.awt.datatransfer.Clipboard;
import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.Transferable;
import java.awt.event.KeyEvent;
import java.io.File;
import java.util.List;

import javax.swing.JFrame;
import javax.swing.JTabbedPane;
import javax.swing.TransferHandler;
import javax.swing.WindowConstants;

import com.johnsoft.DebugUtils;
import com.johnsoft.SwingUiFace;
import com.johnsoft.UiFace;

/**
 * @author John Kenrinus Lee
 * @version 2017-06-03
 */
public class SwingMainWindow implements UiFace.MainWindow {
    private final MainFrame frame;
    private final SwingUiFace uiface;

    public SwingMainWindow(SwingUiFace uiFace) {
        frame = new MainFrame();
        uiface = uiFace;
        DebugUtils.hookSwingKeyEvent(new KeyEventDispatcher() {
            @Override
            public boolean dispatchKeyEvent(KeyEvent event) {
                if (event.getID() == KeyEvent.KEY_RELEASED) {
                    if (event.isControlDown() && event.getKeyCode() == KeyEvent.VK_V) {
                        doPasteAction();
                    }
                }
                return false;
            }
            private void doPasteAction() {
                final Clipboard clipboard = Toolkit.getDefaultToolkit().getSystemClipboard();
                final Transferable trans = clipboard.getContents(null);
                if (trans != null) {
                    if (trans.isDataFlavorSupported(DataFlavor.javaFileListFlavor)) {
                        try {
                            final List<File> files = (List<File>)trans.getTransferData(DataFlavor
                                    .javaFileListFlavor);
                            uiface.onReceiveFiles(files);
                        } catch (Exception ex) {
                            ex.printStackTrace();
                        }
                    }
                }
            }
        });
        frame.setTransferHandler(new TransferHandler(null) {
            @Override
            public boolean importData(TransferSupport support) {
                if (!canImport(support)) {
                    return false;
                }
                final Transferable t = support.getTransferable();
                try {
                    final List<File> files = (List<File>) t.getTransferData(DataFlavor
                            .javaFileListFlavor);
                    uiface.onReceiveFiles(files);
                    return true;
                } catch (Exception e) {
                    e.printStackTrace();
                    return false;
                }
            }

            @Override
            public boolean canImport(TransferSupport support) {
                if (!support.isDrop()) {
                    return false;
                }
                if (support.isDataFlavorSupported(DataFlavor.javaFileListFlavor)) {
                    return true;
                }
                return false;
            }
        });
    }

    public UiFace.MainWindow setContent(UiFace.TabPane tabPane) {
        frame.setContentPane((JTabbedPane) tabPane.getPeer());
        return this;
    }

    public UiFace.MainWindow setMenu(UiFace.MenuManager menuManager) {
        frame.setMenuBar((MenuBar) menuManager.getPeer());
        return this;
    }

    public UiFace.MainWindow show(String title, int w, int h) {
        frame.setTitle(title);
        frame.setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
        frame.setSize(w, h);
        frame.setLocationRelativeTo(null);
        frame.setVisible(true);
        return this;
    }

    @Override
    public UiFace.Peer getPeer() {
        return frame;
    }

    @Override
    public Object getClientData() {
        return null;
    }

    private static final class MainFrame extends JFrame implements UiFace.Peer {
    }
}