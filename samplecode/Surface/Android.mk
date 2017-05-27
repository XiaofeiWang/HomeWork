LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := DisplayService.cpp IDisplayService.cpp

LOCAL_CFLAGS:= -DLOG_TAG=\"DisplayService\"

LOCAL_SHARED_LIBRARIES := libbinder \
	libcutils \
	libui \
	libgui \
	libutils \
	liblog

LOCAL_MODULE := libdisplayservice

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)


#build displayservie's executable
###############################################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES := displayservice_main.cpp

LOCAL_CFLAGS:= -DLOG_TAG=\"DisplayService\"

LOCAL_SHARED_LIBRARIES := libbinder \
		libutils \
		libcutils \
		liblog \
		libdisplayservice

LOCAL_MODULE := displayservice

LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

#build surface reader executable
###############################################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES := surfacereader_main.cpp

LOCAL_CFLAGS:= -DLOG_TAG=\"DisplayService\"

LOCAL_SHARED_LIBRARIES := libbinder \
		libutils \
		libcutils \
		liblog \
		libui \
		libgui \
		libskia \
		libdisplayservice

LOCAL_MODULE := surfacereader

LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)
