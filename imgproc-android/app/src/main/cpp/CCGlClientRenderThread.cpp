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
 * GL client render thread cc implement
 *
 * @author John Kenrinus Lee
 * @version 2017-08-18
 */
#include "CCGlClientRenderThread.hpp"

CCGlClientRenderThread::CCGlClientRenderThread(const char *thread_name,
                                               BoolProperty **boolProperty,
                                               Drawer **drawer) {
    this->glcrt = glcrt_create_initialize(thread_name, boolProperty, drawer);
}

CCGlClientRenderThread::~CCGlClientRenderThread() {
    glcrt_destroy_release(this->glcrt);
}

void CCGlClientRenderThread::start() {
    glcrt_start(this->glcrt);
}

void CCGlClientRenderThread::quit() {
    glcrt_quit(this->glcrt);
}

void CCGlClientRenderThread::setWindow(EGLNativeWindowType window) {
    glcrt_set_window(this->glcrt, window);
}

void CCGlClientRenderThread::setAvailable(bool available) {
    glcrt_set_available(this->glcrt, available);
}

bool CCGlClientRenderThread::isLoop() {
    bool result = false;
    glcrt_get_loop(this->glcrt, &result);
    return result;
}

const char *CCGlClientRenderThread::getName() {
    const char *result = NULL;
    glcrt_get_name(this->glcrt, &result);
    return result;
}

void CCGlClientRenderThread::handleMessage(const char *message) {
    glcrt_handle_message(this->glcrt, message);
}
