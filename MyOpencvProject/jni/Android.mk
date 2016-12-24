LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
include D:/OpenCV-2.4.9-android-sdk/sdk/native/jni/OpenCV.mk  

#include ../../sdk/native/jni/OpenCV.mk

LOCAL_SRC_FILES  := ImageProc.cpp  
LOCAL_SRC_FILES  += Plate_Recognition.cpp
LOCAL_SRC_FILES  += Plate_Segment.cpp
LOCAL_SRC_FILES  += Plate.cpp
LOCAL_SRC_FILES  += camshift.cpp
LOCAL_SRC_FILES  += PreviceGray.cpp
LOCAL_SRC_FILES  += LK_OpenCV.cpp
LOCAL_SRC_FILES  += imageshow.cpp
LOCAL_SRC_FILES  += blend.cpp
LOCAL_SRC_FILES  += stitch.cpp
LOCAL_SRC_FILES  += stitching_detailed0.cpp
LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_MODULE     := imageproc
LOCAL_LDLIBS += -llog 
include $(BUILD_SHARED_LIBRARY)  