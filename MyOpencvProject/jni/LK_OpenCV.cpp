#include <android/log.h>
#include <string>
#include <jni.h>
#include <android/log.h>
#include <iostream>
#include <fstream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/types_c.h"
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/video/tracking.hpp"
#include "opencv2/video/video.hpp"
#include <opencv2/ml/ml.hpp>

using namespace std;
using namespace cv;

#define LOG_TAG "FeatureTest"
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))

#ifdef __cplusplus
extern "C" {
#endif

static IplImage *image=0;
IplImage *grey=0,*prev_grey=0,*pyramid=0,*prev_pyramid=0,*swap_temp;   //创建各类指针，以便存储
CvPoint2D32f *points[2]={0,0},*swap_points;
char* status=0;   //对变量进行初始化
//int count=0;
int need_to_init=1;
int night_mode=0;
int flags=0;

int runLKOpenCV(IplImage* imgA)   //主函数体，可以载入到其他工程，调用
{
	int i=0,k=0,c=0;
	int win_size=15;//10;
	int r=3;
	static int count=0;
	const int MAX_COUNT=500; // 500

	if (!image){
		//allocate all the buffers
		image=cvCreateImage(cvGetSize(imgA),8,3);  
		image->origin=imgA->origin;

		grey=cvCreateImage(cvGetSize(imgA),8,1);
		prev_grey=cvCreateImage(cvGetSize(imgA),8,1);

		pyramid=cvCreateImage(cvGetSize(imgA),8,1);
		prev_pyramid=cvCreateImage(cvGetSize(imgA),8,1);

		//CvPoint Point1;
		//CvPoint2D32f Point6 = cvPointTo32f(Point1);
		//CvPoint Point7 = cvPointFrom32f(Point6);
		//cvPoint(int型别row,int型别colunm)
		//cvPoint2D32f(float型别row,float型别colunm)

		points[0]=(CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(points[0][0]));
		points[1]=(CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(points[0][0]));
		status=(char*)cvAlloc(MAX_COUNT);
		flags=0;
	}

    //convert to grayscale
    //cvCopy(imgA,image,0);
    cvCvtColor(imgA, image, CV_BGRA2BGR);
    cvCvtColor(image,grey,CV_BGR2GRAY);

    if (need_to_init)
    {
	   //automatic initialization
	   IplImage* eig=cvCreateImage(cvGetSize(grey),32,1);
	   IplImage* temp=cvCreateImage(cvGetSize(grey),32,1);
	   double quality=0.01;
	   double min_distance= 15;//10;

	   count=MAX_COUNT;
	   //void cvGoodFeaturesToTrack( const CvArr* image, CvArr* eig_image, CvArr* temp_image,
	   //                            CvPoint2D32f* corners, int* corner_count,
	   //                            double quality_level,
	   //                            double min_distance,
	   //                            const CvArr* mask=NULL );
	   /*
	        输入图象image必须是8位或是32位，也就是IPL_DEPTH_8U 或是 IPL_DEPTH_32F 单通道图象。
	       第2和第3个参数是大小与输入图象相同的32位单通道图象。
	        参数 temp_image 和 eig_image 在计算进程中被当作临时变量使用，
	       计算结束后eig_image中的内容是有效的。特别的，每一个函数包括了输入图象中对应的最小特点值。
	   corners 是函数的输出，为检测到 32位（CvPoint2D32f）的角点数组
	   corner_count 表示可以返回的最大角点数目，函数调用结束后，其返回实际检测到的角点数目。
       quality_level 表示1点呗认为是角点的可接受的最小特点值，
                 实际用于过滤角点的最小特点值是quality_level与图象汇总最大特点值的乘积，
                  所以quality_level的值不应当超过1，通常取值为（0.10或是0.01）
                  检测完以后还要进1步剔除掉1些距离较近的角点，min_distance 保证返回的角点之间的距离不小于min_distance个像素
	   */
	   cvGoodFeaturesToTrack(grey, eig, temp,
			    points[1],
			    &count,
			    quality,
			    min_distance,
			    0,
			    3,
			    0,
			    0.04);   //读取第一帧影像

	   //能够将角点位置精确到亚像素级精度
	   /*
	   void cvFindCornerSubPix( const CvArr* image, CvPoint2D32f* corners,//输入角点的初始坐标，也存储精确的输出坐标
	   int count, //角点数目
	   CvSize win, //搜索窗口的一半尺寸。如果 win=(5,5) 那么使用 5*2+1 × 5*2+1 = 11 × 11 大小的搜索窗口
	   CvSize zero_zone,
	   CvTermCriteria criteria );
                   死区的一半尺寸，死区为不对搜索区的中央位置做求和运算的区域。
                   它是用来避免自相关矩阵出现的某些可能的奇异性。当值为 (-1,-1) 表示没有死区。
       criteria
                   即角点位置的确定，要么迭代数大于某个设定值，或者是精确度达到某个设定值。
       criteria 可以是最大迭代数目，或者是设定的精确度，也可以是它们的组合
	   */
	   cvFindCornerSubPix(grey, points[1], count,
			      cvSize(win_size,win_size),
			      cvSize(-1,-1), // cvSize(1,1)就表示成忽略掉相邻1个像素
		          cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03)   //迭代次数（iteration） 最小精度（epsilon）
				  );   //提取易于跟踪的特征点，特征点精确描述

	   for (i=0;i<count;i++){
	       cvRectangle(image, cvPoint(points[1][i].x-r,points[1][i].y-r),
	        		 cvPoint(points[1][i].x+r, points[1][i].y+r), cvScalar(0,0,255), 2);
	   }

	   cvReleaseImage(&eig);
	   cvReleaseImage(&temp);
    }
    else if (count>0)
    {
        //金字塔LK算法（cvCalcOpticalFlowPyrLK）的两个特点，一、金字塔原理；二、偏导方法求解位移。
    	//Lucas-Kandade 的光流方程
    	/*
    	void cvCalcOpticalFlowPyrLK( const CvArr* prev, const CvArr* curr, CvArr* prev_pyr, CvArr* curr_pyr,
    	                             const CvPoint2D32f* prev_features, CvPoint2D32f* curr_features,
    	                             int count,
    	                             CvSize win_size,
    	                             int level,
    	                             char* status,
    	                             float* track_error,
    	                             CvTermCriteria criteria,
    	                             int flags );
    	                             CV_LKFLOW_PYR_A_READY , 在调用之前，第一帧的金字塔已经准备好
                                     CV_LKFLOW_PYR_B_READY , 在调用之前，第二帧的金字塔已经准备好
                                     CV_LKFLOW_INITIAL_GUESSES , 在调用之前，数组 B 包含特征的初始坐标
    	                             */
	   cvCalcOpticalFlowPyrLK(prev_grey,grey,prev_pyramid,pyramid,
		points[0],
		points[1],
		count,
		cvSize(win_size,win_size),
		3,
		status,
		0,
		cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03),
		flags);  //读取下一帧影像，进行KLT跟踪，特征点筛选
		
        flags|=CV_LKFLOW_PYR_A_READY;

	    for (i=k=0;i<count;i++)
	    {
		   if (!status[i])
			 continue;
		   points[1][k++]=points[1][i];
		   cvCircle(image,cvPointFrom32f(points[1][i]),3,CV_RGB(255,0,0),-1,8,0);
	     }
	    count=k;
    }

    CV_SWAP(prev_grey,grey,swap_temp);
    CV_SWAP(prev_pyramid,pyramid,swap_temp);
    CV_SWAP(points[0],points[1],swap_points);

    need_to_init=0;
   //cvShowImage("LK_OpenCV",image);
   //need_to_init=1;
   //night_mode^=1;

}

JNIEXPORT jlong JNICALL Java_com_example_carplate_CarPlateDetection_doLKOpenCV
(JNIEnv *env, jclass clz, jlong imageGray)
{
	LOGD("          doLKOpenCV         !");
	//cvSmooth(frame,frame,CV_BLUR,5);   //使用高斯滤波，参数应为奇数，否则报错
	//if(frame->origin!=IPL_ORIGIN_TL)
	//cvFlip(frame,frame,0);

	Mat imageMat = Mat(*(Mat*)imageGray);
	IplImage  temp_src = imageMat;
	IplImage* imageg = &temp_src;

	runLKOpenCV(imageg);

    //Mat mtx(image,0);
    Mat *hist = new Mat(image);
    return (jlong) hist;
}

Mat gray;	// 当前图片
Mat gray_prev;	// 预测图片
vector<Point2f> points0[2];	// point0为特征点的原来位置，point1为特征点的新位置
vector<Point2f> initial;	// 初始化跟踪点的位置
vector<Point2f> features;	// 检测的特征
int maxCount = 500;	// 检测的最大特征数
double qLevel = 0.01;	// 特征检测的等级
double minDist = 10.0;	// 两特征点之间的最小距离
vector<uchar> status0;	// 跟踪特征的状态，特征的流发现为1，否则为0
vector<float> err;

//检测新点是否应该被添加
bool addNewPoints(){
	return points0[0].size() <= 10;
}

//决定哪些跟踪点被接受
bool acceptTrackedPoint(int i){
	return status0[i] && ((abs(points0[0][i].x - points0[1][i].x) + abs(points0[0][i].y - points0[1][i].y)) > 2);
}

void tracking(Mat &frame, Mat &output)
{
	cvtColor(frame, gray, CV_BGR2GRAY);
	frame.copyTo(output);
	// 添加特征点
	if (addNewPoints())
	{
		goodFeaturesToTrack(gray, features, maxCount, qLevel, minDist);
		points0[0].insert(points0[0].end(), features.begin(), features.end());
		initial.insert(initial.end(), features.begin(), features.end());
	}

	if (gray_prev.empty())
	{
		gray.copyTo(gray_prev);
	}
	// l-k光流法运动估计
	calcOpticalFlowPyrLK(gray_prev, gray, points0[0], points0[1], status0, err);
	// 去掉一些不好的特征点
	int k = 0;
	for (size_t i=0; i<points0[1].size(); i++)
	{
		if (acceptTrackedPoint(i))
		{
			initial[k] = initial[i];
			points0[1][k++] = points0[1][i];
		}
	}
	points0[1].resize(k);
	initial.resize(k);
	// 显示特征点和运动轨迹
	for (size_t i=0; i<points0[1].size(); i++)
	{
		line(output, initial[i], points0[1][i], Scalar(0, 0, 255));
		circle(output, points0[1][i], 3, Scalar(255, 0, 0), -1);
	}

	// 把当前跟踪结果作为下一此参考
	swap(points0[1], points0[0]);
	swap(gray_prev, gray);

	//imshow(window_name, output);
}

JNIEXPORT jlong JNICALL Java_com_example_carplate_CarPlateDetection_doLKOpenCV0
(JNIEnv *env, jclass clz, jlong imageGray)
{
	LOGD("          doLKOpenCV0         !");
	//cvSmooth(frame,frame,CV_BLUR,5);   //使用高斯滤波，参数应为奇数，否则报错
	//if(frame->origin!=IPL_ORIGIN_TL)
	//cvFlip(frame,frame,0);

	Mat frame = Mat(*(Mat*)imageGray);
	Mat result;
	tracking(frame, result);
    //Mat mtx(image,0);
    Mat *hist = new Mat(result);
    return (jlong) hist;
}

#ifdef __cplusplus
}
#endif
