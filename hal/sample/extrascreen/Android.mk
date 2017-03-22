LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := extrascreen.c
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_CFLAGS := $(common_flags) -DLOG_TAG=\"extra_screen\"
LOCAL_SHARED_LIBRARIES := liblog
LOCAL_MODULE := extrascreen.default
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

##main application
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	extrascreen_main.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libhardware

LOCAL_CFLAGS := $(common_flags) -DLOG_TAG=\"extra_screen\"

LOCAL_MODULE:= extrascreen

LOCAL_MODULE_TAGS := extrascreen_tag

LOCAL_CFLAGS += -Wall -Werror -Wunused -Wunreachable-code

include $(BUILD_EXECUTABLE)
