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
 * camera native view cc interface
 *
 * @author John Kenrinus Lee
 * @version 2017-08-18
 */
#ifndef IMGPROC_ANDROID_CCCAMERANATIVEVIEW_HPP
#define IMGPROC_ANDROID_CCCAMERANATIVEVIEW_HPP

#include "camera_native_view.h"

class CCCameraNativeView {
public:
    CCCameraNativeView();
    ~CCCameraNativeView();
    void setUserTag(void *data);
    void *getUserTag();
    void setFrameSize(GLuint frame_width, GLuint frame_height);
    void setFrontCamera(GLboolean is_front_camera);
    void setShaderSource(const GLchar *vertex_source, const GLchar *fragment_source);
    void createShader();
    void destroyShader();
    void drawFrame(GLuint texture_id);
    void setFrameDataCallback(cnv_frame_data_callback callback_func);
    void readPixels(GLubyte **pixels, size_t *pixels_size);
private:
    CameraNativeViewStruct *cnv;
};

#endif //IMGPROC_ANDROID_CCCAMERANATIVEVIEW_HPP
