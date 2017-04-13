LOCAL_PATH := $(call my-dir)

#build accountservie's executable
###############################################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES := loopertest_main.cpp

LOCAL_CFLAGS:= -DLOG_TAG=\"LooperTest\"

LOCAL_SHARED_LIBRARIES := libbinder \
		libutils \
		libcutils \
		libgui \
		liblog

LOCAL_MODULE := LooperTest

LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

