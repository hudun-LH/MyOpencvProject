#include <iostream>
#include <fstream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/stitching/stitcher.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/imgproc/types_c.h"
#include "opencv2/imgproc/imgproc_c.h"
#include <opencv2/objdetect/objdetect.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/video/video.hpp"
#include <opencv2/ml/ml.hpp>

#include <android/log.h>
#include <string>
#include <jni.h>

#define LOG_TAG "libstitching"
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))

using namespace std;
using namespace cv;

#ifdef __cplusplus
extern "C" {
#endif

bool try_use_gpu = false;

JNIEXPORT jlong JNICALL Java_com_example_carplate_CarPlateDetection_doStitching
  (JNIEnv *env, jclass obj, jlong imageGray, jlong tempGray)
{
	LOGD("           doStitching ss         ");
	LOGD("           doStitching ss        ");

	//∂¡»ÎÕºœÒ
	//Mat img1 =  Mat(*(Mat*)imageGray);
	//Mat img2 = Mat(*(Mat*)tempGray);
	Mat img1 = imread("/storage/emulated/0/one.jpg"); //Mat(*(Mat*)imageGray);
	Mat img2 = imread("/storage/emulated/0/two.jpg"); //Mat(*(Mat*)tempGray);

	vector<Mat> imgs;

	LOGD("           doStitching img1 %d  %d    ",img1.type(), img1.channels());
	LOGD("           doStitching img2 %d  %d    ",img2.type(), img2.channels());

	//cvtColor(img1,img1,CV_BGR2GRAY);  //CV_BGRA2BGR
	//cvtColor(img2,img2,CV_BGR2GRAY);  //CV_BGRA2BGR

	//Mat input1 = Mat(img1.size(), CV_8UC1); //CV_8UC3
	//cvtColor(img1,input1,CV_BGR2GRAY);

	//Mat input2 = Mat(img2.size(), CV_8UC1);
	//cvtColor(img2,input2,CV_BGR2GRAY);//CV_BGRA2BGR
	LOGD("           doStitching imgs %d  %d  %d  ",imgs.size(), img1.channels(), img2.channels());

	imgs.push_back(img1);
	imgs.push_back(img2);

	LOGD("           doStitching imgs %d  %d  %d  ",imgs.size(), img1.channels(), img2.channels());

    Mat pano;//(img1.size(), CV_8UC1);
    //Status stitch(InputArray images, OutputArray pano);
    //Status stitch(InputArray images, const std::vector<std::vector<Rect> > &rois, OutputArray pano);
    Stitcher stitcher = Stitcher::createDefault(try_use_gpu); //try_use_gpu
    LOGD("           doStitching createDefault       ");

    Stitcher::Status status = stitcher.stitch(imgs, pano);

    LOGD("           doStitching stitch       ");
    if (status != Stitcher::OK){
    	LOGD("Can't stitch images, error code");
        return -1;
    }

    LOGD("           doStitching ee         ");
    LOGD("           doStitching ee        ");

    Mat *hist = new Mat(pano);
    return (jlong) hist;
}

#ifdef __cplusplus
}
#endif

