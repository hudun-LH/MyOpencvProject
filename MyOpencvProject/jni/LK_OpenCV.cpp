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
IplImage *grey=0,*prev_grey=0,*pyramid=0,*prev_pyramid=0,*swap_temp;   //��������ָ�룬�Ա�洢
CvPoint2D32f *points[2]={0,0},*swap_points;
char* status=0;   //�Ա������г�ʼ��
//int count=0;
int need_to_init=1;
int night_mode=0;
int flags=0;

int runLKOpenCV(IplImage* imgA)   //�������壬�������뵽�������̣�����
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
		//cvPoint(int�ͱ�row,int�ͱ�colunm)
		//cvPoint2D32f(float�ͱ�row,float�ͱ�colunm)

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
	        ����ͼ��image������8λ����32λ��Ҳ����IPL_DEPTH_8U ���� IPL_DEPTH_32F ��ͨ��ͼ��
	       ��2�͵�3�������Ǵ�С������ͼ����ͬ��32λ��ͨ��ͼ��
	        ���� temp_image �� eig_image �ڼ�������б�������ʱ����ʹ�ã�
	       ���������eig_image�е���������Ч�ġ��ر�ģ�ÿһ����������������ͼ���ж�Ӧ����С�ص�ֵ��
	   corners �Ǻ����������Ϊ��⵽ 32λ��CvPoint2D32f���Ľǵ�����
	   corner_count ��ʾ���Է��ص����ǵ���Ŀ���������ý������䷵��ʵ�ʼ�⵽�Ľǵ���Ŀ��
       quality_level ��ʾ1������Ϊ�ǽǵ�Ŀɽ��ܵ���С�ص�ֵ��
                 ʵ�����ڹ��˽ǵ����С�ص�ֵ��quality_level��ͼ���������ص�ֵ�ĳ˻���
                  ����quality_level��ֵ��Ӧ������1��ͨ��ȡֵΪ��0.10����0.01��
                  ������Ժ�Ҫ��1���޳���1Щ����Ͻ��Ľǵ㣬min_distance ��֤���صĽǵ�֮��ľ��벻С��min_distance������
	   */
	   cvGoodFeaturesToTrack(grey, eig, temp,
			    points[1],
			    &count,
			    quality,
			    min_distance,
			    0,
			    3,
			    0,
			    0.04);   //��ȡ��һ֡Ӱ��

	   //�ܹ����ǵ�λ�þ�ȷ�������ؼ�����
	   /*
	   void cvFindCornerSubPix( const CvArr* image, CvPoint2D32f* corners,//����ǵ�ĳ�ʼ���꣬Ҳ�洢��ȷ���������
	   int count, //�ǵ���Ŀ
	   CvSize win, //�������ڵ�һ��ߴ硣��� win=(5,5) ��ôʹ�� 5*2+1 �� 5*2+1 = 11 �� 11 ��С����������
	   CvSize zero_zone,
	   CvTermCriteria criteria );
                   ������һ��ߴ磬����Ϊ����������������λ����������������
                   ����������������ؾ�����ֵ�ĳЩ���ܵ������ԡ���ֵΪ (-1,-1) ��ʾû��������
       criteria
                   ���ǵ�λ�õ�ȷ����Ҫô����������ĳ���趨ֵ�������Ǿ�ȷ�ȴﵽĳ���趨ֵ��
       criteria ��������������Ŀ���������趨�ľ�ȷ�ȣ�Ҳ���������ǵ����
	   */
	   cvFindCornerSubPix(grey, points[1], count,
			      cvSize(win_size,win_size),
			      cvSize(-1,-1), // cvSize(1,1)�ͱ�ʾ�ɺ��Ե�����1������
		          cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03)   //����������iteration�� ��С���ȣ�epsilon��
				  );   //��ȡ���ڸ��ٵ������㣬�����㾫ȷ����

	   for (i=0;i<count;i++){
	       cvRectangle(image, cvPoint(points[1][i].x-r,points[1][i].y-r),
	        		 cvPoint(points[1][i].x+r, points[1][i].y+r), cvScalar(0,0,255), 2);
	   }

	   cvReleaseImage(&eig);
	   cvReleaseImage(&temp);
    }
    else if (count>0)
    {
        //������LK�㷨��cvCalcOpticalFlowPyrLK���������ص㣬һ��������ԭ������ƫ���������λ�ơ�
    	//Lucas-Kandade �Ĺ�������
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
    	                             CV_LKFLOW_PYR_A_READY , �ڵ���֮ǰ����һ֡�Ľ������Ѿ�׼����
                                     CV_LKFLOW_PYR_B_READY , �ڵ���֮ǰ���ڶ�֡�Ľ������Ѿ�׼����
                                     CV_LKFLOW_INITIAL_GUESSES , �ڵ���֮ǰ������ B ���������ĳ�ʼ����
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
		flags);  //��ȡ��һ֡Ӱ�񣬽���KLT���٣�������ɸѡ
		
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
	//cvSmooth(frame,frame,CV_BLUR,5);   //ʹ�ø�˹�˲�������ӦΪ���������򱨴�
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

Mat gray;	// ��ǰͼƬ
Mat gray_prev;	// Ԥ��ͼƬ
vector<Point2f> points0[2];	// point0Ϊ�������ԭ��λ�ã�point1Ϊ���������λ��
vector<Point2f> initial;	// ��ʼ�����ٵ��λ��
vector<Point2f> features;	// ��������
int maxCount = 500;	// �������������
double qLevel = 0.01;	// �������ĵȼ�
double minDist = 10.0;	// ��������֮�����С����
vector<uchar> status0;	// ����������״̬��������������Ϊ1������Ϊ0
vector<float> err;

//����µ��Ƿ�Ӧ�ñ����
bool addNewPoints(){
	return points0[0].size() <= 10;
}

//������Щ���ٵ㱻����
bool acceptTrackedPoint(int i){
	return status0[i] && ((abs(points0[0][i].x - points0[1][i].x) + abs(points0[0][i].y - points0[1][i].y)) > 2);
}

void tracking(Mat &frame, Mat &output)
{
	cvtColor(frame, gray, CV_BGR2GRAY);
	frame.copyTo(output);
	// ���������
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
	// l-k�������˶�����
	calcOpticalFlowPyrLK(gray_prev, gray, points0[0], points0[1], status0, err);
	// ȥ��һЩ���õ�������
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
	// ��ʾ��������˶��켣
	for (size_t i=0; i<points0[1].size(); i++)
	{
		line(output, initial[i], points0[1][i], Scalar(0, 0, 255));
		circle(output, points0[1][i], 3, Scalar(255, 0, 0), -1);
	}

	// �ѵ�ǰ���ٽ����Ϊ��һ�˲ο�
	swap(points0[1], points0[0]);
	swap(gray_prev, gray);

	//imshow(window_name, output);
}

JNIEXPORT jlong JNICALL Java_com_example_carplate_CarPlateDetection_doLKOpenCV0
(JNIEnv *env, jclass clz, jlong imageGray)
{
	LOGD("          doLKOpenCV0         !");
	//cvSmooth(frame,frame,CV_BLUR,5);   //ʹ�ø�˹�˲�������ӦΪ���������򱨴�
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
