#ifndef IACCOUNT_SERVICE_H
#define IACCOUNT_SERVICE_H

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

using namespace android;

/*
 * This is the test account service
 */
class IAccountService: public IInterface {
public:
    enum {
        TEST = IBinder::FIRST_CALL_TRANSACTION + 0,
        GET_LOCKED = IBinder::FIRST_CALL_TRANSACTION + 1,
        SET_LOCKED = IBinder::FIRST_CALL_TRANSACTION + 2,
    };

    DECLARE_META_INTERFACE(AccountService);

    virtual int32_t getlocked() = 0;
    virtual int32_t setlocked(const int32_t& locked) = 0;
};


// ----------------------------------------------------------------------------
class BnAccountService:public BnInterface<IAccountService> {
public:
    virtual status_t onTransact(uint32_t code, const Parcel& data, Parcel* reply,
            uint32_t flags = 0);
};

#endif
