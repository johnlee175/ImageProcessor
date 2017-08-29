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
package com.johnsoft.viewer;

import java.util.HashMap;
import java.util.HashSet;
import java.util.concurrent.LinkedBlockingDeque;

/**
 * @author John Kenrinus Lee
 * @version 2017-04-13
 */
public class MessageThread extends Thread {
    public static class Message {
        public final String what;
        public final HashMap<String, Object> map = new HashMap<>();

        public Message(String what) {
            this.what = what;
        }

        public Message addKeyValue(String key, Object value) {
            map.put(key, value);
            return this;
        }

        public Message removeKey(String key) {
            map.remove(key);
            return this;
        }
    }

    public static abstract class Handler {
        private volatile MessageThread messageThread;

        abstract void handleMessage(Message message);

        public final void sendMessage(Message message) {
            if (messageThread != null && !messageThread.quit) {
                messageThread.deque.addLast(message);
            }
        }

        public final void sendMessageFront(Message message) {
            if (messageThread != null && !messageThread.quit) {
                messageThread.deque.addFirst(message);
            }
        }

        public final void removeMessage(Message message) {
            if (messageThread != null && !messageThread.quit) {
                messageThread.deque.remove(message);
            }
        }
    }

    private final LinkedBlockingDeque<Message> deque = new LinkedBlockingDeque<>();
    private final byte[] handleLock = new byte[0];
    private final HashSet<Handler> handlers = new HashSet<>();
    private volatile boolean quit;

    public void quit() {
        deque.clear();
        quit = true;
        interrupt();
    }

    public void addHandler(Handler handler) {
        synchronized (handleLock) {
            handlers.add(handler);
        }
        handler.messageThread = this;
    }

    public void removeHandler(Handler handler) {
        handler.messageThread = null;
        synchronized (handleLock) {
            handlers.remove(handler);
        }
    }

    @Override
    public void run() {
        try {
            while (!quit) {
                final Message message = deque.takeFirst();
                synchronized (handleLock) {
                    for (Handler handler : handlers) {
                        try {
                            handler.handleMessage(message);
                        } catch (Throwable e) {
                            e.printStackTrace();
                        }
                    }
                }
            }
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}
