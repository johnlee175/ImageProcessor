#include "base/base.h"
#include "../gpu_image.h"

struct tagInternalGContext {
    CGLContextObj context_obj;
};

static int prepare_off_screen_context(GContext *g_context, GCreateFlags *flag) {
    if (!g_context || !g_context->native_context) {
        base_error_log("pass a GContext pointer to NULL or its native_context is NULL\n");
        return -1;
    }
    CGLPixelFormatAttribute attributes[] = {
            kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute) kCGLOGLPVersion_3_2_Core,
            kCGLPFAAccelerated,
            kCGLPFANoRecovery,
            kCGLPFADoubleBuffer,
            kCGLPFAColorSize, (CGLPixelFormatAttribute) 24,
            kCGLPFAAlphaSize, (CGLPixelFormatAttribute) 8,
            kCGLPFADepthSize, (CGLPixelFormatAttribute) 24,
            kCGLPFAStencilSize, (CGLPixelFormatAttribute) 8,
            kCGLPFAAccumSize, (CGLPixelFormatAttribute) 0,
            kCGLPFASampleBuffers, (CGLPixelFormatAttribute) 1,
            kCGLPFASamples,  (CGLPixelFormatAttribute) 4,
            (CGLPixelFormatAttribute) 0
    };
    CGLError errorCode;
    CGLPixelFormatObj pixelFormatObj;
    CGLContextObj context;
    GLint num = 0; /* stores the number of possible pixel formats */
    errorCode = CGLChoosePixelFormat(attributes, &pixelFormatObj, &num);
    if (errorCode != kCGLNoError || pixelFormatObj == NULL) {
        base_error_log("call CGLChoosePixelFormat failed\n");
        return -1;
    }
    errorCode = CGLCreateContext(pixelFormatObj, NULL, &context);
    if (errorCode != kCGLNoError || context == NULL) {
        base_error_log("call CGLCreateContext failed\n");
        return -1;
    }
    errorCode = CGLDestroyPixelFormat(pixelFormatObj);
    if (errorCode != kCGLNoError) {
        base_info_log("call CGLDestroyPixelFormat failed\n");
    }
    errorCode = CGLSetCurrentContext(context);
    if (errorCode != kCGLNoError) {
        base_error_log("call CGLSetCurrentContext failed\n");
        return -1;
    }
    glewExperimental = GL_TRUE;
    if (glewInit()) {
        base_error_log("call glewInit failed\n");
        return -1;
    }
    g_context->native_context->context_obj = context;
    return 0;
}

static int mark_current_off_screen_context(GContext *context) {
    if (context && context->native_context->context_obj) {
        if (CGLSetCurrentContext(context->native_context->context_obj) != kCGLNoError) {
            base_error_log("call CGLSetCurrentContext failed\n");
            return -1;
        } else {
            return 0;
        }
    } else {
        base_error_log("mark_current_off_screen_context: "
                               "context == NULL || context->context_obj == NULL\n");
        return -1;
    }
}

static void release_off_screen_context(GContext *context) {
    if (context && context->native_context->context_obj) {
        CGLDestroyContext(context->native_context->context_obj);
    }
    CGLSetCurrentContext(NULL);
}
