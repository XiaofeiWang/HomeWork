LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	resize.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libskia \
	libutils \
    libui \
    libbinder \
    libgui

LOCAL_MODULE:= test-resize

LOCAL_MODULE_TAGS := tests

LOCAL_CFLAGS += -Wall -Werror -Wunused -Wunreachable-code

include $(BUILD_EXECUTABLE)
