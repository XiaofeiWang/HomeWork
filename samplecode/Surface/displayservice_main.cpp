#define LOG_NDDEBUG 0


#include <stdint.h>
#include <sys/types.h>
#include <stdio.h>

#include <binder/Parcel.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>

#include <utils/Log.h>
#include <DisplayService.h>

int main(int argc, char* argv[])
{
    ALOGD("DisplayService start");
    android::sp<android::IServiceManager> sm = android::defaultServiceManager();
    android::sp<android::DisplayService> displayService = new android::DisplayService();
    android::status_t ret = sm->addService(android::String16("android.wangxiaofei.displayservice"), displayService);
    if (ret != android::OK) {
        ALOGD("Could not register DisplayService !");
        return -1;
    }

    ALOGD("join DisplayService Thread");
    /*
     * We're the only thread in existence, so we're just going to process
     * Binder transaction as a single-threaded program.
     */
    android::IPCThreadState::self()->joinThreadPool();
    ALOGD("DisplayService Thread exit");
    return 0;
}
