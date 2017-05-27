#define LOG_NDDEBUG 0

#include <cutils/memory.h>
#include <binder/Parcel.h>
#include <utils/Log.h>
#include <DisplayService.h>
#include <gui/Surface.h>
#include <android/native_window.h>

using namespace android;

DisplayService::DisplayService()
{
}

int32_t DisplayService::createDisplaySurface(const sp<IGraphicBufferProducer>& bufferProducer)
{
    mBufferProducer =  bufferProducer;
    mSurface = new Surface(bufferProducer, true);
    run("DisplayService", PRIORITY_URGENT_DISPLAY);
    return NO_ERROR;
}

bool DisplayService::threadLoop() {
    ALOGD("DisplayService thread starting...");
    ANativeWindow_Buffer outBuffer;
    while (1) {
        mSurface->lock(&outBuffer, NULL);
        ssize_t bpr = outBuffer.stride * bytesPerPixel(outBuffer.format);
        android_memset16((uint16_t*)outBuffer.bits, 0xF800, bpr*outBuffer.height);
        mSurface->unlockAndPost();
        sleep(1);
    }
    return false;
}

