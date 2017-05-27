LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := samples tests

# Only compile source java files in this apk.
LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_JAVA_LIBRARIES := telephony-common

LOCAL_STATIC_JAVA_LIBRARIES = android-support-v4

LOCAL_PACKAGE_NAME := WXFApiDemos

LOCAL_SDK_VERSION := current

include $(BUILD_PACKAGE)
