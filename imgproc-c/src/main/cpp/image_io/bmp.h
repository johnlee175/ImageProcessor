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
 * @author John Kenrinus Lee
 * @version 2017-09-25
 */
#ifndef PROJECT_BMP_H
#define PROJECT_BMP_H

#include "base/base.h"

int load_bmp_24_b2t(const char *bmp_path, uint8_t **rgb_buffer/* should user free */,
                    uint32_t *width, uint32_t *height);
int save_bmp_24_b2t(const char *bmp_path, const uint8_t *rgb_buffer/* should user free */,
                    const uint32_t width, const uint32_t height);

#endif /* PROJECT_BMP_H */
