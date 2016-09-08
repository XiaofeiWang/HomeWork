#define NDEBUG 0
#define LOG_TAG "MyRefBase"

#include <utils/Log.h>

#include "WRefBase.h"
#include "WStrongPointer.h"

using namespace android;

class B;

class A : public WRefBase {
public:
    A();
    void setRefB(const wsp<B> &b);
protected:
    virtual ~A();
private:
    wsp<B> mB;
};

A::A()
{
    ALOGD("A constructor");
}

void A::setRefB(const wsp<B> &b)
{
    mB = b;
}

A::~A()
{
    ALOGD("A deconstructor");
}

class B : public WRefBase {
public:
    B();
    void setRefA(const wsp<A> &a);
protected:
    virtual ~B();
private:
    wsp<A> mA;
};

B::B()
{
    ALOGD("B constructor");
}

B::~B()
{
    ALOGD("B deconstructor");
}

void B::setRefA(const wsp<A> &a)
{
    mA = a;
}


int main(int /*argc*/, char** /*argv*/)
{
    A *pA = new A();
    B *pB = new B();
    wsp<A> spA = pA;
    wsp<B> spB = pB;
    spA->setRefB(spB);
    spB->setRefA(spA);
    return 0;
}










