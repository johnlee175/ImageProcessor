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
 * camera native view cc implement
 *
 * @author John Kenrinus Lee
 * @version 2017-08-18
 */
#include "CCCameraNativeView.hpp"

CCCameraNativeView::CCCameraNativeView() {
    this->cnv = cnv_create_initialize();
}

CCCameraNativeView::~CCCameraNativeView() {
    cnv_destroy_release(this->cnv);
}

void CCCameraNativeView::setUserTag(void *data) {
    cnv_set_user_tag(this->cnv, data);
}

void *CCCameraNativeView::getUserTag() {
    return cnv_get_user_tag(this->cnv);
}

void CCCameraNativeView::setFrameSize(GLuint frame_width, GLuint frame_height) {
    cnv_set_frame_size(this->cnv, frame_width, frame_height);
}

void CCCameraNativeView::setFrontCamera(GLboolean is_front_camera) {
    cnv_set_front_camera(this->cnv, is_front_camera);
}

void CCCameraNativeView::setShaderSource(const GLchar *vertex_source, const GLchar *fragment_source) {
    cnv_set_shader_source(this->cnv, vertex_source, fragment_source);
}

void CCCameraNativeView::createShader() {
    cnv_create_shader(this->cnv);
}

void CCCameraNativeView::destroyShader() {
    cnv_destroy_shader(this->cnv);
}

void CCCameraNativeView::drawFrame(GLuint texture_id) {
    cnv_draw_frame(this->cnv, texture_id);
}

void CCCameraNativeView::setFrameDataCallback(cnv_frame_data_callback callback_func) {
    cnv_set_frame_data_callback(this->cnv, callback_func);
}

void CCCameraNativeView::readPixels(GLubyte **pixels, size_t *pixels_size) {
    cnv_get_pixels(this->cnv, pixels, pixels_size);
}
