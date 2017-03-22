#include <utils/misc.h>
#include <utils/Log.h>
#include <hardware/hardware.h>
#include <hardware/extrascreen.h>

#include <stdio.h>

int main(int /*argc*/, char** /*argv*/)
{
    int err;
    hw_module_t* module;

    ALOGD("extrascreen main start");
    err = hw_get_module(EXTRASCREEN_HARDWARE_MODULE_ID, (hw_module_t const**)&module);
    if (err == 0) {
        hw_device_t* device;
        err = module->methods->open(module, EXTRASCREEN_HARDWARE_MODULE_ID, &device);
        if (err == 0) {
            extrascreen_device_t* myDevice = (extrascreen_device_t*)device;
            myDevice->extrascreen_on(myDevice);
            myDevice->extrascreen_off(myDevice);
        } else {
            ALOGE("main open devices error, pls check");
        }
    } else {
        ALOGE("main hw_get_module error, pls check");
    }
    return 0;
}
