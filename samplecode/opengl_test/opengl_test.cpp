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

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES/gl.h>
#include <GLES/glext.h>

#include <utils/Timers.h>

#include <ui/GraphicBuffer.h>
#include <EGLUtils.h>
#include <gui/SurfaceControl.h>
#include <EGL/egl.h>

using namespace android;

const int yuvTexWidth = 600;
const int yuvTexHeight = 480;
const int yuvTexUsage = GraphicBuffer::USAGE_HW_TEXTURE |
        GraphicBuffer::USAGE_SW_WRITE_RARELY;
const int yuvTexFormat = HAL_PIXEL_FORMAT_YV12;


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
    GLuint texture;
    EGLint configAttribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
            EGL_NONE };

    sp<SurfaceComposerClient> surfaceComposerClient = new SurfaceComposerClient;
    err = surfaceComposerClient->initCheck();
    if (err != NO_ERROR) {
        fprintf(stderr, "SurfaceComposerClient::initCheck error: %#x\n", err);
        return;
    }

    // Get main display parameters.
    sp<IBinder> mainDpy = SurfaceComposerClient::getBuiltInDisplay(
            ISurfaceComposer::eDisplayIdMain);
    DisplayInfo mainDpyInfo;
    err = SurfaceComposerClient::getDisplayInfo(mainDpy, &mainDpyInfo);
    if (err != NO_ERROR) {
        fprintf(stderr, "ERROR: unable to get display characteristics\n");
        return;
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
        return;
    }

    SurfaceComposerClient::openGlobalTransaction();
    err = surfaceControl->setLayer(0x7FFFFFFF);     // always on top
    if (err != NO_ERROR) {
        fprintf(stderr, "SurfaceComposer::setLayer error: %#x\n", err);
        return;
    }

    err = surfaceControl->show();
    if (err != NO_ERROR) {
        fprintf(stderr, "SurfaceComposer::show error: %#x\n", err);
        return;
    }
    SurfaceComposerClient::closeGlobalTransaction();

    dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(dpy, &majorVersion, &minorVersion);

    status_t err = EGLUtils::selectConfigForNativeWindow(
            dpy, configAttribs, (EGLNativeWindowType)surfaceControl.getSurface(), &config);
    if (err) {
        fprintf(stderr, "couldn't find an EGLConfig matching the screen format\n");
        return 0;
    }
    surface = eglCreateWindowSurface(dpy, config, window, NULL);
    context = eglCreateContext(dpy, config, NULL, NULL);
    eglMakeCurrent(dpy, surface, surface, context);   
    eglQuerySurface(dpy, surface, EGL_WIDTH, &w);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &h);

    printf("w=%d, h=%d\n", w, h);
    glBindTexture(GL_TEXTURE_2D, 0);
    glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    GraphicBuffer graphicBuffer1 = new GraphicBuffer(w, h, HAL_PIXEL_FORMAT_RGBA_8888,
             GraphicBuffer::USAGE_HW_TEXTURE | GraphicBuffer::USAGE_SW_WRITE_RARELY);
    uint32_t* bits;
    graphicBuffer1->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)(&bits));
    ssize_t bpr = outBuffer.stride * bytesPerPixel(graphicBuffer1.format);
    android_memset16((uint16_t*)outBuffer.bits, 0xF800, bpr*graphicBuffer1.height);
    err = graphicBuffer1->unlock();
    if (err != 0) {
        fprintf(stderr, "graphicBuffer1->unlock() failed: %d\n", err);
        return 0;
    }

    EGLClientBuffer clientBuffer = (EGLClientBuffer)graphicBuffer1->getNativeBuffer();
    EGLImageKHR img = eglCreateImageKHR(dpy, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID,
            clientBuffer, 0);
    if (img == EGL_NO_IMAGE_KHR) {
        return false;
    }
    glGenTextures(1, &texture);
    
}