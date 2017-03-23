LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := AccountService.cpp IAccountService.cpp

LOCAL_CFLAGS:= -DLOG_TAG=\"AccountService\"

LOCAL_SHARED_LIBRARIES := libbinder libutils libcutils liblog

LOCAL_MODULE := libaccountservice

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)


#build accountservie's executable
###############################################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES := accountservice_main.cpp

LOCAL_CFLAGS:= -DLOG_TAG=\"AccountService\"

LOCAL_SHARED_LIBRARIES := libbinder \
		libutils \
		libcutils \
		liblog \
		libaccountservice

LOCAL_MODULE := accountservice

LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

#build accountclient's executable
###############################################################
include $(CLEAR_VARS)

LOCAL_SRC_FILES := accountclient_main.cpp

LOCAL_CFLAGS:= -DLOG_TAG=\"AccountService\"

LOCAL_SHARED_LIBRARIES := libbinder \
		libutils \
		libcutils \
		liblog \
		libaccountservice

LOCAL_MODULE := accountclient

LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

