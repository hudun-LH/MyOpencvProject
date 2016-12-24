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
	initModule_features2d();//初始化模块，使用SIFT或SURF时用到
	Ptr<FeatureDetector> detector = FeatureDetector::create("ORB");//创建SIFT特征检测器，可改成SURF/ORB
	Ptr<DescriptorExtractor> descriptor_extractor = DescriptorExtractor::create("ORB");//创建特征向量生成器，可改成SURF/ORB
	Ptr<DescriptorMatcher> descriptor_matcher = DescriptorMatcher::create("BruteForce");//创建特征匹配器
	if( detector.empty() || descriptor_extractor.empty()) {
		LOGD("fail to create detector!");
	}
	*/
	ORB *orb = new ORB();

	//读入图像  
	Mat img1 =  Mat(*(Mat*)imageGray);
	Mat img2 = Mat(*(Mat*)tempGray);

	cvtColor(img1,img1,CV_BGR2GRAY);  //CV_BGRA2BGR
	cvtColor(img2,img2,CV_BGR2GRAY);  //CV_BGRA2BGR

	//特征点检测  
	double t = getTickCount();//当前滴答数  
	vector<KeyPoint> m_LeftKey,m_RightKey;  
	//detector->detect( img1, m_LeftKey );//检测img1中的SIFT特征点，存储到m_LeftKey中
	//detector->detect( img2, m_RightKey );
	orb->detect(img1, m_LeftKey);
	orb->detect(img2, m_RightKey);
	LOGD("img1 KeyPoint: %d",m_LeftKey.size());
	LOGD("img2 KeyPoint: %d",m_RightKey.size());

	//根据特征点计算特征描述子矩阵，即特征向量矩阵  
	Mat descriptors1,descriptors2;  
	//BriefDescriptorExtractor
	//descriptor_extractor->compute( img1, m_LeftKey, descriptors1 );
	//descriptor_extractor->compute( img2, m_RightKey, descriptors2 );
	orb->compute(img1, m_LeftKey, descriptors1 );
    orb->compute(img2, m_RightKey, descriptors2 );
	t = ((double)getTickCount() - t)/getTickFrequency();  
	LOGD("SIFT time: %f 秒", t);

	//descriptors1.size()
	LOGD("图像1特征描述矩阵大小：特征向量个数 %d, 维数：%d", descriptors1.rows, descriptors1.cols);
	LOGD("图像2特征描述矩阵大小：特征向量个数 %d, 维数：%d", descriptors2.rows, descriptors2.cols);

	//画出特征点  
	//Mat img_m_LeftKey,img_m_RightKey;
	//drawKeypoints(img1,m_LeftKey,img_m_LeftKey,Scalar::all(-1),0);
	//drawKeypoints(img2,m_RightKey,img_m_RightKey,Scalar::all(-1),0);

	////imshow("Src1",img_m_LeftKey);
	////imshow("Src2",img_m_RightKey);

	//特征匹配   FlannBasedMatcher
	BFMatcher *descriptor_matcher = new BFMatcher();  //BruteForceMatcher
	//descriptor_matcher->normType = NORM_HAMMING;
	//enum { NORM_INF=1, NORM_L1=2, NORM_L2=4, NORM_L2SQR=5, NORM_HAMMING=6, NORM_HAMMING2=7, NORM_TYPE_MASK=7, NORM_RELATIVE=8, NORM_MINMAX=32 };
	vector<DMatch> matches;//匹配结果  
	descriptor_matcher->match( descriptors1, descriptors2, matches );//匹配两个图像的特征矩阵  
	LOGD("Match: %d",matches.size());

	//计算匹配结果中距离的最大和最小值  
	//距离是指两个特征向量间的欧式距离，表明两个特征的差异，值越小表明两个特征点越接近  
	//SIFT算法原文作者的思路，每个特征点产生一个128维的向量，
	//计算向量之间的欧式距离，采用最近比次近的方式完成匹配，如果最近距离比上次近距离小于0.8，则认为这是一个正确的匹配， 否则认为匹配不成功

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

	//筛选出较好的匹配点  
	vector<DMatch> goodMatches;  
	for(int i=0; i<matches.size(); i++)  
	{  
		if(matches[i].distance < 4*min_dist) //0.8 * max_dist
		{  
			goodMatches.push_back(matches[i]);  
		}  
	}  
	LOGD("goodMatch: %d",goodMatches.size());

	//画出匹配结果  
	Mat img_matches;  
	//红色连接的是匹配的特征点对，绿色是未匹配的特征点  
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
	initModule_features2d();//初始化模块，使用SIFT或SURF时用到
	Ptr<FeatureDetector> detector = FeatureDetector::create("ORB");//创建SIFT特征检测器，可改成SURF/ORB
	Ptr<DescriptorExtractor> descriptor_extractor = DescriptorExtractor::create("ORB");//创建特征向量生成器，可改成SURF/ORB
	Ptr<DescriptorMatcher> descriptor_matcher = DescriptorMatcher::create("BruteForce");//创建特征匹配器
	if( detector.empty() || descriptor_extractor.empty()) {
		LOGD("fail to create detector!");
	}
	*/

	ORB *orb = new ORB();

	//读入图像
	Mat img1 =  Mat(*(Mat*)imageGray);
	Mat img2 = Mat(*(Mat*)tempGray);

	cvtColor(img1,img1,CV_BGR2GRAY);  //CV_BGRA2BGR
    cvtColor(img2,img2,CV_BGR2GRAY);  //CV_BGRA2BGR

	//特征点检测
	double t = getTickCount();//当前滴答数
	vector<KeyPoint> m_LeftKey,m_RightKey;
	//detector->detect( img1, m_LeftKey );//检测img1中的SIFT特征点，存储到m_LeftKey中
	//detector->detect( img2, m_RightKey );
	orb->detect(img1, m_LeftKey);
	orb->detect(img2, m_RightKey);
	LOGD("img1 KeyPoint: %d",m_LeftKey.size());
	LOGD("img2 KeyPoint: %d",m_RightKey.size());

	//根据特征点计算特征描述子矩阵，即特征向量矩阵
	Mat descriptors1,descriptors2;
	//BriefDescriptorExtractor
	//descriptor_extractor->compute( img1, m_LeftKey, descriptors1 );
	//descriptor_extractor->compute( img2, m_RightKey, descriptors2 );
	orb->compute(img1, m_LeftKey, descriptors1 );
    orb->compute(img2, m_RightKey, descriptors2 );
	t = ((double)getTickCount() - t)/getTickFrequency();
	LOGD("SIFT time: %f 秒", t);

	//descriptors1.size()
	LOGD("图像1特征描述矩阵大小：特征向量个数 %d, 维数：%d", descriptors1.rows, descriptors1.cols);
	LOGD("图像2特征描述矩阵大小：特征向量个数 %d, 维数：%d", descriptors2.rows, descriptors2.cols);

	//画出特征点
	//Mat img_m_LeftKey,img_m_RightKey;
	//drawKeypoints(img1,m_LeftKey,img_m_LeftKey,Scalar::all(-1),0);
	//drawKeypoints(img2,m_RightKey,img_m_RightKey,Scalar::all(-1),0);

	////imshow("Src1",img_m_LeftKey);
	////imshow("Src2",img_m_RightKey);

	//特征匹配   FlannBasedMatcher
	BFMatcher *descriptor_matcher = new BFMatcher();  //BruteForceMatcher
	//descriptor_matcher->normType = NORM_HAMMING;
	//enum { NORM_INF=1, NORM_L1=2, NORM_L2=4, NORM_L2SQR=5, NORM_HAMMING=6, NORM_HAMMING2=7, NORM_TYPE_MASK=7, NORM_RELATIVE=8, NORM_MINMAX=32 };
	vector<DMatch> matches;//匹配结果
	descriptor_matcher->match( descriptors1, descriptors2, matches );//匹配两个图像的特征矩阵
	LOGD("Match: %d",matches.size());

	//计算匹配结果中距离的最大和最小值
	//距离是指两个特征向量间的欧式距离，表明两个特征的差异，值越小表明两个特征点越接近
	//SIFT算法原文作者的思路，每个特征点产生一个128维的向量，
	//计算向量之间的欧式距离，采用最近比次近的方式完成匹配，如果最近距离比上次近距离小于0.8，则认为这是一个正确的匹配， 否则认为匹配不成功

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

	//筛选出较好的匹配点
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
	//画出匹配结果
	Mat img_matches;
	//红色连接的是匹配的特征点对，绿色是未匹配的特征点
	drawMatches(img1,m_LeftKey,img2,m_RightKey,goodMatches,img_matches,
		Scalar::all(-1),CV_RGB(0,255,0),Mat(),2);   //CV_RGB(255,0,0)
    */

	//imshow("MatchSIFT",img_matches);
	//IplImage result=img_matches;

	//RANSAC匹配过程
	vector<DMatch> m_Matches=goodMatches;
	// 分配空间
	int ptCount = (int)m_Matches.size();
	Mat p1(ptCount, 2, CV_32F);
	Mat p2(ptCount, 2, CV_32F);

	// 把Keypoint转换为Mat
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

	// 用RANSAC方法计算F
	Mat m_Fundamental;
	vector<uchar> m_RANSACStatus;       // 这个变量用于存储RANSAC后每个点的状态
	//RASIC方法计算基础矩阵，并细化匹配结果
	/*
	CV_EXPORTS_W Mat findFundamentalMat( InputArray points1, InputArray points2,
	                                     int method=FM_RANSAC,
	                                     double param1=3., double param2=0.99,
	                                     OutputArray mask=noArray());
     points1，points2 两幅图像间相匹配的点，点的坐标要是浮点数（float或者double）
           第三个参数method是用来计算基础矩阵的具体方法，是一个枚举值。
     param1,param2保持默认值即可。
           主要来说下mask参数，有N个匹配点用来计算基础矩阵，则该值有N个元素，每个元素的值为0或者1.
           值为0时，代表该匹配点事错误的匹配（离群值），只在使用RANSAC和LMeds方法时该值有效，
           可以使用该值来删除错误的匹配。
	*/
	/*
	 RANSAC是“RANdom SAmple Consensus（随机抽样一致）”的缩写。
	 它可以从一组包含“局外点”的观测数据集中，通过迭代方式估计数学模型的参数。
	 它是一种不确定的算法——它有一定的概率得出一个合理的结果；
	 为了提高概率必须提高迭代次数。该算法最早由Fischler和Bolles于1981年提出。
     RANSAC的基本假设是：（1）数据由“局内点”组成，例如：数据的分布可以用一些模型参数来解释；
         （2）“局外点”是不能适应该模型的数据；
         （3）除此之外的数据属于噪声。局外点产生的原因有：噪声的极值；错误的测量方法；对数据的错误假设
	 * */
	findFundamentalMat(p1, p2, m_RANSACStatus, FM_RANSAC); //findFundamentalMat函数消除错误匹配
	//基础矩阵和变换矩阵是两个不同的概念。基础矩阵描述是三维场景中的像点之间的对应关系

	LOGD("m_RANSACStatus: %d",m_RANSACStatus.size());
	// 计算野点个数

	int OutlinerCount = 0;
	for (int i=0; i<ptCount; i++)
	{
		if (m_RANSACStatus[i] == 0)    // 状态为0表示野点
		{
			OutlinerCount++;
		}
	}
	int InlinerCount = ptCount - OutlinerCount;   // 计算内点
	LOGD("Inliner Count: %d",InlinerCount);

	
   // 这三个变量用于保存内点和匹配关系
   vector<Point2f> m_LeftInlier;
   vector<Point2f> m_RightInlier;
   vector<DMatch> m_InlierMatches;

	m_InlierMatches.resize(InlinerCount);
	m_LeftInlier.resize(InlinerCount);
	m_RightInlier.resize(InlinerCount);
	InlinerCount=0;
	float inlier_minRx=img1.cols;        //用于存储内点中右图最小横坐标，以便后续融合

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
				inlier_minRx=m_RightInlier[InlinerCount].x;   //存储内点中右图最小横坐标

			InlinerCount++;
		}
	}

	// 把内点转换为drawMatches可以使用的格式
	vector<KeyPoint> key1(InlinerCount);
	vector<KeyPoint> key2(InlinerCount);
	KeyPoint::convert(m_LeftInlier, key1);
	KeyPoint::convert(m_RightInlier, key2);

    // 显示计算F过后的内点匹配
	Mat OutImage;
	drawMatches(img1, key1, img2, key2, m_InlierMatches, OutImage);
	//drawMatches(img1, key1, img2, key2, m_InlierMatches,OutImage,
	//		Scalar::all(-1)/*CV_RGB(255,0,0)*/,CV_RGB(0,255,0),Mat(),2);

	//cvNamedWindow( "Match features", 1);
	//cvShowImage("Match features", &IplImage(OutImage));

	//矩阵H用以存储RANSAC得到的单应矩阵   == 用RANSAC随机采样求透视矩阵
	//RASIC方法计算基础矩阵，并细化匹配结果
	//计算单应矩阵H，并细化匹配结果
	/*
	CV_EXPORTS_W Mat findHomography( InputArray srcPoints, InputArray dstPoints,
                                  int method=0, double ransacReprojThreshold=3,
                                  OutputArray mask=noArray());
	                              srcPoints,dstPoints是两视图中匹配的点
	                              method 是计算单应矩阵所使用的方法，是一个枚举值 , CV_FM_RANSAC。
	                              ransacReprojThreshold 是允许的最大反投影错误，只在使用RANSAC方法时有效。
	                              mask 同findFundamentalMat 类似，指出匹配的点是不是离群值，用来优化匹配结果。
	*/
	Mat H = findHomography( m_LeftInlier, m_RightInlier, RANSAC );//函数findHomography，这个函数才是真正的计算变换矩阵的函数

	LOGD("       findHomography       ");

	//存储左图四角，及其变换到右图位置
	vector<Point2f> obj_corners(4);
	obj_corners[0] = Point(0,0);
	obj_corners[1] = Point( img1.cols, 0 );
	obj_corners[2] = Point( img1.cols, img1.rows );
	obj_corners[3] = Point( 0, img1.rows );
	vector<Point2f> scene_corners(4);
	perspectiveTransform( obj_corners, scene_corners, H);

	//画出变换后图像位置
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

    //2 寻找class里面的方法
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

	int drift = scene_corners[1].x; //储存偏移量

	LOGD("scene_corners[].x: %f, %f, %f, %f",scene_corners[1].x, scene_corners[2].x, scene_corners[3].x, scene_corners[4].x);

	//新建一个矩阵存储配准后四角的位置

	int width = int(max(abs(scene_corners[1].x), abs(scene_corners[2].x)));
	int height= img1.rows;

	LOGD("00 abs(scene_corners[1/2/3/4].x): %f, %f, %f, %f", abs(scene_corners[0].x), abs(scene_corners[1].x),
			abs(scene_corners[2].x), abs(scene_corners[3].x));
	LOGD("00 scene_corners[1/2/3/4].x: %f, %f, %f, %f", scene_corners[0].x, scene_corners[1].x,
				scene_corners[2].x, scene_corners[3].x);
	LOGD("width: %d, height = %d",width, height);

	//或者：int height = int(max(abs(scene_corners[2].y), abs(scene_corners[3].y)));
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

	//可选：height-=int(origin_y);
	LOGD("img1.cols: %d, img1.rows :%d, img1.type() = %d", img1.cols, img1.rows, img1.type());
	LOGD("width: %d, height = %d, %f, %f",width, height, origin_x, origin_y);

	Mat imageturn;//=Mat::zeros(width, height, img1.type());//width

	//获取新的变换矩阵，使图像完整显示
	for (int i=0;i<4;i++){
		scene_corners[i].x -= origin_x; //可选：scene_corners[i].y -= (float)origin_y; }
	}

	LOGD("22 abs(scene_corners[1/2/3/4].x): %f, %f, %f, %f", abs(scene_corners[0].x), abs(scene_corners[1].x),
				abs(scene_corners[2].x), abs(scene_corners[3].x));
	LOGD("22 scene_corners[1/2/3/4].x: %f, %f, %f, %f", scene_corners[0].x, scene_corners[1].x,
					scene_corners[2].x, scene_corners[3].x);
	/*
	//由四边形的4个点计算透射变换
	CvMat* cvGetPerspectiveTransform( const CvPoint2D32f* src, const CvPoint2D32f* dst,
	                                  CvMat* map_matrix );

	#define cvWarpPerspectiveQMatrix cvGetPerspectiveTransform
	src
	输入图像的四边形顶点坐标。
	dst
	输出图像的相应的四边形顶点坐标。
	map_matrix
	指向3×3输出矩阵的指针。
	函数cvGetPerspectiveTransform计算满足以下关系的透射变换矩阵：
	*/

	Mat H1=getPerspectiveTransform(obj_corners, scene_corners);

	//进行图像变换，显示效果
/*
C++: void warpPerspective(InputArray src, OutputArray dst, InputArray M,
Size dsize, int flags=INTER_LINEAR, int borderMode=BORDER_CONSTANT, const Scalar& borderValue=Scalar())
参数详解：
InputArray src：输入的图像
OutputArray dst：输出的图像
InputArray M：透视变换的矩阵
Size dsize：输出图像的大小
int flags=INTER_LINEAR：输出图像的插值方法，
combination of interpolation methods (INTER_LINEAR or INTER_NEAREST)
and the optional flagWARP_INVERSE_MAP, that sets M as the inverse transformation ( \texttt{dst}\rightarrow\texttt{src} )
int borderMode=BORDER_CONSTANT：图像边界的处理方式
const Scalar& borderValue=Scalar()：边界的颜色设置，一般默认是0
*/
	LOGD("imageturn.cols: %d, imageturn.rows = %d",imageturn.cols, imageturn.rows);
	LOGD("img1.cols: %d, img1.rows = %d",img1.cols, img1.rows);

	warpPerspective(img1, imageturn, H1, Size(width,height));
	//imshow("image_Perspective", imageturn);
	LOGD("imageturn.cols: %d, imageturn.rows = %d",imageturn.cols, imageturn.rows);

	//图像融合
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
	initModule_features2d();//初始化模块，使用SIFT或SURF时用到
	Ptr<FeatureDetector> detector = FeatureDetector::create("ORB");//创建SIFT特征检测器，可改成SURF/ORB
	Ptr<DescriptorExtractor> descriptor_extractor = DescriptorExtractor::create("ORB");//创建特征向量生成器，可改成SURF/ORB
	Ptr<DescriptorMatcher> descriptor_matcher = DescriptorMatcher::create("BruteForce");//创建特征匹配器
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

	//读入图像
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
	stitcher.setRegistrationResol(0.8);//为了加速，我选0.1,默认是0.6,最大值1最慢，此方法用于特征点检测阶段，如果找不到特征点，调高吧
	//stitcher.setSeamEstimationResol(0.1);//默认是0.1
	//stitcher.setCompositingResol(-1);//默认是-1，用于特征点检测阶段，找不到特征点的话，改-1
	stitcher.setPanoConfidenceThresh(1);//默认是1,见过有设0.6和0.4的
	stitcher.setWaveCorrection(false);//默认是true，为加速选false，表示跳过WaveCorrection步骤
	//stitcher.setWaveCorrectKind(detail::WAVE_CORRECT_HORIZ);//还可以选detail::WAVE_CORRECT_VERT ,波段修正(wave correction)功能（水平方向/垂直方向修正）。因为setWaveCorrection设的false，此语句没用

	//找特征点surf算法，此算法计算量大,但对刚体运动、缩放、环境影响等情况下较为稳定
	detail::OrbFeaturesFinder *featureFinder = new detail::OrbFeaturesFinder(); //SurfFeaturesFinder
	stitcher.setFeaturesFinder(featureFinder);

	//找特征点ORB算法,但是发现草地这组图，这个算法不能完成拼接
	//detail::OrbFeaturesFinder *featureFinder = new detail::OrbFeaturesFinder();
	//stitcher.setFeaturesFinder(featureFinder);

	//Features matcher which finds two best matches for each feature and leaves the best one only if the ratio between descriptor distances is greater than the threshold match_conf.
	detail::BestOf2NearestMatcher *matcher = new detail::BestOf2NearestMatcher(false,
			0.80f/*=match_conf默认是0.65，我选0.8，选太大了就没特征点啦,0.8都失败了*/);
	stitcher.setFeaturesMatcher(matcher);

	// Rotation Estimation,It takes features of all images, pairwise matches between all images and estimates rotations of all cameras.
	//Implementation of the camera parameters refinement algorithm which minimizes sum of the distances between the rays passing through the camera center and a feature,这个耗时短
	stitcher.setBundleAdjuster(new detail::BundleAdjusterRay());
	//Implementation of the camera parameters refinement algorithm which minimizes sum of the reprojection error squares.
	//stitcher.setBundleAdjuster(new detail::BundleAdjusterReproj());

	//Seam Estimation
	//Minimum graph cut-based seam estimator
	//stitcher.setSeamFinder(new detail::GraphCutSeamFinder(detail::GraphCutSeamFinderBase::COST_COLOR));//默认就是这个
	//stitcher.setSeamFinder(new detail::GraphCutSeamFinder(detail::GraphCutSeamFinderBase::COST_COLOR_GRAD));//GraphCutSeamFinder的第二种形式
	//啥SeamFinder也不用，Stub seam estimator which does nothing.
	stitcher.setSeamFinder(new detail::NoSeamFinder);
	//Voronoi diagram-based seam estimator.
	//stitcher.setSeamFinder(new detail::VoronoiSeamFinder);

	//exposure compensators曝光补偿
	//stitcher.setExposureCompensator(new detail::BlocksGainCompensator());//默认的就是这个
	//不要曝光补偿
	stitcher.setExposureCompensator(new detail::NoExposureCompensator());
	//Exposure compensator which tries to remove exposure related artifacts by adjusting image intensities
	//stitcher.setExposureCompensator(new detail::detail::GainCompensator());
	//Exposure compensator which tries to remove exposure related artifacts by adjusting image block intensities
	//stitcher.setExposureCompensator(new detail::detail::BlocksGainCompensator());

	//Image Blenders
	//Blender which uses multi-band blending algorithm
	//stitcher.setBlender(new detail::MultiBandBlender(try_use_gpu));//默认的是这个
	//Simple blender which mixes images at its borders
	stitcher.setBlender(new detail::FeatherBlender());//这个简单，耗时少

	//柱面？球面OR平面？默认为球面
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
	LOGD("程序开始");
	//imwrite(result_name, pano);
	finish=clock();
	totaltime=(double)(finish-start)/CLOCKS_PER_SEC;
	LOGD("\n此程序的运行时间为 %d  秒！ ",totaltime);

	LOGD("           doBlending1 ee         ");
	LOGD("           doBlending1 ee         ");

	//Mat mtx(image_final,0);
	Mat *hist = new Mat(pano);
	return (jlong) hist;
}

#ifdef __cplusplus
}
#endif
