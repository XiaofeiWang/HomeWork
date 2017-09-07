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
    }

    virtual int32_t getlocked() {
        ALOGD("client getlocked");
        Parcel data, reply;
        data.writeInterfaceToken(IAccountService::getInterfaceDescriptor());
        remote()->transact(BnAccountService::GET_LOCKED, data, &reply);
        int32_t ret = reply.readExceptionCode();
        if (ret != 0) {
            ALOGD("getlocked could not contact remote %d\n", ret);
            return ret;
        }
        return reply.readInt32();
    }

    virtual int32_t setlocked(const int32_t& locked)
    {
        ALOGD("client setlocked: %d", locked);
        Parcel data, reply;
        data.writeInterfaceToken(IAccountService::getInterfaceDescriptor());
        data.writeInt32(locked);
        remote()->transact(BnAccountService::SET_LOCKED, data, &reply);
        int32_t ret = reply.readExceptionCode();
        if (ret != 0) {
            ALOGD("setlocked could not contact remote %d\n", ret);
            return ret;
        }
        ret = reply.readInt32();
        return ret;
    }
};

IMPLEMENT_META_INTERFACE(AccountService, "android.wangxiaofei.IAccountService");

status_t BnAccountService::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
        case GET_LOCKED:{
            ALOGD("remote GETLOCKED");
            CHECK_INTERFACE(IAccountService, data, reply);
            int32_t locked = getlocked();
            ALOGD("remote GETLOCKED: %d", locked);
            reply->writeNoException();
            reply->writeInt32(locked);
            return OK;
            } break;
        case SET_LOCKED: {
            ALOGD("remote SETLOCKED");
            CHECK_INTERFACE(IAccountService, data, reply);
            int32_t locked = data.readInt32();
            int32_t ret = setlocked(locked);
            reply->writeNoException();
            reply->writeInt32(ret);
            return OK;
            } break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}
// ----------------------------------------------------------------------------

