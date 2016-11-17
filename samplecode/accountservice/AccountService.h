#define LOG_NDDEBUG 0

#include <binder/Parcel.h>
#include <utils/Log.h>
#include <IAccountService.h>

namespace android {
class AccountService : public BnAccountService {
public:
    AccountService();

    virtual int32_t getlocked();
    virtual int32_t setlocked(const int32_t& locked);

private:
    int32_t mLocked;
};
};// namespace android
