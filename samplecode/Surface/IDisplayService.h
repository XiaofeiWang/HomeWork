#ifndef IACCOUNT_SERVICE_H
#define IACCOUNT_SERVICE_H

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

#include <gui/IGraphicBufferProducer.h>

using namespace android;

/*
 * This is the test account service
 */
class IDisplayService : public IInterface {
public:
    enum {
        CREATE_DISPLAY_SURFACE = IBinder::FIRST_CALL_TRANSACTION,
    };

    DECLARE_META_INTERFACE(DisplayService);

    virtual int32_t createDisplaySurface(const sp<IGraphicBufferProducer>& bufferProducer) = 0;
};


// ----------------------------------------------------------------------------
class BnDisplayService : public BnInterface<IDisplayService> {
public:
    virtual status_t onTransact(uint32_t code, const Parcel& data, Parcel* reply,
            uint32_t flags = 0);
};

#endif
