#include <stdio.h>
#include <iostream>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/types_c.h"
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/video/tracking.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include <android/log.h>
#include <string>
#include <jni.h>
#include <fstream>

//#define LOG_TAG "show infomation"
#define Template 100


#define LOG_TAG "libtracking"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

using namespace cv;  
using namespace std;  

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void Java_com_example_carplate_TrackActivity0_grayProc(JNIEnv* env, jclass obj, jlong imageGray,jint touch_x,jint touch_y){
	int width,height;
    int k = 0,n = 0,i,j;
    CvScalar s;

    int result_cols;
    int result_rows;
    double minVal;
    double maxVal;
    Point minLoc;
    Point maxLoc;
    Point matchLoc;
    int match_method = CV_TM_SQDIFF;

    Mat result;
	Mat mat = Mat(*((Mat*)imageGray));
	width = mat.rows;
	height = mat.cols;
    
    result_cols = height - Template + 1;
    result_rows = width - Template + 1;

    cv::Mat img = cv::Mat(Template,Template,CV_8UC3,cv::Scalar(0,0,0));
    IplImage pI = mat;
    IplImage pI_2 = img;

    Mat img_display = cv::Mat(width,height,CV_8UC3,cv::Scalar(0,0,0));
    IplImage pI_3 = img_display;
    for(i=0;i<width;i++){
        for(j=0;j<height;j++){
            s = cvGet2D(&pI,i,j);
            cvSet2D(&pI_3,i,j,s);
        }
    }

    if((touch_x < (Template/2)) || (touch_x>(height-(Template/2))) || (touch_y < (Template/2)) || (touch_y > (width-(Template/2)))){
    	//__android_log_print(ANDROID_LOG_ERROR, "JNITag","touch_x,touch_y is too small or too large\n");
    	LOGD("touch_x = %d, touch_y = %d, width = %d, height = %d", touch_x, touch_y, width, height);
        return;
    }

    for(i=(touch_x-(Template/2));i<(touch_x+(Template/2));i++){
        for(j=(touch_y-(Template/2));j<(touch_y+(Template/2));j++){
            s = cvGet2D(&pI,j,i);
            cvSet2D(&pI_2,k,n,s);
            n++;
        }   
        k++;
        n=0;
    }

    /*
    result.create(result_cols, result_rows, CV_32FC1 );
    /// Do the Matching and Normalize
    matchTemplate( img_display, img, result, match_method);
    normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat() );
    /// Localizing the best match with minMaxLoc
    minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );
    */

    /// Do the Matching and Normalize
    matchTemplate( img_display, img, img_display, match_method);
    normalize(img_display, img_display, 0, 1, NORM_MINMAX, -1, Mat() );
    /// Localizing the best match with minMaxLoc

    minMaxLoc(img_display, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );
    /// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
    if(match_method == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED ){
        matchLoc = minLoc;
    }else{
        matchLoc = maxLoc;
    }

    rectangle(mat, matchLoc, Point(matchLoc.x + img.cols , matchLoc.y + img.rows ), Scalar::all(0), 2, 8, 0);

}

int run_time;

JNIEXPORT jlong Java_com_example_carplate_TrackActivity0_grayProc_0(JNIEnv* env, jclass obj, jlong imageGray, jlong tempGray )
{
	//Mat& img_display = *(Mat*)imageGray;
	//Mat& img  = *(Mat*)tempGray;
	LOGD("         grayProc_0 ss       !");
	LOGD("         grayProc_0 ss       !");

	int result_cols;
	int result_rows;
	double minVal;
	double maxVal;
	Point minLoc;
	Point maxLoc;
	Point matchLoc;
	int match_method = CV_TM_SQDIFF;

	Mat img_display =  Mat(*(Mat*)imageGray);
	Mat img  =  Mat(*(Mat*)tempGray);

    /// Do the Matching and Normalize
    matchTemplate( img_display, img, img_display, match_method);
    normalize(img_display, img_display, 0, 1, NORM_MINMAX, -1, Mat() );
    /// Localizing the best match with minMaxLoc

    minMaxLoc(img_display, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );
    /// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
    if(match_method == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED ){
        matchLoc = minLoc;
    }else{
        matchLoc = maxLoc;
    }

    //Mat& src  = *(Mat*)addrGray;
    //rectangle(src, cvPoint(0,0),cvPoint(64,64), CV_RGB(0,255,0), 2);
    rectangle(img_display, matchLoc, Point(matchLoc.x + img.cols , matchLoc.y + img.rows ), Scalar::all(0), 2, 8, 0);

    LOGD("         grayProc_0 ee       !");
    LOGD("         grayProc_0 ee       !");

    Mat *hist = new Mat(img_display);
    return (jlong) hist;
}

#ifdef __cplusplus
}
#endif
