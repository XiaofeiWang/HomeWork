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
    void setWeakRefA(const wwp<A> &a);
protected:
    virtual ~B();
private:
    wwp<A> mA;
};

B::B()
{
    ALOGD("B constructor");
}

B::~B()
{
    ALOGD("B deconstructor");
}

void B::setWeakRefA(const wwp<A> &a)
{
    mA = a;
}


int main(int /*argc*/, char** /*argv*/)
{
    A *pA = new A();
    B *pB = new B();
    wwp<A> wpA = pA;
    wsp<A> spA = wpA.promote();
    wsp<B> spB = pB;
    spA->setRefB(spB);
    spB->setWeakRefA(spA);
    return 0;
}










