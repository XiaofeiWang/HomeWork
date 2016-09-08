#####################################################################
# build executable
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	WRefBase.cpp \
	mywspwwptest_main.cpp \


LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	liblog \
	libbinder \
	libui \
	libgui

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE:= lightrefbasetest

include $(BUILD_EXECUTABLE)
