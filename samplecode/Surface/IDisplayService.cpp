#include <stdint.h>
#include <sys/types.h>

#define LOG_TAG "DisplayService"
#include <utils/Log.h>
#include <utils/CallStack.h>

#include <binder/Parcel.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>

#include <IDisplayService.h>

using namespace android;

class BpDisplayService: public BpInterface<IDisplayService>
{
public:
    BpDisplayService(const sp<IBinder>& impl) : BpInterface<IDisplayService>(impl)
    {
        ALOGD("BpDisplayService ctor");
    }

    virtual int32_t createDisplaySurface(const sp<IGraphicBufferProducer>& bufferProducer) {
        ALOGD("createDisplaySurface");
        Parcel data, reply;
        data.writeInterfaceToken(IDisplayService::getInterfaceDescriptor());
        data.writeStrongBinder(IGraphicBufferProducer::asBinder(bufferProducer));
        status_t status = remote()->transact(BnDisplayService::CREATE_DISPLAY_SURFACE, data, &reply);
        if (status != NO_ERROR) {
            ALOGD("createDisplaySurface failed: %d\n", status);
            return -1;
        }
        return reply.readInt32();
    }
};

IMPLEMENT_META_INTERFACE(DisplayService, "android.wangxiaofei.displayservice");

status_t BnDisplayService::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
        case CREATE_DISPLAY_SURFACE:{
            CHECK_INTERFACE(IDisplayService, data, reply);
            sp<IGraphicBufferProducer> graphicBufferProducer = interface_cast<IGraphicBufferProducer>(data.readStrongBinder());
            int32_t ret = createDisplaySurface(graphicBufferProducer);
            reply->writeInt32(ret);
            return NO_ERROR;
            } break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}
// ----------------------------------------------------------------------------

