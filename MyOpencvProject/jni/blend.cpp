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
#include <cv.h>
#include <highgui.h>
#include <iostream>  
#include <stdio.h>  

#define LOG_TAG "Blending"
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))

using namespace cv; 
using namespace std; 

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jlong JNICALL Java_com_example_carplate_CarPlateDetection_doBlending
  (JNIEnv *env, jclass obj, jlong imageGray, jlong tempGray)
{
	LOGD("           doBlending ss         ");
	LOGD("           doBlending ss        ");

	/*
	initModule_features2d();//��ʼ��ģ�飬ʹ��SIFT��SURFʱ�õ�
	Ptr<FeatureDetector> detector = FeatureDetector::create("ORB");//����SIFT������������ɸĳ�SURF/ORB
	Ptr<DescriptorExtractor> descriptor_extractor = DescriptorExtractor::create("ORB");//���������������������ɸĳ�SURF/ORB
	Ptr<DescriptorMatcher> descriptor_matcher = DescriptorMatcher::create("BruteForce");//��������ƥ����
	if( detector.empty() || descriptor_extractor.empty()) {
		LOGD("fail to create detector!");
	}
	*/
	ORB *orb = new ORB();

	//����ͼ��  
	Mat img1 =  Mat(*(Mat*)imageGray);
	Mat img2 = Mat(*(Mat*)tempGray);

	cvtColor(img1,img1,CV_BGR2GRAY);  //CV_BGRA2BGR
	cvtColor(img2,img2,CV_BGR2GRAY);  //CV_BGRA2BGR

	//��������  
	double t = getTickCount();//��ǰ�δ���  
	vector<KeyPoint> m_LeftKey,m_RightKey;  
	//detector->detect( img1, m_LeftKey );//���img1�е�SIFT�����㣬�洢��m_LeftKey��
	//detector->detect( img2, m_RightKey );
	orb->detect(img1, m_LeftKey);
	orb->detect(img2, m_RightKey);
	LOGD("img1 KeyPoint: %d",m_LeftKey.size());
	LOGD("img2 KeyPoint: %d",m_RightKey.size());

	//����������������������Ӿ��󣬼�������������  
	Mat descriptors1,descriptors2;  
	//BriefDescriptorExtractor
	//descriptor_extractor->compute( img1, m_LeftKey, descriptors1 );
	//descriptor_extractor->compute( img2, m_RightKey, descriptors2 );
	orb->compute(img1, m_LeftKey, descriptors1 );
    orb->compute(img2, m_RightKey, descriptors2 );
	t = ((double)getTickCount() - t)/getTickFrequency();  
	LOGD("SIFT time: %f ��", t);

	//descriptors1.size()
	LOGD("ͼ��1�������������С�������������� %d, ά����%d", descriptors1.rows, descriptors1.cols);
	LOGD("ͼ��2�������������С�������������� %d, ά����%d", descriptors2.rows, descriptors2.cols);

	//����������  
	//Mat img_m_LeftKey,img_m_RightKey;
	//drawKeypoints(img1,m_LeftKey,img_m_LeftKey,Scalar::all(-1),0);
	//drawKeypoints(img2,m_RightKey,img_m_RightKey,Scalar::all(-1),0);

	////imshow("Src1",img_m_LeftKey);
	////imshow("Src2",img_m_RightKey);

	//����ƥ��   FlannBasedMatcher
	BFMatcher *descriptor_matcher = new BFMatcher();  //BruteForceMatcher
	//descriptor_matcher->normType = NORM_HAMMING;
	//enum { NORM_INF=1, NORM_L1=2, NORM_L2=4, NORM_L2SQR=5, NORM_HAMMING=6, NORM_HAMMING2=7, NORM_TYPE_MASK=7, NORM_RELATIVE=8, NORM_MINMAX=32 };
	vector<DMatch> matches;//ƥ����  
	descriptor_matcher->match( descriptors1, descriptors2, matches );//ƥ������ͼ�����������  
	LOGD("Match: %d",matches.size());

	//����ƥ�����о����������Сֵ  
	//������ָ���������������ŷʽ���룬�������������Ĳ��죬ֵԽС��������������Խ�ӽ�  
	//SIFT�㷨ԭ�����ߵ�˼·��ÿ�����������һ��128ά��������
	//��������֮���ŷʽ���룬��������ȴν��ķ�ʽ���ƥ�䣬������������ϴν�����С��0.8������Ϊ����һ����ȷ��ƥ�䣬 ������Ϊƥ�䲻�ɹ�

	double max_dist = 0;  
	double min_dist = 100;
	for(int i=0; i<matches.size(); i++)  
	{  
		double dist = matches[i].distance;  
		//LOGD("dist %f", dist);
		if(dist < min_dist) min_dist = dist;  
		if(dist > max_dist) max_dist = dist;  
	}  
	LOGD("max_dist: %f",max_dist);
	LOGD("min_dist: %f",min_dist);

	//ɸѡ���Ϻõ�ƥ���  
	vector<DMatch> goodMatches;  
	for(int i=0; i<matches.size(); i++)  
	{  
		if(matches[i].distance < 4*min_dist) //0.8 * max_dist
		{  
			goodMatches.push_back(matches[i]);  
		}  
	}  
	LOGD("goodMatch: %d",goodMatches.size());

	//����ƥ����  
	Mat img_matches;  
	//��ɫ���ӵ���ƥ���������ԣ���ɫ��δƥ���������  
	drawMatches(img1,m_LeftKey,img2,m_RightKey,goodMatches,img_matches,  
		Scalar::all(-1)/*CV_RGB(255,0,0)*/,CV_RGB(0,255,0),Mat(),2);

	//imshow( "Good Matches & Object detection", img_matches );
	//Mat *hist0 = new Mat(img_matches);
	//return (jlong) hist0;

	//-- Localize the object
	vector<Point2f> objs;
	vector<Point2f> scenes;

	for( int i = 0; i < goodMatches.size(); i++ )
    {
	    //-- Get the keypoints from the good matches
	    objs.push_back( m_LeftKey[ goodMatches[i].queryIdx ].pt );
	    scenes.push_back( m_RightKey[ goodMatches[i].trainIdx ].pt );
	}

	Mat H = findHomography( objs, scenes, CV_RANSAC );

	//-- Get the corners from the image_1 ( the object to be "detected" )
	vector<Point2f> obj_corners(4);
	obj_corners[0] = cvPoint(0, 0);
	obj_corners[1] = cvPoint( img1.cols, 0 );
	obj_corners[2] = cvPoint( img1.cols, img1.rows );
	obj_corners[3] = cvPoint( 0, img1.rows );
	vector<Point2f> scene_corners(4);

	line( img_matches, obj_corners[0], obj_corners[1], Scalar(255, 0, 0), 2 );
	line( img_matches, obj_corners[1], obj_corners[2], Scalar(255, 0, 0), 2 );
	line( img_matches, obj_corners[2], obj_corners[3], Scalar(255, 0, 0), 2 );
	line( img_matches, obj_corners[3], obj_corners[0], Scalar(255, 0, 0), 2 );

	LOGD("img1.cols: %d, img1.rows :%d", img1.cols,img1.rows);
	LOGD("img_matches: cols = %d, rows = %d, type = %d, channels = %d", img_matches.cols,img_matches.rows, img_matches.type(), img_matches.channels());

	perspectiveTransform( obj_corners, scene_corners, H);
    /*
	line( img_matches, scene_corners[0], scene_corners[1], Scalar(255, 255, 0), 2 );
	line( img_matches, scene_corners[1], scene_corners[2], Scalar(255, 255, 0), 2 );
	line( img_matches, scene_corners[2], scene_corners[3], Scalar(255, 255, 0), 2 );
	line( img_matches, scene_corners[3], scene_corners[0], Scalar(255, 255, 0), 2 );
    */

	//-- Draw lines between the corners (the mapped object in the scene - image_2 )
	line( img_matches, scene_corners[0] + Point2f( img1.cols, 0), scene_corners[1] + Point2f( img1.cols, 0), Scalar(255, 255, 255), 2 );
    line( img_matches, scene_corners[1] + Point2f( img1.cols, 0), scene_corners[2] + Point2f( img1.cols, 0), Scalar(255, 255, 255), 2 );
	line( img_matches, scene_corners[2] + Point2f( img1.cols, 0), scene_corners[3] + Point2f( img1.cols, 0), Scalar(255, 255, 255), 2 );
	line( img_matches, scene_corners[3] + Point2f( img1.cols, 0), scene_corners[0] + Point2f( img1.cols, 0), Scalar(255, 255, 255), 2 );

	//cvtColor(img_matches,img_matches,CV_BGR2GRAY);  //CV_BGRA2BGR

	//-- Show detected matches
	//imshow( "Good Matches & Object detection", img_matches );
    Mat *hist = new Mat(img_matches);
	return (jlong) hist;
}

JNIEXPORT jlong JNICALL Java_com_example_carplate_CarPlateDetection_doBlending0
  (JNIEnv *env, jclass obj, jlong imageGray, jlong tempGray)
{
	LOGD("           doBlending0 ss         ");
	LOGD("           doBlending0 ss        ");

	/*
	initModule_features2d();//��ʼ��ģ�飬ʹ��SIFT��SURFʱ�õ�
	Ptr<FeatureDetector> detector = FeatureDetector::create("ORB");//����SIFT������������ɸĳ�SURF/ORB
	Ptr<DescriptorExtractor> descriptor_extractor = DescriptorExtractor::create("ORB");//���������������������ɸĳ�SURF/ORB
	Ptr<DescriptorMatcher> descriptor_matcher = DescriptorMatcher::create("BruteForce");//��������ƥ����
	if( detector.empty() || descriptor_extractor.empty()) {
		LOGD("fail to create detector!");
	}
	*/

	ORB *orb = new ORB();

	//����ͼ��
	Mat img1 =  Mat(*(Mat*)imageGray);
	Mat img2 = Mat(*(Mat*)tempGray);

	cvtColor(img1,img1,CV_BGR2GRAY);  //CV_BGRA2BGR
    cvtColor(img2,img2,CV_BGR2GRAY);  //CV_BGRA2BGR

	//��������
	double t = getTickCount();//��ǰ�δ���
	vector<KeyPoint> m_LeftKey,m_RightKey;
	//detector->detect( img1, m_LeftKey );//���img1�е�SIFT�����㣬�洢��m_LeftKey��
	//detector->detect( img2, m_RightKey );
	orb->detect(img1, m_LeftKey);
	orb->detect(img2, m_RightKey);
	LOGD("img1 KeyPoint: %d",m_LeftKey.size());
	LOGD("img2 KeyPoint: %d",m_RightKey.size());

	//����������������������Ӿ��󣬼�������������
	Mat descriptors1,descriptors2;
	//BriefDescriptorExtractor
	//descriptor_extractor->compute( img1, m_LeftKey, descriptors1 );
	//descriptor_extractor->compute( img2, m_RightKey, descriptors2 );
	orb->compute(img1, m_LeftKey, descriptors1 );
    orb->compute(img2, m_RightKey, descriptors2 );
	t = ((double)getTickCount() - t)/getTickFrequency();
	LOGD("SIFT time: %f ��", t);

	//descriptors1.size()
	LOGD("ͼ��1�������������С�������������� %d, ά����%d", descriptors1.rows, descriptors1.cols);
	LOGD("ͼ��2�������������С�������������� %d, ά����%d", descriptors2.rows, descriptors2.cols);

	//����������
	//Mat img_m_LeftKey,img_m_RightKey;
	//drawKeypoints(img1,m_LeftKey,img_m_LeftKey,Scalar::all(-1),0);
	//drawKeypoints(img2,m_RightKey,img_m_RightKey,Scalar::all(-1),0);

	////imshow("Src1",img_m_LeftKey);
	////imshow("Src2",img_m_RightKey);

	//����ƥ��   FlannBasedMatcher
	BFMatcher *descriptor_matcher = new BFMatcher();  //BruteForceMatcher
	//descriptor_matcher->normType = NORM_HAMMING;
	//enum { NORM_INF=1, NORM_L1=2, NORM_L2=4, NORM_L2SQR=5, NORM_HAMMING=6, NORM_HAMMING2=7, NORM_TYPE_MASK=7, NORM_RELATIVE=8, NORM_MINMAX=32 };
	vector<DMatch> matches;//ƥ����
	descriptor_matcher->match( descriptors1, descriptors2, matches );//ƥ������ͼ�����������
	LOGD("Match: %d",matches.size());

	//����ƥ�����о����������Сֵ
	//������ָ���������������ŷʽ���룬�������������Ĳ��죬ֵԽС��������������Խ�ӽ�
	//SIFT�㷨ԭ�����ߵ�˼·��ÿ�����������һ��128ά��������
	//��������֮���ŷʽ���룬��������ȴν��ķ�ʽ���ƥ�䣬������������ϴν�����С��0.8������Ϊ����һ����ȷ��ƥ�䣬 ������Ϊƥ�䲻�ɹ�

	double max_dist = 0;
	double min_dist = 100;
	for(int i=0; i<matches.size(); i++)
	{
		double dist = matches[i].distance;
		//LOGD("dist %f", dist);
		if(dist < min_dist) min_dist = dist;
		if(dist > max_dist) max_dist = dist;
	}
	LOGD("max_dist: %f",max_dist);
	LOGD("min_dist: %f",min_dist);

	//ɸѡ���Ϻõ�ƥ���
	vector<DMatch> goodMatches;
	for(int i=0; i<matches.size(); i++)
	{
		if(matches[i].distance < 0.7 * max_dist)
		{
			goodMatches.push_back(matches[i]);
		}
	}
	LOGD("goodMatch: %d",goodMatches.size());

    /*
	//����ƥ����
	Mat img_matches;
	//��ɫ���ӵ���ƥ���������ԣ���ɫ��δƥ���������
	drawMatches(img1,m_LeftKey,img2,m_RightKey,goodMatches,img_matches,
		Scalar::all(-1),CV_RGB(0,255,0),Mat(),2);   //CV_RGB(255,0,0)
    */

	//imshow("MatchSIFT",img_matches);
	//IplImage result=img_matches;

	//RANSACƥ�����
	vector<DMatch> m_Matches=goodMatches;
	// ����ռ�
	int ptCount = (int)m_Matches.size();
	Mat p1(ptCount, 2, CV_32F);
	Mat p2(ptCount, 2, CV_32F);

	// ��Keypointת��ΪMat
	Point2f pt;
	for (int i=0; i<ptCount; i++)
	{
		pt = m_LeftKey[m_Matches[i].queryIdx].pt;
		p1.at<float>(i, 0) = pt.x;
		p1.at<float>(i, 1) = pt.y;

		pt = m_RightKey[m_Matches[i].trainIdx].pt;
		p2.at<float>(i, 0) = pt.x;
		p2.at<float>(i, 1) = pt.y;
	}

	// ��RANSAC��������F
	Mat m_Fundamental;
	vector<uchar> m_RANSACStatus;       // ����������ڴ洢RANSAC��ÿ�����״̬
	//RASIC��������������󣬲�ϸ��ƥ����
	/*
	CV_EXPORTS_W Mat findFundamentalMat( InputArray points1, InputArray points2,
	                                     int method=FM_RANSAC,
	                                     double param1=3., double param2=0.99,
	                                     OutputArray mask=noArray());
     points1��points2 ����ͼ�����ƥ��ĵ㣬�������Ҫ�Ǹ�������float����double��
           ����������method�����������������ľ��巽������һ��ö��ֵ��
     param1,param2����Ĭ��ֵ���ɡ�
           ��Ҫ��˵��mask��������N��ƥ���������������������ֵ��N��Ԫ�أ�ÿ��Ԫ�ص�ֵΪ0����1.
           ֵΪ0ʱ�������ƥ����´����ƥ�䣨��Ⱥֵ����ֻ��ʹ��RANSAC��LMeds����ʱ��ֵ��Ч��
           ����ʹ�ø�ֵ��ɾ�������ƥ�䡣
	*/
	/*
	 RANSAC�ǡ�RANdom SAmple Consensus���������һ�£�������д��
	 �����Դ�һ�����������㡱�Ĺ۲����ݼ��У�ͨ��������ʽ������ѧģ�͵Ĳ�����
	 ����һ�ֲ�ȷ�����㷨��������һ���ĸ��ʵó�һ������Ľ����
	 Ϊ����߸��ʱ�����ߵ������������㷨������Fischler��Bolles��1981�������
     RANSAC�Ļ��������ǣ���1�������ɡ����ڵ㡱��ɣ����磺���ݵķֲ�������һЩģ�Ͳ��������ͣ�
         ��2��������㡱�ǲ�����Ӧ��ģ�͵����ݣ�
         ��3������֮���������������������������ԭ���У������ļ�ֵ������Ĳ��������������ݵĴ������
	 * */
	findFundamentalMat(p1, p2, m_RANSACStatus, FM_RANSAC); //findFundamentalMat������������ƥ��
	//��������ͱ任������������ͬ�ĸ������������������ά�����е����֮��Ķ�Ӧ��ϵ

	LOGD("m_RANSACStatus: %d",m_RANSACStatus.size());
	// ����Ұ�����

	int OutlinerCount = 0;
	for (int i=0; i<ptCount; i++)
	{
		if (m_RANSACStatus[i] == 0)    // ״̬Ϊ0��ʾҰ��
		{
			OutlinerCount++;
		}
	}
	int InlinerCount = ptCount - OutlinerCount;   // �����ڵ�
	LOGD("Inliner Count: %d",InlinerCount);

	
   // �������������ڱ����ڵ��ƥ���ϵ
   vector<Point2f> m_LeftInlier;
   vector<Point2f> m_RightInlier;
   vector<DMatch> m_InlierMatches;

	m_InlierMatches.resize(InlinerCount);
	m_LeftInlier.resize(InlinerCount);
	m_RightInlier.resize(InlinerCount);
	InlinerCount=0;
	float inlier_minRx=img1.cols;        //���ڴ洢�ڵ�����ͼ��С�����꣬�Ա�����ں�

	for (int i=0; i<ptCount; i++)
	{
		if (m_RANSACStatus[i] != 0)
		{
			m_LeftInlier[InlinerCount].x = p1.at<float>(i, 0);
			m_LeftInlier[InlinerCount].y = p1.at<float>(i, 1);
			m_RightInlier[InlinerCount].x = p2.at<float>(i, 0);
			m_RightInlier[InlinerCount].y = p2.at<float>(i, 1);
			m_InlierMatches[InlinerCount].queryIdx = InlinerCount;
			m_InlierMatches[InlinerCount].trainIdx = InlinerCount;

			if(m_RightInlier[InlinerCount].x<inlier_minRx)
				inlier_minRx=m_RightInlier[InlinerCount].x;   //�洢�ڵ�����ͼ��С������

			InlinerCount++;
		}
	}

	// ���ڵ�ת��ΪdrawMatches����ʹ�õĸ�ʽ
	vector<KeyPoint> key1(InlinerCount);
	vector<KeyPoint> key2(InlinerCount);
	KeyPoint::convert(m_LeftInlier, key1);
	KeyPoint::convert(m_RightInlier, key2);

    // ��ʾ����F������ڵ�ƥ��
	Mat OutImage;
	drawMatches(img1, key1, img2, key2, m_InlierMatches, OutImage);
	//drawMatches(img1, key1, img2, key2, m_InlierMatches,OutImage,
	//		Scalar::all(-1)/*CV_RGB(255,0,0)*/,CV_RGB(0,255,0),Mat(),2);

	//cvNamedWindow( "Match features", 1);
	//cvShowImage("Match features", &IplImage(OutImage));

	//����H���Դ洢RANSAC�õ��ĵ�Ӧ����   == ��RANSAC���������͸�Ӿ���
	//RASIC��������������󣬲�ϸ��ƥ����
	//���㵥Ӧ����H����ϸ��ƥ����
	/*
	CV_EXPORTS_W Mat findHomography( InputArray srcPoints, InputArray dstPoints,
                                  int method=0, double ransacReprojThreshold=3,
                                  OutputArray mask=noArray());
	                              srcPoints,dstPoints������ͼ��ƥ��ĵ�
	                              method �Ǽ��㵥Ӧ������ʹ�õķ�������һ��ö��ֵ , CV_FM_RANSAC��
	                              ransacReprojThreshold ����������ͶӰ����ֻ��ʹ��RANSAC����ʱ��Ч��
	                              mask ͬfindFundamentalMat ���ƣ�ָ��ƥ��ĵ��ǲ�����Ⱥֵ�������Ż�ƥ������
	*/
	Mat H = findHomography( m_LeftInlier, m_RightInlier, RANSAC );//����findHomography������������������ļ���任����ĺ���

	LOGD("       findHomography       ");

	//�洢��ͼ�Ľǣ�����任����ͼλ��
	vector<Point2f> obj_corners(4);
	obj_corners[0] = Point(0,0);
	obj_corners[1] = Point( img1.cols, 0 );
	obj_corners[2] = Point( img1.cols, img1.rows );
	obj_corners[3] = Point( 0, img1.rows );
	vector<Point2f> scene_corners(4);
	perspectiveTransform( obj_corners, scene_corners, H);

	//�����任��ͼ��λ��
	/*
	Point2f offset( (float)img1.cols, 0);  
	line( OutImage, scene_corners[0]+offset, scene_corners[1]+offset, Scalar( 0, 255, 0), 4 );
	line( OutImage, scene_corners[1]+offset, scene_corners[2]+offset, Scalar( 0, 255, 0), 4 );
	line( OutImage, scene_corners[2]+offset, scene_corners[3]+offset, Scalar( 0, 255, 0), 4 );
	line( OutImage, scene_corners[3]+offset, scene_corners[0]+offset, Scalar( 0, 255, 0), 4 );
	*/
	line( OutImage, scene_corners[0] + Point2f( img1.cols, 0), scene_corners[1] + Point2f( img1.cols, 0), Scalar(255, 255, 255), 2 );
    line( OutImage, scene_corners[1] + Point2f( img1.cols, 0), scene_corners[2] + Point2f( img1.cols, 0), Scalar(255, 255, 255), 2 );
	line( OutImage, scene_corners[2] + Point2f( img1.cols, 0), scene_corners[3] + Point2f( img1.cols, 0), Scalar(255, 255, 255), 2 );
	line( OutImage, scene_corners[3] + Point2f( img1.cols, 0), scene_corners[0] + Point2f( img1.cols, 0), Scalar(255, 255, 255), 2 );
	//imshow( "Good Matches & Object detection", OutImage );

	IplImage image_warp_t = OutImage;
	IplImage *image_warp = &image_warp_t;
	//cvSaveImage("/storage/emulated/0/warp_position.jpg", image_warp);// &IplImage(OutImage));

	Mat *hist0 = new Mat(OutImage);
    //return (jlong) hist0;

    jclass dpclazz = env->FindClass("com/example/carplate/MainActivity"); //com_example_mymp4v2h264_MainActivity0_Mp4FpgDCE0
    if(dpclazz==0){
    	LOGD("find class error");
    	return -1;
    }
    LOGD("find class ");

    //jmethodID mid = env->GetMethodID(dpclazz, "<init>", "()V");
    // jmethodID post_event = env->GetMethodID(clazz, "jShow", "()V");
    //jobject jobt = env->NewObject(dpclazz, mid);

    //2 Ѱ��class����ķ���
    //   jmethodID   (*GetMethodID)(JNIEnv*, jclass, const char*, const char*);
    //jmethodID method1 = env->GetMethodID(dpclazz,"updateMat","(J)V");
    jmethodID method1 = env->GetStaticMethodID(dpclazz,"updateMat","(J)V");
    if(method1==0){
    	LOGD("find method1 error");
    	return -1;
    }
    LOGD("find method1 ");

    jlong mat_add = (jlong)hist0;
    //env->CallVoidMethod(obj,method1,mat_add);
    env->CallStaticVoidMethod(dpclazz,method1,mat_add);

	int drift = scene_corners[1].x; //����ƫ����

	LOGD("scene_corners[].x: %f, %f, %f, %f",scene_corners[1].x, scene_corners[2].x, scene_corners[3].x, scene_corners[4].x);

	//�½�һ������洢��׼���Ľǵ�λ��

	int width = int(max(abs(scene_corners[1].x), abs(scene_corners[2].x)));
	int height= img1.rows;

	LOGD("00 abs(scene_corners[1/2/3/4].x): %f, %f, %f, %f", abs(scene_corners[0].x), abs(scene_corners[1].x),
			abs(scene_corners[2].x), abs(scene_corners[3].x));
	LOGD("00 scene_corners[1/2/3/4].x: %f, %f, %f, %f", scene_corners[0].x, scene_corners[1].x,
				scene_corners[2].x, scene_corners[3].x);
	LOGD("width: %d, height = %d",width, height);

	//���ߣ�int height = int(max(abs(scene_corners[2].y), abs(scene_corners[3].y)));
	float origin_x=0 ,origin_y=0;

	if(scene_corners[0].x<0) {
		if (scene_corners[3].x<0)
			origin_x+=min(scene_corners[0].x, scene_corners[3].x);
		else
			origin_x+=scene_corners[0].x;}
		width-=int(origin_x);

	if(scene_corners[0].y<0) {
		if (scene_corners[1].y)
			origin_y+=min(scene_corners[0].y, scene_corners[1].y);
		else
			origin_y+=scene_corners[0].y;}

	//��ѡ��height-=int(origin_y);
	LOGD("img1.cols: %d, img1.rows :%d, img1.type() = %d", img1.cols, img1.rows, img1.type());
	LOGD("width: %d, height = %d, %f, %f",width, height, origin_x, origin_y);

	Mat imageturn;//=Mat::zeros(width, height, img1.type());//width

	//��ȡ�µı任����ʹͼ��������ʾ
	for (int i=0;i<4;i++){
		scene_corners[i].x -= origin_x; //��ѡ��scene_corners[i].y -= (float)origin_y; }
	}

	LOGD("22 abs(scene_corners[1/2/3/4].x): %f, %f, %f, %f", abs(scene_corners[0].x), abs(scene_corners[1].x),
				abs(scene_corners[2].x), abs(scene_corners[3].x));
	LOGD("22 scene_corners[1/2/3/4].x: %f, %f, %f, %f", scene_corners[0].x, scene_corners[1].x,
					scene_corners[2].x, scene_corners[3].x);
	/*
	//���ı��ε�4�������͸��任
	CvMat* cvGetPerspectiveTransform( const CvPoint2D32f* src, const CvPoint2D32f* dst,
	                                  CvMat* map_matrix );

	#define cvWarpPerspectiveQMatrix cvGetPerspectiveTransform
	src
	����ͼ����ı��ζ������ꡣ
	dst
	���ͼ�����Ӧ���ı��ζ������ꡣ
	map_matrix
	ָ��3��3��������ָ�롣
	����cvGetPerspectiveTransform�����������¹�ϵ��͸��任����
	*/

	Mat H1=getPerspectiveTransform(obj_corners, scene_corners);

	//����ͼ��任����ʾЧ��
/*
C++: void warpPerspective(InputArray src, OutputArray dst, InputArray M,
Size dsize, int flags=INTER_LINEAR, int borderMode=BORDER_CONSTANT, const Scalar& borderValue=Scalar())
������⣺
InputArray src�������ͼ��
OutputArray dst�������ͼ��
InputArray M��͸�ӱ任�ľ���
Size dsize�����ͼ��Ĵ�С
int flags=INTER_LINEAR�����ͼ��Ĳ�ֵ������
combination of interpolation methods (INTER_LINEAR or INTER_NEAREST)
and the optional flagWARP_INVERSE_MAP, that sets M as the inverse transformation ( \texttt{dst}\rightarrow\texttt{src} )
int borderMode=BORDER_CONSTANT��ͼ��߽�Ĵ���ʽ
const Scalar& borderValue=Scalar()���߽����ɫ���ã�һ��Ĭ����0
*/
	LOGD("imageturn.cols: %d, imageturn.rows = %d",imageturn.cols, imageturn.rows);
	LOGD("img1.cols: %d, img1.rows = %d",img1.cols, img1.rows);

	warpPerspective(img1, imageturn, H1, Size(width,height));
	//imshow("image_Perspective", imageturn);
	LOGD("imageturn.cols: %d, imageturn.rows = %d",imageturn.cols, imageturn.rows);

	//ͼ���ں�
	int width_ol = width - int(inlier_minRx - origin_x);
	int start_x  = int(inlier_minRx - origin_x);

	LOGD("drift: %f, inlier_minRx: %f, origin_x: %f",drift, inlier_minRx, origin_x);

    LOGD("width: %d, height: %d",width, height);
	LOGD("img1.width: %d",img1.cols);
	LOGD("start_x: %d",start_x);
	LOGD("width_ol: %d",width_ol);

	uchar* ptr=imageturn.data;
	double alpha=0, beta=1;
	for (int row=0;row<height;row++) {
		ptr = imageturn.data+row*imageturn.step + (start_x)*imageturn.elemSize();

		for(int col=0;col<width_ol;col++)
		{
			uchar* ptr_c1= ptr + imageturn.elemSize1();
			uchar* ptr_c2=ptr_c1 + imageturn.elemSize1();
			uchar* ptr2=img2.data+row*img2.step + (col+int(inlier_minRx))*img2.elemSize();
			uchar* ptr2_c1=ptr2+img2.elemSize1();
			uchar* ptr2_c2=ptr2_c1+img2.elemSize1();

			
			alpha=double(col)/double(width_ol);
			beta=1-alpha;

			if (*ptr==0&&*ptr_c1==0&&*ptr_c2==0) {
				*ptr=(*ptr2);
				*ptr_c1=(*ptr2_c1);
				*ptr_c2=(*ptr2_c2);
			}

			*ptr=(*ptr)*beta+(*ptr2)*alpha;
			*ptr_c1=(*ptr_c1)*beta+(*ptr2_c1)*alpha;
			*ptr_c2=(*ptr_c2)*beta+(*ptr2_c2)*alpha;

			ptr+=imageturn.elemSize();
		}	}
	
	////imshow("image_overlap", imageturn);
	//waitKey(0);

	Mat img_result=Mat::zeros(height, width+img2.cols-drift, img1.type());
	uchar* ptr_r=imageturn.data;
	
	for (int row=0;row<height;row++) {
		ptr_r=img_result.data+row*img_result.step;

		for(int col=0;col<imageturn.cols;col++)
		{
			uchar* ptr_rc1=ptr_r+imageturn.elemSize1();
			uchar*  ptr_rc2=ptr_rc1+imageturn.elemSize1();

			uchar* ptr=imageturn.data+row*imageturn.step+col*imageturn.elemSize();
			uchar* ptr_c1=ptr+imageturn.elemSize1();
			uchar*  ptr_c2=ptr_c1+imageturn.elemSize1();

			*ptr_r=*ptr;
			*ptr_rc1=*ptr_c1;
			*ptr_rc2=*ptr_c2;

			ptr_r+=img_result.elemSize();
		}	

		ptr_r=img_result.data+row*img_result.step+imageturn.cols*img_result.elemSize();
		for(int col=imageturn.cols;col<img_result.cols;col++)
		{
			uchar* ptr_rc1=ptr_r+imageturn.elemSize1();  uchar*  ptr_rc2=ptr_rc1+imageturn.elemSize1();

			uchar* ptr2=img2.data+row*img2.step+(col-imageturn.cols+drift)*img2.elemSize();
			uchar* ptr2_c1=ptr2+img2.elemSize1();  uchar* ptr2_c2=ptr2_c1+img2.elemSize1();

			*ptr_r=*ptr2;
			*ptr_rc1=*ptr2_c1;
			*ptr_rc2=*ptr2_c2;

			ptr_r+=img_result.elemSize();
		}	
	}
	//Mat m_mat();
	//IplImage *src=&IplImage(m_mat);

	//IplImage image_final_r = img_result;
    //IplImage *image_final = &image_final_r;
	//imshow("image_result", img_result);
    //cvSaveImage("/storage/emulated/0/final_result.jpg", image_final);//&IplImage(img_result));

    LOGD("           doBlending0 ee         ");
    LOGD("           doBlending0 ee         ");

    //Mat mtx(image_final,0);
    Mat *hist = new Mat(img_result);
    return (jlong) hist;

	//return 0;
}

JNIEXPORT jlong JNICALL Java_com_example_carplate_CarPlateDetection_doBlending1
  (JNIEnv *env, jclass obj, jlong imageGray, jlong tempGray)
{
	LOGD("           doBlending1 ss         ");
	LOGD("           doBlending1 ss        ");

	/*
	initModule_features2d();//��ʼ��ģ�飬ʹ��SIFT��SURFʱ�õ�
	Ptr<FeatureDetector> detector = FeatureDetector::create("ORB");//����SIFT������������ɸĳ�SURF/ORB
	Ptr<DescriptorExtractor> descriptor_extractor = DescriptorExtractor::create("ORB");//���������������������ɸĳ�SURF/ORB
	Ptr<DescriptorMatcher> descriptor_matcher = DescriptorMatcher::create("BruteForce");//��������ƥ����
	if( detector.empty() || descriptor_extractor.empty()) {
		LOGD("fail to create detector!");
	}
	*/

	bool try_use_gpu = false;
	int filenum;
	vector<Mat> imgs;
	string result_name = "result.jpg";
	clock_t start,finish;
	double totaltime;
	start=clock();

	//ORB *orb = new ORB();

	//����ͼ��
	//Mat img1 =  Mat(*(Mat*)imageGray);
	//Mat img2 = Mat(*(Mat*)tempGray);

	//cvtColor(img1,img1,CV_BGR2GRAY);  //CV_BGRA2BGR
    //cvtColor(img2,img2,CV_BGR2GRAY);  //CV_BGRA2BGR

	Mat pano;
	//img = imread(fdir[i]);
    Mat img1 = imread("/storage/emulated/0/one.jpg"); //Mat(*(Mat*)imageGray);
    Mat img2 = imread("/storage/emulated/0/two.jpg"); //Mat(*(Mat*)tempGray);
	imgs.push_back(img1);
	imgs.push_back(img2);

	Stitcher stitcher = Stitcher::createDefault(try_use_gpu);
	stitcher.setRegistrationResol(0.8);//Ϊ�˼��٣���ѡ0.1,Ĭ����0.6,���ֵ1�������˷���������������׶Σ�����Ҳ��������㣬���߰�
	//stitcher.setSeamEstimationResol(0.1);//Ĭ����0.1
	//stitcher.setCompositingResol(-1);//Ĭ����-1��������������׶Σ��Ҳ���������Ļ�����-1
	stitcher.setPanoConfidenceThresh(1);//Ĭ����1,��������0.6��0.4��
	stitcher.setWaveCorrection(false);//Ĭ����true��Ϊ����ѡfalse����ʾ����WaveCorrection����
	//stitcher.setWaveCorrectKind(detail::WAVE_CORRECT_HORIZ);//������ѡdetail::WAVE_CORRECT_VERT ,��������(wave correction)���ܣ�ˮƽ����/��ֱ��������������ΪsetWaveCorrection���false�������û��

	//��������surf�㷨�����㷨��������,���Ը����˶������š�����Ӱ�������½�Ϊ�ȶ�
	detail::OrbFeaturesFinder *featureFinder = new detail::OrbFeaturesFinder(); //SurfFeaturesFinder
	stitcher.setFeaturesFinder(featureFinder);

	//��������ORB�㷨,���Ƿ��ֲݵ�����ͼ������㷨�������ƴ��
	//detail::OrbFeaturesFinder *featureFinder = new detail::OrbFeaturesFinder();
	//stitcher.setFeaturesFinder(featureFinder);

	//Features matcher which finds two best matches for each feature and leaves the best one only if the ratio between descriptor distances is greater than the threshold match_conf.
	detail::BestOf2NearestMatcher *matcher = new detail::BestOf2NearestMatcher(false,
			0.80f/*=match_confĬ����0.65����ѡ0.8��ѡ̫���˾�û��������,0.8��ʧ����*/);
	stitcher.setFeaturesMatcher(matcher);

	// Rotation Estimation,It takes features of all images, pairwise matches between all images and estimates rotations of all cameras.
	//Implementation of the camera parameters refinement algorithm which minimizes sum of the distances between the rays passing through the camera center and a feature,�����ʱ��
	stitcher.setBundleAdjuster(new detail::BundleAdjusterRay());
	//Implementation of the camera parameters refinement algorithm which minimizes sum of the reprojection error squares.
	//stitcher.setBundleAdjuster(new detail::BundleAdjusterReproj());

	//Seam Estimation
	//Minimum graph cut-based seam estimator
	//stitcher.setSeamFinder(new detail::GraphCutSeamFinder(detail::GraphCutSeamFinderBase::COST_COLOR));//Ĭ�Ͼ������
	//stitcher.setSeamFinder(new detail::GraphCutSeamFinder(detail::GraphCutSeamFinderBase::COST_COLOR_GRAD));//GraphCutSeamFinder�ĵڶ�����ʽ
	//ɶSeamFinderҲ���ã�Stub seam estimator which does nothing.
	stitcher.setSeamFinder(new detail::NoSeamFinder);
	//Voronoi diagram-based seam estimator.
	//stitcher.setSeamFinder(new detail::VoronoiSeamFinder);

	//exposure compensators�عⲹ��
	//stitcher.setExposureCompensator(new detail::BlocksGainCompensator());//Ĭ�ϵľ������
	//��Ҫ�عⲹ��
	stitcher.setExposureCompensator(new detail::NoExposureCompensator());
	//Exposure compensator which tries to remove exposure related artifacts by adjusting image intensities
	//stitcher.setExposureCompensator(new detail::detail::GainCompensator());
	//Exposure compensator which tries to remove exposure related artifacts by adjusting image block intensities
	//stitcher.setExposureCompensator(new detail::detail::BlocksGainCompensator());

	//Image Blenders
	//Blender which uses multi-band blending algorithm
	//stitcher.setBlender(new detail::MultiBandBlender(try_use_gpu));//Ĭ�ϵ������
	//Simple blender which mixes images at its borders
	stitcher.setBlender(new detail::FeatherBlender());//����򵥣���ʱ��

	//���棿����ORƽ�棿Ĭ��Ϊ����
	PlaneWarper* cw = new PlaneWarper();
	//SphericalWarper* cw = new SphericalWarper();
	//CylindricalWarper* cw = new CylindricalWarper();
	stitcher.setWarper(cw);

	/*
	Stitcher::Status status = stitcher.estimateTransform(imgs);
	if (status != Stitcher::OK)
	{
		LOGD("Can't stitch images, error code = %d", int(status));
	    return -1;
	}
	status = stitcher.composePanorama(pano);
	*/
	Stitcher::Status status = stitcher.stitch(imgs, pano);

	if (status != Stitcher::OK)
	{
		LOGD("Can't stitch images, error code = %d", int(status));
	    return -1;
	}
	LOGD("����ʼ");
	//imwrite(result_name, pano);
	finish=clock();
	totaltime=(double)(finish-start)/CLOCKS_PER_SEC;
	LOGD("\n�˳��������ʱ��Ϊ %d  �룡 ",totaltime);

	LOGD("           doBlending1 ee         ");
	LOGD("           doBlending1 ee         ");

	//Mat mtx(image_final,0);
	Mat *hist = new Mat(pano);
	return (jlong) hist;
}

#ifdef __cplusplus
}
#endif
