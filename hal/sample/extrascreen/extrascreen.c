#include <cutils/log.h>
#include <malloc.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>

#include <hardware/hardware.h>
#include "hardware/extrascreen.h"

static const char THE_DEVICE[] = "/sys/class/extra_screen/enable";

/*
**start extra screen to show
*/
static void screen_on(extrascreen_device_t *dev)
{
    int fd;
    fd = open(THE_DEVICE, O_WRONLY);
    if (fd >= 0) {
        char buffer[] = "1";
        write(fd, buffer, 1);
    } else {
        ALOGE("screen_on failed because of open devices error: %s", strerror(errno));
    }
}

/*
**off the extra screen to show
*/
static void screen_off(extrascreen_device_t *dev)
{
    int fd;
    fd = open(THE_DEVICE, O_WRONLY);
    if (fd >= 0) {
        char buffer[] = "0";
        write(fd, buffer, 1);
    } else {
        ALOGE("screen_off failed because of open devices error: %s", strerror(errno));
    }
}

static int screen_exists() {
    int fd;

    fd = TEMP_FAILURE_RETRY(open(THE_DEVICE, O_RDWR));
    if(fd < 0) {
        ALOGE("Extrascreen file does not exist : %d", fd);
        return 0;
    }

    close(fd);
    return 1;
}

static int screen_close(hw_device_t *device)
{
    free(device);
        return 0;
}

static int screen_open(const struct hw_module_t* module, const char* id,
         struct hw_device_t** device)
{
    ALOGD("screen_open");
    if (!screen_exists()) {
        ALOGE("extrascreen does not exist,pls check!");
    }

    extrascreen_device_t *screenDev = calloc(1, sizeof(extrascreen_device_t));
    if (!screenDev) {
        ALOGE("Can not allocate momory for the extrascreen device");
        return -ENOMEM;
    }

    screenDev->common.tag = HARDWARE_DEVICE_TAG;
    screenDev->common.module = (hw_module_t *) module;;
    screenDev->common.version = HARDWARE_DEVICE_API_VERSION(1,0);
    screenDev->common.close = screen_close;

    screenDev->extrascreen_on = screen_on;
    screenDev->extrascreen_off = screen_off;

    *device = (hw_device_t *) screenDev;
    return 0;
}

static struct hw_module_methods_t extrascreen_module_methods = {
    .open = screen_open,
};

struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .module_api_version = EXTRASCREEN_API_VERSION, 
    .hal_api_version = HARDWARE_HAL_API_VERSION,
    .id = EXTRASCREEN_HARDWARE_MODULE_ID,
    .name = "Default extrascreen HAL",
    .author = "The Android Open Source Project",
    .methods = &extrascreen_module_methods,
};
