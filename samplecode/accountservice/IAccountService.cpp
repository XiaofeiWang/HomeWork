#include <stdint.h>
#include <sys/types.h>

#define LOG_TAG "AccountService"
#include <utils/Log.h>
#include <utils/CallStack.h>

#include <binder/Parcel.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>

#include <IAccountService.h>

using namespace android;

class BpAccountService: public BpInterface<IAccountService>
{
public:
    BpAccountService(const sp<IBinder>& impl) : BpInterface<IAccountService>(impl)
    {
        ALOGD("BpAccountService ctor");
        CallStack stack(LOG_TAG);
        stack.update();
        stack.dump(0);
    }

    virtual int32_t getlocked() {
        ALOGD("client getlocked");
        CallStack stack(LOG_TAG);
        stack.update();
        stack.dump(0);
        Parcel data, reply;
        data.writeInterfaceToken(IAccountService::getInterfaceDescriptor());
        status_t status = remote()->transact(BnAccountService::GET_LOCKED, data, &reply);
        if (status != NO_ERROR) {
            ALOGD("getlocked could not contact remote %d\n", status);
            return -1;
        }
        return reply.readInt32();
    }

    virtual int32_t setlocked(const int32_t& locked)
    {
        ALOGD("client setlocked: %d", locked);
        Parcel data, reply;
        data.writeInterfaceToken(IAccountService::getInterfaceDescriptor());
        data.writeInt32(locked);
        status_t status = remote()->transact(BnAccountService::SET_LOCKED, data, &reply);
        if (status != NO_ERROR) {
            ALOGD("getlocked could not contact remote %d\n", status);
            return -1;
        }
        return reply.readInt32();
    }
};

IMPLEMENT_META_INTERFACE(AccountService, "android.wangxiaofei.account");

status_t BnAccountService::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
        case GET_LOCKED:{
            ALOGD("remote GETLOCKED");
            CHECK_INTERFACE(IAccountService, data, reply);
            int32_t ret = getlocked();
            ALOGD("remote GETLOCKED: %d", ret);
            reply->writeInt32(ret);
            return NO_ERROR;
            } break;
        case SET_LOCKED: {
            ALOGD("remote SETLOCKED");
            CHECK_INTERFACE(IAccountService, data, reply);
            int32_t locked = data.readInt32();
            setlocked(locked);
            return NO_ERROR;
            } break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}
// ----------------------------------------------------------------------------

