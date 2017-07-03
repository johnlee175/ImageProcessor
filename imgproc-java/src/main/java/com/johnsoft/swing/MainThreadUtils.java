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

import java.awt.EventQueue;
import java.util.LinkedList;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.ScheduledThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

/**
 * @author John Kenrinus Lee
 * @version 2017-07-02
 */
public class MainThreadUtils {
    private static final ScheduledExecutorService scheduler = new ScheduledThreadPoolExecutor(2);

    private static LinkedList<ScheduledFuture<?>> futures = new LinkedList<>();

    public static void post(Runnable runnable) {
        EventQueue.invokeLater(runnable);
    }

    public static void postDelay(final Runnable runnable, long delayMillis) {
        futures.add(scheduler.schedule(new Runnable() {
            @Override
            public void run() {
                EventQueue.invokeLater(runnable);
            }
        }, delayMillis, TimeUnit.MILLISECONDS));
    }

    public static void clearDelayPost() {
        ScheduledFuture<?> future;
        while ((future = futures.pollLast()) != null) {
            future.cancel(true);
        }
    }
}
