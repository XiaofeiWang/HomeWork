#define NDEBUG 0
#define LOG_TAG "MyRefBase"

#include <utils/Log.h>

#include "WRefBase.h"
#include "WStrongPointer.h"

using namespace android;

class MyRefBase : public WRefBase {
public:
    MyRefBase();
protected:
    virtual ~MyRefBase();
};

MyRefBase::MyRefBase()
{
    ALOGD("MyRefbase constructor");
}

MyRefBase::~MyRefBase()
{
    ALOGD("MyRefbase deconstructor");
}

int main(int /*argc*/, char** /*argv*/)
{
    MyRefBase *myRefBase = new MyRefBase();
    wsp<MyRefBase> spRef = myRefBase;
    wwp<MyRefBase> wpRef = myRefBase;
    {
        wsp<MyRefBase> spRefInner = spRef;
    }
}










