#define LOG_NDDEBUG 0

#include <binder/Parcel.h>
#include <utils/Log.h>
#include <IDisplayService.h>
#include <utils/threads.h>

namespace android {
class DisplayService : public BnDisplayService, protected Thread {
public:
    DisplayService();
    virtual int32_t createDisplaySurface(const sp<IGraphicBufferProducer>& bufferProducer);

private:
    virtual bool threadLoop();
    sp<IGraphicBufferProducer> mBufferProducer;
    sp<Surface> mSurface;
};


};// namespace android
