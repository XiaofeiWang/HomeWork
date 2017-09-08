/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sched.h>
#include <sys/resource.h>

#include <stdint.h>
#include <stdlib.h>

#include <system/window.h>
#include <utils/Errors.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES/gl.h>
#include <GLES/glext.h>

#include <utils/Timers.h>

#include <ui/GraphicBuffer.h>
#include <gui/SurfaceControl.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/ISurfaceComposer.h>
#include <gui/Surface.h>
#include <ui/DisplayInfo.h>

using namespace android;

status_t selectConfigForPixelFormat(
        EGLDisplay dpy,
        EGLint const* attrs,
        int32_t format,
        EGLConfig* outConfig)
{
    EGLint numConfigs = -1, n=0;

    if (!attrs)
        return BAD_VALUE;

    if (outConfig == NULL)
        return BAD_VALUE;

    // Get all the "potential match" configs...
    if (eglGetConfigs(dpy, NULL, 0, &numConfigs) == EGL_FALSE)
        return BAD_VALUE;

    EGLConfig* const configs = (EGLConfig*)malloc(sizeof(EGLConfig)*numConfigs);
    if (eglChooseConfig(dpy, attrs, configs, numConfigs, &n) == EGL_FALSE) {
        free(configs);
        return BAD_VALUE;
    }

    int i;
    EGLConfig config = NULL;
    for (i=0 ; i<n ; i++) {
        EGLint nativeVisualId = 0;
        eglGetConfigAttrib(dpy, configs[i], EGL_NATIVE_VISUAL_ID, &nativeVisualId);
        if (nativeVisualId>0 && format == nativeVisualId) {
            config = configs[i];
            break;
        }
    }

    free(configs);

    if (i<n) {
        *outConfig = config;
        return NO_ERROR;
    }

    return NAME_NOT_FOUND;
}

void memset16(uint16_t* dst, uint16_t value, size_t size)
{
    size >>= 1;
    while (size--) {
        *dst++ = value;
    }
}

int main(int argc, char *argv[])
{
    EGLint majorVersion;
    EGLint minorVersion;
    EGLContext context;
    EGLConfig config;
    EGLSurface surface;
    EGLint w, h;
    EGLDisplay dpy;
    status_t err; 
    GLuint texture1;
    GLuint texture2;
    int format;

    EGLint configAttribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
            EGL_NONE };

    sp<SurfaceComposerClient> surfaceComposerClient = new SurfaceComposerClient;
    err = surfaceComposerClient->initCheck();
    if (err != NO_ERROR) {
        fprintf(stderr, "SurfaceComposerClient::initCheck error: %#x\n", err);
        return err;
    }

    // Get main display parameters.
    sp<IBinder> mainDpy = SurfaceComposerClient::getBuiltInDisplay(
            ISurfaceComposer::eDisplayIdMain);
    DisplayInfo mainDpyInfo;
    err = SurfaceComposerClient::getDisplayInfo(mainDpy, &mainDpyInfo);
    if (err != NO_ERROR) {
        fprintf(stderr, "ERROR: unable to get display characteristics\n");
        return err;
    }

    uint32_t width, height;
    if (mainDpyInfo.orientation != DISPLAY_ORIENTATION_0 &&
            mainDpyInfo.orientation != DISPLAY_ORIENTATION_180) {
        // rotated
        width = mainDpyInfo.h;
        height = mainDpyInfo.w;
    } else {
        width = mainDpyInfo.w;
        height = mainDpyInfo.h;
    }

    sp<SurfaceControl> surfaceControl = surfaceComposerClient->createSurface(
            String8("Opengl-test"), width, height,
            PIXEL_FORMAT_RGBX_8888, ISurfaceComposerClient::eOpaque);
    if (surfaceControl == NULL || !surfaceControl->isValid()) {
        fprintf(stderr, "Failed to create SurfaceControl\n");
        return -1;
    }

    SurfaceComposerClient::openGlobalTransaction();
    err = surfaceControl->setLayer(0x7FFFFFFF);     // always on top
    if (err != NO_ERROR) {
        fprintf(stderr, "SurfaceComposer::setLayer error: %#x\n", err);
        return err;
    }

    err = surfaceControl->show();
    if (err != NO_ERROR) {
        fprintf(stderr, "SurfaceComposer::show error: %#x\n", err);
        return err;
    }
    SurfaceComposerClient::closeGlobalTransaction();

    dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(dpy, &majorVersion, &minorVersion);
    sp<ANativeWindow> anw = surfaceControl->getSurface();
    EGLNativeWindowType window = (EGLNativeWindowType) anw.get();
    if ((err = window->query(window, NATIVE_WINDOW_FORMAT, &format)) < 0) {
        return err;
    }

    err = selectConfigForPixelFormat(
            dpy, configAttribs, format, &config);
    if (err) {
        fprintf(stderr, "couldn't find an EGLConfig matching the screen format\n");
        return err;
    }
    surface = eglCreateWindowSurface(dpy, config, window, NULL);
    context = eglCreateContext(dpy, config, NULL, NULL);
    eglMakeCurrent(dpy, surface, surface, context);   
    eglQuerySurface(dpy, surface, EGL_WIDTH, &w);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &h);

    printf("w=%d, h=%d\n", w, h);

    /*
    * set the GraphicBuffer1
    */
    sp<GraphicBuffer> graphicBuffer1 = new GraphicBuffer(w, h, HAL_PIXEL_FORMAT_RGBA_8888,
             GraphicBuffer::USAGE_HW_TEXTURE | GraphicBuffer::USAGE_SW_WRITE_RARELY);
    uint16_t* bits1;
    graphicBuffer1->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)(&bits1));
    ssize_t bpr1 = graphicBuffer1->getStride() * bytesPerPixel(graphicBuffer1->getPixelFormat());
    memset16(bits1, 0xF800, bpr1*graphicBuffer1->getHeight());
    err = graphicBuffer1->unlock();
    if (err != 0) {
        fprintf(stderr, "graphicBuffer1->unlock() failed: %d\n", err);
        return err;
    }

    /*
    * set the GraphicBuffer2
    */
    sp<GraphicBuffer> graphicBuffer2 = new GraphicBuffer(w, h, HAL_PIXEL_FORMAT_RGBA_8888,
             GraphicBuffer::USAGE_HW_TEXTURE | GraphicBuffer::USAGE_SW_WRITE_RARELY);
    uint16_t* bits2;
    graphicBuffer2->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)(&bits2));
    ssize_t bpr2 = graphicBuffer2->getStride() * bytesPerPixel(graphicBuffer2->getPixelFormat());
    memset16(bits2, 0xF800, bpr2*graphicBuffer2->getHeight());
    err = graphicBuffer2->unlock();
    if (err != 0) {
        fprintf(stderr, "graphicBuffer2->unlock() failed: %d\n", err);
        return err;
    }

    /*
    * set the param1
    */
    Rect crop1(0, 0, w, h);
    EGLClientBuffer clientBuffer1 = (EGLClientBuffer)graphicBuffer1->getNativeBuffer();
    EGLint attrs1[] = {
        EGL_IMAGE_PRESERVED_KHR,        EGL_TRUE,
        EGL_IMAGE_CROP_LEFT_ANDROID,    crop1.left,
        EGL_IMAGE_CROP_TOP_ANDROID,     crop1.top,
        EGL_IMAGE_CROP_RIGHT_ANDROID,   crop1.right,
        EGL_IMAGE_CROP_BOTTOM_ANDROID,  crop1.bottom,
        EGL_NONE,
    };
    EGLImageKHR img1 = eglCreateImageKHR(dpy, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID,
            clientBuffer1, attrs1);
    if (img1 == EGL_NO_IMAGE_KHR) {
        return err;
    }
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture1);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, (GLeglImageOES)img1);
    glTexParameterx(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterx(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterx(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterx(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_REPEAT);

    /*
    * set the param2
    */
    Rect crop2(0, 0, w, h);
    EGLClientBuffer clientBuffer2 = (EGLClientBuffer)graphicBuffer2->getNativeBuffer();
    EGLint attrs2[] = {
        EGL_IMAGE_PRESERVED_KHR,        EGL_TRUE,
        EGL_IMAGE_CROP_LEFT_ANDROID,    crop2.left,
        EGL_IMAGE_CROP_TOP_ANDROID,     crop2.top,
        EGL_IMAGE_CROP_RIGHT_ANDROID,   crop2.right,
        EGL_IMAGE_CROP_BOTTOM_ANDROID,  crop2.bottom,
        EGL_NONE,
    };
    EGLImageKHR img2 = eglCreateImageKHR(dpy, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID,
            clientBuffer2, attrs2);
    if (img2 == EGL_NO_IMAGE_KHR) {
        return err;
    }
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture2);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, (GLeglImageOES)img2);
    glTexParameterx(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterx(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterx(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterx(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_REPEAT);

    for(;;) {
        glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
        glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture1);
        glEnable(GL_TEXTURE_EXTERNAL_OES);
        glDrawTexiOES(0, 0, 0, w, h);

        glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture2);
        glEnable(GL_TEXTURE_EXTERNAL_OES);
        glDrawTexiOES(w / 2, h / 2, 0, w / 2 , h / 2);
        eglSwapBuffers(dpy, surface);
    }
    return 0;
}