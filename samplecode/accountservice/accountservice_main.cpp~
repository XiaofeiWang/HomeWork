#define LOG_NDDEBUG 0


#include <stdint.h>
#include <sys/types.h>
#include <stdio.h>

#include <binder/Parcel.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>

#include <utils/Log.h>
#include <AccountService.h>

int main(int argc, char* argv[])
{
    ALOGD("AccountService start");
    android::sp<android::IServiceManager> sm = android::defaultServiceManager();
    android::sp<android::AccountService> accountService = new android::AccountService();
    android::status_t ret = sm->addService(android::String16("android.wangxiaofei.account"), accountService);
    if (ret != android::OK) {
        ALOGD("Could not register AccountService !");
        return -1;
    }

    LOG_ALWAYS_FATAL("FATAL AccountService");
    ALOGD("join AccountService Thread");
    /*
     * We're the only thread in existence, so we're just going to process
     * Binder transaction as a single-threaded program.
     */
    android::IPCThreadState::self()->joinThreadPool();
    ALOGD("AccountService Thread exit");
    return 0;
}
