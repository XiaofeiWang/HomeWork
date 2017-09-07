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
    sp<IBinder> binder = sm->getService(android::String16("accountservice"));
    sp<IAccountService> accountservice = interface_cast<IAccountService>(binder);
    ALOGD("AccountClient accountservice->setlocked(10): %d",accountservice->setlocked(10));
    ALOGD("AccountClient accountservice->getlocked(): %d",accountservice->getlocked());
    return 0;
}
