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
#ifndef PROJECT_COMMON_H
#define PROJECT_COMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include <stdarg.h>
#include <setjmp.h>
#include <limits.h>

#define ASSIGN_HEAP_ARRAY_POINTER(type, name, size, ...) \
    name = (type *) malloc(sizeof(type) * size); \
    if (name != NULL) { \
        memset(name, 0, sizeof(type) * size); \
    } else { \
        __VA_ARGS__ \
    } \
    do { } while(0)

#define DEFINE_HEAP_ARRAY_POINTER(type, name, size, ...) \
    type *name = (type *) malloc(sizeof(type) * size); \
    if (name != NULL) { \
        memset(name, 0, sizeof(type) * size); \
    } else { \
        __VA_ARGS__ \
    } \
    do { } while(0)

#define ASSIGN_HEAP_TYPE_POINTER(type, name, ...) \
    ASSIGN_HEAP_ARRAY_POINTER(type, name, 1, __VA_ARGS__)

#define DEFINE_HEAP_TYPE_POINTER(type, name, ...) \
    DEFINE_HEAP_ARRAY_POINTER(type, name, 1, __VA_ARGS__)

#define FREE_POINTER(name) \
    if (name != NULL) { \
        free(name); \
        name = NULL; \
    } \
    do { } while(0)

#define PARAM_IN
#define PARAM_OUT
#define PARAM_INOUT
#define OPTIONAL

#endif /* PROJECT_COMMON_H */
