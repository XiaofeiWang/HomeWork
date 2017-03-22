#ifndef ANDROID_INCLUDE_HARDWARE_EXTRASCREEN_H
#define ANDROID_INCLUDE_HARDWARE_EXTRASCREEN_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include <hardware/hardware.h>


__BEGIN_DECLS

#define EXTRASCREEN_API_VERSION HARDWARE_MODULE_API_VERSION(1,0)

/*
 *the main device id
 */
#define EXTRASCREEN_DEVICE_ID_MAIN "main_extrascreen"

/*
 * The id of this module
 */
#define EXTRASCREEN_HARDWARE_MODULE_ID "extrascreen"

typedef struct extrascreen_device {
    struct hw_device_t common;
    /*
    **start extra screen to show
    */
    void (*extrascreen_on)(struct extrascreen_device *dev);

    /*
    **off the extra screen to show
    */
    void (*extrascreen_off)(struct extrascreen_device *dev);
} extrascreen_device_t;

static inline int extrascreen_open(const struct hw_module_t* module, extrascreen_device_t** device)
{
    return module->methods->open(module, EXTRASCREEN_DEVICE_ID_MAIN, (struct hw_device_t**)device);
}

__END_DECLS
#endif
