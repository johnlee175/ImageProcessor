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

import java.awt.AWTEvent;
import java.awt.KeyEventDispatcher;
import java.awt.KeyboardFocusManager;
import java.awt.Toolkit;
import java.awt.event.AWTEventListener;
import java.awt.event.KeyEvent;
import java.util.logging.ConsoleHandler;
import java.util.logging.Formatter;
import java.util.logging.Level;
import java.util.logging.LogRecord;
import java.util.logging.Logger;

import sun.util.logging.PlatformLogger;

/**
 * @author John Kenrinus Lee
 * @version 2017-06-10
 */
public class DebugUtils {
    /** debug Component.dispatchEventImpl */
    public static void interceptSwingKeyEvent(KeyEventDispatcher dispatcher) {
        KeyboardFocusManager.getCurrentKeyboardFocusManager()
                .addKeyEventDispatcher(dispatcher);
    }

    /** debug Component.dispatchEventImpl */
    public static void hookSwingKeyEvent(final KeyEventDispatcher dispatcher) {
        Toolkit.getDefaultToolkit().addAWTEventListener(new AWTEventListener() {
            @Override
            public void eventDispatched(final AWTEvent event) {
                if (event instanceof KeyEvent) {
                    dispatcher.dispatchKeyEvent((KeyEvent) event);
                }
            }
        }, AWTEvent.KEY_EVENT_MASK);
    }

    public static void enableSwingAllDebugLog() {
        PlatformLogger log = PlatformLogger.getLogger("java.awt.Component");
        log.setLevel(PlatformLogger.Level.ALL);
        PlatformLogger eventLog = PlatformLogger.getLogger("java.awt.event.Component");
        eventLog.setLevel(PlatformLogger.Level.ALL);
        PlatformLogger focusLog = PlatformLogger.getLogger("java.awt.focus.Component");
        focusLog.setLevel(PlatformLogger.Level.ALL);
        PlatformLogger mixingLog = PlatformLogger.getLogger("java.awt.mixing.Component");
        mixingLog.setLevel(PlatformLogger.Level.ALL);

        PlatformLogger containerLog = PlatformLogger.getLogger("java.awt.Container");
        containerLog.setLevel(PlatformLogger.Level.ALL);
        PlatformLogger containerEventLog = PlatformLogger.getLogger("java.awt.event.Container");
        containerEventLog.setLevel(PlatformLogger.Level.ALL);

        PlatformLogger focusManagerLog = PlatformLogger.getLogger("java.awt.focus.KeyboardFocusManager");
        focusManagerLog.setLevel(PlatformLogger.Level.ALL);
        PlatformLogger defaultFocusManagerLog = PlatformLogger.getLogger("java.awt.focus.DefaultKeyboardFocusManager");
        defaultFocusManagerLog.setLevel(PlatformLogger.Level.ALL);
    }

    public static void logClass(final Class<?> clazz) {
        logClass(clazz.getName());
    }

    public static void logClass(final String name) {
        ConsoleHandler consoleHandler = new ConsoleHandler();
        consoleHandler.setLevel(Level.ALL);
        consoleHandler.setFormatter(new Formatter() {
            @Override
            public String format(LogRecord record) {
                return name + ": " + record.getMessage() + '\n';
            }
        });
        Logger logger = Logger.getLogger(name);
        logger.setLevel(Level.ALL);
        logger.addHandler(consoleHandler);
    }
}
