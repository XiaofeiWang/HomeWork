/*
 * Copyright 2013 The Android Open Source Project
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

#define LOG_NDEBUG 0
//#define LOG_TAG "SurfaceReader_main"
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <math.h>
#include <poll.h>
#include <pthread.h>
#include <stdlib.h>
#include <utils/Log.h>
#include <utils/misc.h>
#include <utils/List.h>
#include <utils/String8.h>

#include <cstdio>

#include <gui/BufferItemConsumer.h>
#include <gui/Surface.h>

#include <android_runtime/AndroidRuntime.h>
#include <android_runtime/android_view_Surface.h>

#include <jni.h>
#include <JNIHelp.h>

#include <stdint.h>
#include <inttypes.h>

#include <utils/CallStack.h>

#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include "IDisplayService.h"

#include <SkCanvas.h>
#include <SkGraphics.h>
#include <SkImageEncoder.h>
#include <SkTypeface.h>
#include <SkPaint.h>

// ----------------------------------------------------------------------------

using namespace android;

//awake message
static const char WAKE_MESSAGE = 'W';

static int mapPublicFormatToHalFormat(PublicFormat f) {
    switch(f) {
        case PublicFormat::JPEG:
        case PublicFormat::DEPTH_POINT_CLOUD:
            return HAL_PIXEL_FORMAT_BLOB;
        case PublicFormat::DEPTH16:
            return HAL_PIXEL_FORMAT_Y16;
        case PublicFormat::RAW_SENSOR:
            return HAL_PIXEL_FORMAT_RAW16;
        default:
            // Most formats map 1:1
            return static_cast<int>(f);
    }
}

static android_dataspace mapPublicFormatToHalDataspace(
        PublicFormat f) {
    switch(f) {
        case PublicFormat::JPEG:
            return HAL_DATASPACE_V0_JFIF;
        case PublicFormat::DEPTH_POINT_CLOUD:
        case PublicFormat::DEPTH16:
            return HAL_DATASPACE_DEPTH;
        case PublicFormat::RAW_SENSOR:
        case PublicFormat::RAW_PRIVATE:
        case PublicFormat::RAW10:
        case PublicFormat::RAW12:
            return HAL_DATASPACE_ARBITRARY;
        case PublicFormat::YUV_420_888:
        case PublicFormat::NV21:
        case PublicFormat::YV12:
            return HAL_DATASPACE_V0_JFIF;
        default:
            // Most formats map to UNKNOWN
            return HAL_DATASPACE_UNKNOWN;
    }
}

// Get an ID that's unique within this process.
static int32_t createProcessUniqueId() {
    static volatile int32_t globalCounter = 0;
    return android_atomic_inc(&globalCounter);
}

// ----------------------------------------------------------------------------

class SurfaceReaderContext : public ConsumerBase::FrameAvailableListener
{
public:
    SurfaceReaderContext(int maxImages);

    virtual ~SurfaceReaderContext();

    virtual void onFrameAvailable(const BufferItem& item);

    BufferItem* getBufferItem();
    void returnBufferItem(BufferItem* buffer);


    void setBufferConsumer(const sp<BufferItemConsumer>& consumer) { mConsumer = consumer; }
    BufferItemConsumer* getBufferConsumer() { return mConsumer.get(); }

    void setProducer(const sp<IGraphicBufferProducer>& producer) { mProducer = producer; }
    IGraphicBufferProducer* getProducer() { return mProducer.get(); }

    void setBufferFormat(int format) { mFormat = format; }
    int getBufferFormat() { return mFormat; }

    void setBufferDataspace(android_dataspace dataSpace) { mDataSpace = dataSpace; }
    android_dataspace getBufferDataspace() { return mDataSpace; }

    void setBufferWidth(int width) { mWidth = width; }
    int getBufferWidth() { return mWidth; }

    void setBufferHeight(int height) { mHeight = height; }
    int getBufferHeight() { return mHeight; }

    bool isFormatOpaque(int format);
    void setWritePipeFd(int writePipeFd);
private:

    List<BufferItem*> mBuffers;
    sp<BufferItemConsumer> mConsumer;
    sp<IGraphicBufferProducer> mProducer;
    int mFormat;
    android_dataspace mDataSpace;
    int mWidth;
    int mHeight;
    int mWritePipeFd;
};

SurfaceReaderContext::SurfaceReaderContext(int maxImages) :
    mFormat(0),
    mDataSpace(HAL_DATASPACE_UNKNOWN),
    mWidth(-1),
    mHeight(-1),
    mWritePipeFd(-1) {
    for (int i = 0; i < maxImages; i++) {
        BufferItem* buffer = new BufferItem;
        mBuffers.push_back(buffer);
    }
}

SurfaceReaderContext::~SurfaceReaderContext()
{
    for (List<BufferItem *>::iterator it = mBuffers.begin();
            it != mBuffers.end(); it++) {
        delete *it;
    }

    if (mConsumer != 0) {
        mConsumer.clear();
    }
}

bool SurfaceReaderContext::isFormatOpaque(int format)
{
    return format == HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED;
}

BufferItem* SurfaceReaderContext::getBufferItem() {
    if (mBuffers.empty()) {
        return NULL;
    }
    // Return a BufferItem pointer and remove it from the list
    List<BufferItem*>::iterator it = mBuffers.begin();
    BufferItem* buffer = *it;
    mBuffers.erase(it);
    return buffer;
}

void SurfaceReaderContext::returnBufferItem(BufferItem* buffer) {
    mBuffers.push_back(buffer);
}

void SurfaceReaderContext::onFrameAvailable(const BufferItem& /*item*/)
{
    if (mWritePipeFd >= 0) {
        const char wakeMessage(WAKE_MESSAGE);
        int result = write(mWritePipeFd, &wakeMessage, 1);
        ALOGE_IF(result<0, "error sending wake message (%s)", strerror(errno));
    }
}

void SurfaceReaderContext::setWritePipeFd(int writePipeFd)
{
    mWritePipeFd = writePipeFd;
}

// ----------------------------------------------------------------------------
class SurfaceReader : public Thread {
public:
    SurfaceReader(int width, int height, int format, int maxImages);
    virtual ~SurfaceReader();
    virtual bool threadLoop();
    inline IGraphicBufferProducer* getProducer() { return mSurfaceReaderContext->getProducer(); }
    void close();
    void startReader();
private:
    enum {
        wake            = 0,
        numFds,
    };

    struct pollfd mPollFds[numFds];
    int mWritePipeFd;
    sp<SurfaceReaderContext> mSurfaceReaderContext;

};


SurfaceReader::SurfaceReader(int width, int height, int format, int maxImages)
{
    status_t res;
    int nativeFormat;
    android_dataspace nativeDataspace;

    int wakeFds[2];
    int result = pipe(wakeFds);
    ALOGE_IF(result<0, "error creating wake pipe (%s)", strerror(errno));
    fcntl(wakeFds[0], F_SETFL, O_NONBLOCK);
    fcntl(wakeFds[1], F_SETFL, O_NONBLOCK);
    mWritePipeFd = wakeFds[1];

    mPollFds[wake].fd = wakeFds[0];
    mPollFds[wake].events = POLLIN;
    mPollFds[wake].revents = 0;

    ALOGV("%s: width:%d, height: %d, format: 0x%x, maxImages:%d",
          __FUNCTION__, width, height, format, maxImages);

    PublicFormat publicFormat = static_cast<PublicFormat>(format);
    nativeFormat = mapPublicFormatToHalFormat(
        publicFormat);
    nativeDataspace = mapPublicFormatToHalDataspace(
        publicFormat);

    mSurfaceReaderContext = new SurfaceReaderContext(maxImages);
    mSurfaceReaderContext->setWritePipeFd(mWritePipeFd);

    sp<IGraphicBufferProducer> gbProducer;
    sp<IGraphicBufferConsumer> gbConsumer;
    BufferQueue::createBufferQueue(&gbProducer, &gbConsumer);
    sp<BufferItemConsumer> bufferConsumer;
    String8 consumerName = String8::format("SurfaceReader-%dx%df%xm%d-%d-%d",
            width, height, format, maxImages, getpid(),
            createProcessUniqueId());
    uint32_t consumerUsage = GRALLOC_USAGE_SW_READ_OFTEN;

    if (mSurfaceReaderContext->isFormatOpaque(nativeFormat)) {
        // Use the SW_READ_NEVER usage to tell producer that this format is not for preview or video
        // encoding. The only possibility will be ZSL output.
        consumerUsage = GRALLOC_USAGE_SW_READ_NEVER;
    }
    bufferConsumer = new BufferItemConsumer(gbConsumer, consumerUsage, maxImages,
            /*controlledByApp*/true);
    if (bufferConsumer == nullptr) {
        ALOGD("ctor BufferItemConsumer failed");
        return;
    }
    mSurfaceReaderContext->setBufferConsumer(bufferConsumer);
    bufferConsumer->setName(consumerName);

    mSurfaceReaderContext->setProducer(gbProducer);
    bufferConsumer->setFrameAvailableListener(mSurfaceReaderContext);
    
    mSurfaceReaderContext->setBufferFormat(nativeFormat);
    mSurfaceReaderContext->setBufferDataspace(nativeDataspace);
    mSurfaceReaderContext->setBufferWidth(width);
    mSurfaceReaderContext->setBufferHeight(height);

    // Set the width/height/format/dataspace to the bufferConsumer.
    res = bufferConsumer->setDefaultBufferSize(width, height);
    if (res != OK) {
        ALOGD("Failed to set buffer consumer default size (%dx%d) for format 0x%x",  width, height, nativeFormat);
        return;
    }
    res = bufferConsumer->setDefaultBufferFormat(nativeFormat);
    if (res != OK) {
        ALOGD("Failed to set buffer consumer default format 0x%x", nativeFormat);
    }
    res = bufferConsumer->setDefaultBufferDataSpace(nativeDataspace);
    if (res != OK) {
        ALOGD("Failed to set buffer consumer default dataSpace 0x%x", nativeDataspace);
    }
}

SurfaceReader::~SurfaceReader()
{
    ::close(mPollFds[wake].fd);
    ::close(mWritePipeFd);
}

bool SurfaceReader::threadLoop()
{
    while (1) {
        int n = poll(mPollFds, numFds, -1);
        if (mPollFds[wake].revents & POLLIN) {
            char msg(WAKE_MESSAGE);
            int result = read(mPollFds[wake].fd, &msg, 1);
            ALOGE_IF(result<0, "error reading from wake pipe (%s)", strerror(errno));
            ALOGE_IF(msg != WAKE_MESSAGE, "unknown message on wake queue (0x%02x)", int(msg));
            mPollFds[wake].revents = 0;

            BufferItemConsumer* bufferConsumer = mSurfaceReaderContext->getBufferConsumer();
            BufferItem* buffer = mSurfaceReaderContext->getBufferItem();
            if (buffer == NULL) {
                ALOGW("Unable to acquire a buffer item");
                break;
            }
            status_t res = bufferConsumer->acquireBuffer(buffer, 0);
            if (res != OK) {
                ALOGD("acquireBuffer is not ok");
                mSurfaceReaderContext->returnBufferItem(buffer);
            } else {
                ALOGD("acquireBuffer is ok, success!");
                sp<Fence> releaseFence = Fence::NO_FENCE;
                SkBitmap bitmap;
                bitmap.allocPixels(SkImageInfo::Make(240, 320, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
                void* dst = bitmap.getPixels();
                ALOGD("dst is null: %d", (dst == NULL));
                if (NULL != dst) {
                    void* pData = NULL;
                    Rect rect;
                    res = buffer->mGraphicBuffer->lockAsync(GRALLOC_USAGE_SW_READ_OFTEN, buffer->mCrop, &pData, buffer->mFence->dup());
                    if (res != OK) {
                        ALOGE("Lock buffer failed!");
                        break;
                    }
                    ALOGD("bitmap size: %d", bitmap.getSize());
                    memcpy(dst, pData, bitmap.getSize());
                    bitmap.notifyPixelsChanged();
                    SkImageEncoder::EncodeFile("/data/local/tmp/foo.png", bitmap, SkImageEncoder::kPNG_Type, 100);
                    int fenceFd;
                    res = buffer->mGraphicBuffer->unlockAsync(&fenceFd);
                    if (res != OK) {
                        ALOGD("Graphic buffer unlock failed");
                    } else {
                        releaseFence = new Fence(fenceFd);
                    }
                }
                //bitmap.freePixels();
                bufferConsumer->releaseBuffer(*buffer, releaseFence);
                mSurfaceReaderContext->returnBufferItem(buffer);
            }
        }
    }
    return false;
}

void SurfaceReader::close()
{
    ALOGV("%s:", __FUNCTION__);

    BufferItemConsumer* consumer = mSurfaceReaderContext->getBufferConsumer();

    if (consumer != NULL) {
        consumer->abandon();
        consumer->setFrameAvailableListener(NULL);
    }
}

void SurfaceReader::startReader()
{
    ALOGV("1. %s:", __FUNCTION__);
    android::sp<android::IServiceManager> sm = android::defaultServiceManager();
    sp<IBinder> binder = sm->getService(android::String16("android.wangxiaofei.displayservice"));
    ALOGV("2. %s:", __FUNCTION__);
    sp<IDisplayService> displayservice = interface_cast<IDisplayService>(binder);
    ALOGV("3. %s:", __FUNCTION__);
    displayservice->createDisplaySurface(this->getProducer());
    run("SurfaceReader", PRIORITY_URGENT_DISPLAY);
}
int main(int argc, char *argv[])
{
    sp<SurfaceReader> surfaceReader = new SurfaceReader(240, 320,HAL_PIXEL_FORMAT_BGRA_8888,10);
    surfaceReader->startReader();
    IPCThreadState::self()->joinThreadPool();
    return 0;
}
// ----------------------------------------------------------------------------   

