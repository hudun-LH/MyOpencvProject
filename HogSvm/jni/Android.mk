LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
include D:/OpenCV-2.4.9-android-sdk/sdk/native/jni/OpenCV.mk  

#LOCAL_LDLIBS+= -L$(SYSROOT)/usr/lib -llog
LOCAL_LDLIBS += -llog 
LOCAL_MODULE    := hogsvm
LOCAL_SRC_FILES := hogsvmt.cpp hogsvm.cpp tempray.cpp

include $(BUILD_SHARED_LIBRARY)
