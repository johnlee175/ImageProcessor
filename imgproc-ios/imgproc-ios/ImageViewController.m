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
 * ImageViewController.m
 * imgproc-ios
 * @author John Kenrinus Lee
 * @version 2017-09-30
 */

#import "ImageViewController.h"
#import "SettingsViewController.h"
#import "gpu_image.h"

@interface ImageViewController () {
    UIImage *mImage;
}
@end

@implementation ImageViewController

#define DEFAULT_CHANNELS 4
#define BIT_PER_COMPONENT 8

static GContext *gk_jni_context;
static char *hello1_frag_shader_src;
static char *hello2_frag_shader_src;
static char *hello3_frag_shader_src;
static uint8_t *data;

// core method, make sure call this method in same thread as GL thread
// TODO reuse GProcessFlags object
// TODO memory may increasing while each call this method via OpenGLES
+ (UIImage *)imageProcess:(UIImage *)image shader_source:(const char *)shader_source {
    if (gk_jni_context == NULL) {
        gk_jni_context = glbox2_create_context(NULL);
    }
    
    GImage *origin = [ImageViewController getImagePixels:image];
    if (origin == NULL) {
        @throw [NSException exceptionWithName:@"Create Failed" reason:@"create origin[GImage] failed" userInfo:nil];
    }
    
    DEFINE_HEAP_TYPE_POINTER(GImage, target, {
        @throw [NSException exceptionWithName:@"Malloc Failed" reason:@"malloc target[GImage] failed" userInfo:nil];
    });
    ASSIGN_HEAP_ARRAY_POINTER(uint8_t, target->image,
                              origin->width * origin->height * origin->channels, {
                                  @throw [NSException exceptionWithName:@"Malloc Failed" reason:@"malloc target_bytes[uint8_t] failed" userInfo:nil];
                              });
    target->width = origin->width;
    target->height = origin->height;
    target->channels = origin->channels;
    
    DEFINE_HEAP_TYPE_POINTER(GProcessFlags, process_flags, {
        @throw [NSException exceptionWithName:@"Malloc Failed" reason:@"malloc process_flags[GProcessFlags] failed" userInfo:nil];
    });
    process_flags->fbo_by_texture = true;
    process_flags->reuse_fbo = true;
    process_flags->reuse_vao = true;
    process_flags->reuse_texture = true;
    process_flags->reuse_program = true;
    
    if (glbox2_image_process(gk_jni_context, target, origin, shader_source,
                             process_flags) < 0) {
        @throw [NSException exceptionWithName:@"Call Failed" reason:@"call glbox2_image_process failed" userInfo:nil];
    }
    
    UIImage *targetImage = [ImageViewController getImageObject:target];
    
    GProcessFlags *flags[1];
    flags[0] = process_flags;
    glbox2_destroy_context(gk_jni_context, NULL, flags, 1);
    gk_jni_context = NULL;
    
    FREE_POINTER(origin->image);
    FREE_POINTER(origin);
    FREE_POINTER(target->image);
    FREE_POINTER(target);
    FREE_POINTER(process_flags->private_data);
    FREE_POINTER(process_flags);
    
    return targetImage;
}

+ (GImage *)getImagePixels:(UIImage *)image {
    DEFINE_HEAP_TYPE_POINTER(GImage, cgImage, {
        @throw [NSException exceptionWithName:@"Malloc Failed" reason:@"malloc cgImage[GImage] failed" userInfo:nil];
    });
    cgImage->width = image.size.width;
    cgImage->height = image.size.height;
    cgImage->channels = DEFAULT_CHANNELS;
    ASSIGN_HEAP_ARRAY_POINTER(uint8_t, cgImage->image,
                              cgImage->width * cgImage->height * cgImage->channels, {
                                  @throw [NSException exceptionWithName:@"Malloc Failed" reason:@"malloc cgImage->image[uint8_t] failed" userInfo:nil];
                              });
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef context = CGBitmapContextCreate(cgImage->image, cgImage->width, cgImage->height,
                                                 BIT_PER_COMPONENT, cgImage->width * cgImage->channels,
                                                 colorSpace,
                                                 kCGImageAlphaPremultipliedLast);
    CGRect rect = CGRectMake(0, 0, cgImage->width, cgImage->height);
    CGContextDrawImage(context, rect, [image CGImage]);
    CGContextRelease(context);
    CGColorSpaceRelease(colorSpace);
    return cgImage;
}

+ (UIImage *)getImageObject:(GImage *)image {
    size_t size = image->width * image->height * image->channels;
    memcpy(data, image->image, size);
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, data, size, NULL);
    CGImageRef imageRef = CGImageCreate(image->width, image->height, BIT_PER_COMPONENT,
                                        BIT_PER_COMPONENT * DEFAULT_CHANNELS,
                                        image->width * DEFAULT_CHANNELS,
                                        colorSpace, kCGBitmapByteOrderDefault | kCGImageAlphaLast,
                                        provider, NULL, NO, kCGRenderingIntentDefault);
    UIImage *result = [UIImage imageWithCGImage:imageRef];
    CGImageRelease(imageRef);
    CGDataProviderRelease(provider); // TODO is data free when provider release, is it memory leak?
    CGColorSpaceRelease(colorSpace);
    return result;
}

+ (void)initialize {
    if (self == [ImageViewController class]) {
        [ImageViewController loadShaders];
    }
}

+ (void)loadShaders {
    const char *tmp;
    size_t tmp_len;
    
    NSString *path1 = [[NSBundle mainBundle] pathForResource:@"hello1" ofType:@"fs"];
    NSString *hello1 = [[NSString alloc] initWithContentsOfFile:path1
                                                       encoding:NSUTF8StringEncoding error:nil];
    tmp = [hello1 cStringUsingEncoding:NSUTF8StringEncoding];
    tmp_len = strlen(tmp) + 1;
    ASSIGN_HEAP_ARRAY_POINTER(char, hello1_frag_shader_src, tmp_len, {
        @throw [NSException exceptionWithName:@"Malloc Failed" reason:@"malloc hello1_frag_shader_src[char] failed" userInfo:nil];
    });
    memcpy(hello1_frag_shader_src, tmp, tmp_len);
    
    NSString *path2 = [[NSBundle mainBundle] pathForResource:@"hello2" ofType:@"fs"];
    NSString *hello2 = [[NSString alloc] initWithContentsOfFile:path2
                                                       encoding:NSUTF8StringEncoding error:nil];
    tmp = [hello2 cStringUsingEncoding:NSUTF8StringEncoding];
    tmp_len = strlen(tmp) + 1;
    ASSIGN_HEAP_ARRAY_POINTER(char, hello2_frag_shader_src, tmp_len, {
        @throw [NSException exceptionWithName:@"Malloc Failed" reason:@"malloc hello2_frag_shader_src[char] failed" userInfo:nil];
    });
    memcpy(hello2_frag_shader_src, tmp, tmp_len);
    
    NSString *path3 = [[NSBundle mainBundle] pathForResource:@"hello3" ofType:@"fs"];
    NSString *hello3 = [[NSString alloc] initWithContentsOfFile:path3
                                                       encoding:NSUTF8StringEncoding error:nil];
    tmp = [hello3 cStringUsingEncoding:NSUTF8StringEncoding];
    tmp_len = strlen(tmp) + 1;
    ASSIGN_HEAP_ARRAY_POINTER(char, hello3_frag_shader_src, tmp_len, {
        @throw [NSException exceptionWithName:@"Malloc Failed" reason:@"malloc hello3_frag_shader_src[char] failed" userInfo:nil];
    });
    memcpy(hello3_frag_shader_src, tmp, tmp_len);
}

- (void)setOriginImage:(UIImage *)image {
    size_t size = image.size.width * image.size.height * DEFAULT_CHANNELS;
    ASSIGN_HEAP_ARRAY_POINTER(uint8_t, data, size, {
        @throw [NSException exceptionWithName:@"Malloc Failed" reason:@"malloc data[uint8_t] failed" userInfo:nil];
    });
    mImage = image;
    [self adapterImage:mImage];
}

- (void)adapterImage:(UIImage *)image {
    self.imageView.image = image;
    self.imageView.frame = CGRectMake(0, 0, image.size.width, image.size.height);
    self.scrollView.contentSize = image.size;
    [self.scrollView setNeedsLayout];
}

- (UIImage *)getWillProcessingImage {
    if ([SettingsViewController isOnAlwaysOrigin] == YES) {
        return mImage;
    } else {
        return self.imageView.image;
    }
}

- (void)onLongPressed:(UILongPressGestureRecognizer *)recognizer {
    if (recognizer.state == UIGestureRecognizerStateBegan) {
        UIAlertController *alert = [UIAlertController
                                   alertControllerWithTitle:@"Filter Chooser"
                                   message:nil
                                   preferredStyle:UIAlertControllerStyleActionSheet];
        [alert addAction: [UIAlertAction
                           actionWithTitle:@"Origin"
                           style:UIAlertActionStyleDefault
                           handler:^(UIAlertAction *action)
                           {
                               UIImage *image = [ImageViewController
                                                 imageProcess:mImage
                                                 shader_source:default_fragment_shader_source];
                               [self adapterImage:image];
                           }]];
        [alert addAction: [UIAlertAction
                           actionWithTitle:@"Negative"
                           style:UIAlertActionStyleDefault
                           handler:^(UIAlertAction *action)
                           {
                               UIImage *image = [ImageViewController
                                                 imageProcess:[self getWillProcessingImage]
                                                 shader_source:hello1_frag_shader_src];
                               [self adapterImage:image];
                           }]];
        [alert addAction: [UIAlertAction
                           actionWithTitle:@"Sketch"
                           style:UIAlertActionStyleDefault
                           handler:^(UIAlertAction *action)
                           {
                               UIImage *image = [ImageViewController
                                                 imageProcess:[self getWillProcessingImage]
                                                 shader_source:hello2_frag_shader_src];
                               [self adapterImage:image];
                           }]];
        [alert addAction: [UIAlertAction
                           actionWithTitle:@"WhiteSkin"
                           style:UIAlertActionStyleDefault
                           handler:^(UIAlertAction *action)
                           {
                               UIImage *image = [ImageViewController
                                                 imageProcess:[self getWillProcessingImage]
                                                 shader_source:hello3_frag_shader_src];
                               [self adapterImage:image];
                           }]];
        
        [alert addAction: [UIAlertAction
                           actionWithTitle:@"Cancel"
                           style:UIAlertActionStyleCancel
                           handler:nil]];
        
        [self presentViewController:alert animated:YES completion:^
         {
             
         }];
    }
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.scrollView.delegate = self;
    self.scrollView.maximumZoomScale = 10.0;
    self.scrollView.minimumZoomScale = 1.0;
    
    self.imageView.contentMode =  UIViewContentModeScaleAspectFill;
    self.imageView.autoresizingMask = UIViewAutoresizingFlexibleHeight;
    self.imageView.clipsToBounds = YES;
    [self.imageView setContentScaleFactor:[[UIScreen mainScreen] scale]];
    [self.imageView addGestureRecognizer:[[UILongPressGestureRecognizer alloc]
                                          initWithTarget:self action:@selector(onLongPressed:)]];

    [self setOriginImage:[UIImage imageNamed: @"image_test"]];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
}

-(UIView *)viewForZoomingInScrollView:(UIScrollView *)scrollView {
    return self.imageView;
}

@end
