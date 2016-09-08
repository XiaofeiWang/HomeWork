#define NDEBUG 0
#define LOG_TAG "LightClass"

#include <utils/Log.h>

#include "RefBase.h"

using namespace android;

class LightClass : public WLightRefBase<LightClass> {
public:
    LightClass()
    {
        ALOGD("Constructor LightClass object ");
    }

    virtual ~LightClass()
    {
        ALOGD("Deconstructor LightClass object");
    }
};


int main(int /*argc*/, char** /*argv*/)
{
    LightClass *pLightClass = new LightClass();
    sp<LightClass> spRef = pLightClass;
    ALOGD("LightClass ojbect ref count: %d", pLightClass->getStrongCount());
    {
        sp<LightClass> spRefInner = spRef;
        ALOGD("LightClass ojbect ref count: %d", pLightClass->getStrongCount());
    }
    ALOGD("LightClass ojbect ref count: %d", pLightClass->getStrongCount());
}


