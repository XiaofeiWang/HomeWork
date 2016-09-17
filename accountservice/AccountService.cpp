#define LOG_NDDEBUG 0

#include <binder/Parcel.h>
#include <utils/Log.h>
#include <AccountService.h>

using namespace android;

AccountService::AccountService() : mLocked(0)
{
}

int32_t AccountService::getlocked()
{
    return mLocked;
}

int32_t AccountService::setlocked(const int32_t& locked)
{
    mLocked = locked;
    return NO_ERROR;
}
