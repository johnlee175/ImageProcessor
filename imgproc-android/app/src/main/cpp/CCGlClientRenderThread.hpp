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
/**
 * GL client render thread cc interface
 *
 * @author John Kenrinus Lee
 * @version 2017-08-18
 */
#ifndef IMGPROC_ANDROID_CCGLCLIENTRENDERTHREAD_HPP
#define IMGPROC_ANDROID_CCGLCLIENTRENDERTHREAD_HPP

#include "glclient_render_thread.h"

class CCGlClientRenderThread {
public:
    CCGlClientRenderThread(const char *thread_name,
                           BoolProperty **boolProperty,
                           Drawer **drawer);
    ~CCGlClientRenderThread();
    void start();
    void quit();
    void setWindow(EGLNativeWindowType window);
    void setAvailable(bool available);
    bool isLoop();
    const char *getName();
    void handleMessage(const char *message);
private:
    GLClientRenderThreadStruct *glcrt;
};

#endif //IMGPROC_ANDROID_CCGLCLIENTRENDERTHREAD_HPP
