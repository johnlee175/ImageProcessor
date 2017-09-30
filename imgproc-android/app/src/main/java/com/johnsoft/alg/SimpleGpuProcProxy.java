package com.johnsoft.alg;

import java.util.Map;
import java.util.concurrent.LinkedBlockingQueue;

/**
 * Simple Gpu Process Proxy
 * NOTICE: OpenGL need run on single thread!
 * @author John Kenrinus Lee
 * @version 2017-09-29
 */
public class SimpleGpuProcProxy {
    private static final int STATE_UNDEFINED = -1;
    private static final int STATE_STARTED = 0;
    private static final int STATE_STOPED = 1;

    private int state;
    private LoopThread loopThread;
    private SimpleGpuProc simpleGpuProc;

    public SimpleGpuProcProxy() {
        state = STATE_UNDEFINED;
    }

    public synchronized SimpleGpuProcProxy create() {
        if (state == STATE_UNDEFINED) {
            System.out.println("SimpleGpuProcProxy: #create()");
            if (loopThread != null) {
                loopThread.quit();
                loopThread = null;
            }
            loopThread = new LoopThread();
            loopThread.start();
            loopThread.post(new Runnable() {
                @Override
                public void run() {
                    if (simpleGpuProc != null) {
                        simpleGpuProc.destroy();
                        simpleGpuProc = null;
                    }
                    System.out.println("SimpleGpuProcProxy: #do real create()");
                    simpleGpuProc = new SimpleGpuProc();
                }
            });
            state = STATE_STARTED;
        }
        return this;
    }

    public synchronized int[] imageProc(int[] argb, int w, int h,
                                        int type, Map<String, Object> paramsMap) {
        if (state != STATE_STARTED) {
            System.out.println("SimpleGpuProcProxy: state != STATE_STARTED");
            return null;
        }
        if (loopThread == null) {
            System.out.println("SimpleGpuProcProxy: loopThread == null");
            return null;
        }
        if (simpleGpuProc == null) {
            System.out.println("SimpleGpuProcProxy: simpleGpuProc == null");
            return null;
        }
        final ProcTask procTask = new ProcTask(argb, w, h, type, paramsMap, simpleGpuProc);
        loopThread.post(procTask);
        return procTask.getResult();
    }

    public synchronized SimpleGpuProcProxy destroy() {
        if (state == STATE_STARTED) {
            System.out.println("SimpleGpuProcProxy: #destroy()");
            if (loopThread != null) {
                loopThread.post(new Runnable() {
                    @Override
                    public void run() {
                        if (simpleGpuProc != null) {
                            System.out.println("SimpleGpuProcProxy: #do real destroy()");
                            simpleGpuProc.destroy();
                            simpleGpuProc = null;
                        }
                        if (loopThread != null) {
                            System.out.println("SimpleGpuProcProxy: #do real quit()");
                            loopThread.quit();
                            loopThread = null;
                        }
                    }
                });
            }
            state = STATE_STOPED;
        }
        return this;
    }

    private static final class LoopThread extends Thread {
        private static final int MAX_LEN = 1000;
        private final LinkedBlockingQueue<Runnable> queue = new LinkedBlockingQueue<>();
        private volatile boolean quited;

        public LoopThread() {
            super("Loop-Thread");
        }

        public boolean post(Runnable task) {
            if (!quited && task != null && queue.size() < MAX_LEN) {
                return queue.offer(task);
            }
            return false;
        }

        public void quit() {
            if (!quited) {
                quited = true;
                interrupt();
            }
        }

        @Override
        public void run() {
            try {
                Runnable task;
                while (!quited && !isInterrupted()) {
                    task = queue.take();
                    try {
                        task.run();
                    } catch (Throwable thr) {
                        thr.printStackTrace();
                    }
                }
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }

    private static final class ProcTask implements Runnable {
        private final int[] argb;
        private final int w;
        private final int h;
        private final int type;
        private final Map<String, Object> paramsMap;
        private final SimpleGpuProc simpleGpuProc;
        private int[] result;
        private boolean calcOk;

        public ProcTask(int[] argb, int w, int h,
                        int type, Map<String, Object> paramsMap,
                        SimpleGpuProc simpleGpuProc) {
            this.argb = argb;
            this.w = w;
            this.h = h;
            this.type = type;
            this.paramsMap = paramsMap;
            this.simpleGpuProc = simpleGpuProc;
        }

        @Override
        public synchronized void run() {
            System.out.println("SimpleGpuProcProxy: #do real imageProc()");
            result = simpleGpuProc.imageProc(argb, w, h, type, paramsMap);
            calcOk = true;
            notifyAll();
        }

        public synchronized int[] getResult() {
            try {
                while (!calcOk) {
                    wait(1000L);
                }
                return result;
            } catch (InterruptedException e) {
                e.printStackTrace();
                return null;
            }
        }
    }
}
