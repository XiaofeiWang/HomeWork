#define LOG_NDDEBUG 0

#include <stdint.h>
#include <sys/types.h>
#include <stdio.h>

#include <binder/IServiceManager.h>

#include <utils/Log.h>
#include <IAccountService.h>

int main(int argc, char* argv[])
{
    ALOGD("AccountClient start");
    android::sp<android::IServiceManager> sm = android::defaultServiceManager();
    sp<IBinder> binder = sm->getService(android::String16("android.wangxiaofei.account"));
    sp<IAccountService> accountservice = interface_cast<IAccountService>(binder);
    accountservice->setlocked(10);
    return 0;
}
