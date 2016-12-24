#include <android/log.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <jni.h>
#include <android/log.h>
#include <iostream>
#include <fstream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgproc/imgproc_c.h>
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

#define BYTE unsigned char

void HoughLine(IplImage* pImg)  //Hough变换提取直线
{
	IplImage* pGray=cvCreateImage(cvSize(pImg->width,pImg->height),IPL_DEPTH_8U,1);
	cvCvtColor(pImg,pGray,CV_BGR2GRAY);//提取图像边缘点
	cvCanny(pGray,pGray,10,150);
	CvMemStorage* pMem=cvCreateMemStorage(0);//分配存储直线的内存
	CvSeq* pLineSeq=cvHoughLines2(pGray,pMem,CV_HOUGH_PROBABILISTIC,1,CV_PI/180,100,100,30);//Hough变换提取直线
	if (pLineSeq)
	{
		int i;
		for (i=0;i<MIN(pLineSeq->total,100);i++)//显示不超过100条的直线
		{
			CvPoint* line=(CvPoint*)cvGetSeqElem(pLineSeq,i);//获取直线参数
			cvLine(pImg,line[0],line[1],CV_RGB(255,0,0),2);//显示直线
		}
	}
	cvReleaseMemStorage(&pMem);//释放内存
	//cvShowImage("Canny Edge",pGray);///显示边缘点图像
	cvReleaseImage(&pGray);
}


void HoughCircle(IplImage* pImg)   //Hough变换提取圆
{
    IplImage* pGray=cvCreateImage(cvSize(pImg->width,pImg->height),IPL_DEPTH_8U,1);
    cvCvtColor(pImg,pGray,CV_BGR2GRAY);   //转换为灰度图像
    cvSmooth(pGray,pGray,CV_GAUSSIAN);   //高斯平滑滤波
    int nCannyThresholdMax=150;   //canny算子的高阈值
    CvMemStorage* pMem=cvCreateMemStorage(0);   //存储圆参数的内存
    CvSeq* pCircleSeq=cvHoughCircles(pGray,pMem,CV_HOUGH_GRADIENT,2,pGray->width/10,nCannyThresholdMax,100,pGray->width/10,pGray->width/3);
	                             						 
    for (int i=0;i<MIN(10,pCircleSeq->total);i++)   //最多存储十个圆
    {
	 float* p=(float*)cvGetSeqElem(pCircleSeq,i);
	 cvCircle(pImg,cvPoint(p[0]+0.5,p[1]+0.5),(int)(p[2]+0.5),CV_RGB(255,0,255),3);
	}

    cvReleaseMemStorage(&pMem);   //释放内存
    cvCanny(pGray,pGray,nCannyThresholdMax/2,nCannyThresholdMax);
    //cvShowImage("Canny Circle",pGray);   //同步显示边缘点
    cvReleaseImage(&pGray);
}


void EqualizeGray(IplImage* pImg)  //直方图均衡化
{
	IplImage* pGray=cvCreateImage(cvSize(pImg->width,pImg->height),IPL_DEPTH_8U,1);
	cvCvtColor(pImg,pGray,CV_BGR2GRAY);
	pGray->origin=pImg->origin;
	//cvShowImage("Gray Image",pGray);

	IplImage* pEqualizedImg=cvCreateImage(cvSize(pImg->width,pImg->height),IPL_DEPTH_8U,1);
	cvEqualizeHist(pGray, pEqualizedImg);
	//cvShowImage("Equalize Image",pEqualizedImg);
	cvReleaseImage(&pGray);
}


void cvFindContours2(IplImage* pImg,int r,int g,int b,int n)   //颜色识别 轮廓检测
{
	IplImage* pImg1=cvCreateImage(cvSize(pImg->width,pImg->height),IPL_DEPTH_8U,3);
	IplImage* pGray=cvCreateImage(cvSize(pImg->width,pImg->height),IPL_DEPTH_8U,1);
	int rr,gg,bb;
	int i,j;
	for (i=0;i<pImg1->height;i++)
		for (j=0;j<pImg1->width;j++)
		{
			bb=(BYTE)(pImg->imageData[i*pImg->widthStep+j*pImg->nChannels]);   //把pImg三颜色通道值分别赋给bb、gg、rr
			gg=(BYTE)(pImg->imageData[i*pImg->widthStep+j*pImg->nChannels+1]);
			rr=(BYTE)(pImg->imageData[i*pImg->widthStep+j*pImg->nChannels+2]);  
		    if (rr==r&&gg==g&&bb==b)
			{
				pImg1->imageData[i*pImg1->widthStep+j*pImg1->nChannels]=0;   //如果bb、gg、rr与所给b、g、r的值相等，即为指定颜色，则使其变黑
			    pImg1->imageData[i*pImg1->widthStep+j*pImg1->nChannels+1]=0;
		     	pImg1->imageData[i*pImg1->widthStep+j*pImg1->nChannels+2]=0;
		    }
		    else 
		    {
			    pImg1->imageData[i*pImg1->widthStep+j*pImg1->nChannels]=-1;   //否则（即图像颜色与所给颜色不同），则是其变白
			    pImg1->imageData[i*pImg1->widthStep+j*pImg1->nChannels+1]=-1;
			    pImg1->imageData[i*pImg1->widthStep+j*pImg1->nChannels+2]=-1;
		    }
        }
	cvCvtColor(pImg1,pGray,CV_BGR2GRAY);
	//cvShowImage("gray",pGray);   //显示颜色检测的结果

	//轮廓检测
	CvMemStorage* storage=cvCreateMemStorage(0);   	//跟踪轮廓点
	CvSeq* contour=NULL;
	int number_of_c=cvFindContours(pGray,storage,&contour);

	//处理所有轮廓
	for (int i=0;i<number_of_c;i++,contour=contour->h_next)
	{
		if (contour==NULL)
			break;
		//保留CV_SEQ_KIND_CURVE型的
		if (!CV_IS_SEQ_CURVE(contour))
			continue;
		//多边形逼近
/*
CVAPI(CvSeq*)  cvApproxPoly( const void* src_seq,
                             int header_size, CvMemStorage* storage,
                             int method, double parameter,
                             int parameter2 CV_DEFAULT(0));
使用多边形去逼近轮廓，使顶点数目变少。
CvSeq*             返回值对应第一个 轮廓（可用h_next和v_next访问其他轮廓）
src_seq            要处理的目标序列
header_size    为返回结果指定头结构大小
storage             为返回结果指定新的内存存储器
method             算法：目前只有CV_POLY_APPOX_DP
parameter        逼近算法参数，指定逼近精度，曲线1、n
parameter2      逼近算法参数。若为0，只处理src_seq指向的轮廓。1则处理整个双向链表中的所有轮廓
*/
		CvSeq* pResult=cvApproxPoly(contour,sizeof(CvContour),storage,CV_POLY_APPROX_DP,cvContourPerimeter(contour)*0.03,0);

		int nPtNumPerPolygon=n;//边的数目是多边形条件判断之一   把边的数目n提出来有利于后续利用，提高程序的灵活性和鲁棒性
		if (pResult->total!=nPtNumPerPolygon)
			continue;
		//绘制多边形 
		for (int j=0;j<nPtNumPerPolygon;j++)
		{
			CvPoint ptS,ptE;
			ptS=*(CvPoint*)cvGetSeqElem(pResult,j);//直线起点
			ptE=*(CvPoint*)cvGetSeqElem(pResult,(j+1)%nPtNumPerPolygon);//直线终点
			cvLine(pImg,ptS,ptE,CV_RGB(0,255,255),3);   //在原图像上画出其边界
		}
	}
	//释放内存，注意先后顺序
	if (contour)
		cvReleaseMemStorage(&contour->storage);
	if (storage)
		cvReleaseMemStorage(&storage);
	cvReleaseImage(&pGray);
	cvReleaseImage(&pImg1);
}


int _tmain(int argc, const char* argv[])
{
/* 
	//图像处理
    IplImage *pImg0=cvLoadImage("C:\\Users\\shuan\\Desktop\\课件\\计算机视觉\\04_例子程序\\例子图像\\32.jpg",1);  //0为灰度，1为彩色
	if(pImg0==NULL)
		return 1;
	EqualizeGray(pImg0);
	cvShowImage("lena 原始",pImg0);
	IplImage *pImgDes0 = cvCreateImage(cvSize(pImg0->width,pImg0->height),IPL_DEPTH_16S,pImg0->nChannels);
	cvSobel(pImg0,pImgDes0,0,1,3);
	cvConvertScaleAbs(pImgDes0,pImg0);
	cvShowImage("lena Sobel：0,1,3",pImg0);

	IplImage *pImg=cvLoadImage("C:\\Users\\shuan\\Desktop\\课件\\计算机视觉\\04_例子程序\\例子图像\\shan.bmp",1);  //0为灰度，1为彩色
	if(pImg==NULL)
		return 1;
	cvShowImage("shan 原始",pImg);
	IplImage *pImgDes = cvCreateImage(cvSize(pImg->width,pImg->height),IPL_DEPTH_16S,pImg->nChannels);
	cvSobel(pImg,pImgDes,0,1,3);
	cvConvertScaleAbs(pImgDes,pImg);
	cvShowImage("shan Sobel：0,1,3",pImg);

	IplImage *pImg3=cvLoadImage("C:\\Users\\shuan\\Desktop\\课件\\计算机视觉\\04_例子程序\\例子图像\\shan.bmp",0);  //0为灰度，1为彩色
	if(pImg3==NULL)
		return 1;
	cvCanny(pImg3,pImg3,10,150,3);   //canny算子只能处理灰度图像，3参数分别为sigma，dRatHigh，dRatLow
	cvShowImage("shan Canny：10,150,3",pImg3);

	IplImage *pImg4=cvLoadImage("C:\\Users\\shuan\\Desktop\\课件\\计算机视觉\\04_例子程序\\例子图像\\lena.jpg",0);  //0为灰度，1为彩色
	if(pImg4==NULL)
		return 1;
	cvCanny(pImg4,pImg4,10,150,3);
	cvShowImage("lena Canny：10,150,3",pImg4);

	IplImage *pImg5=cvLoadImage("C:\\Users\\shuan\\Desktop\\课件\\计算机视觉\\04_例子程序\\例子图像\\lena.jpg",1);  //0为灰度，1为彩色
	if(pImg5==NULL)
		return 1;
	cvSmooth(pImg5,pImg5,CV_GAUSSIAN,13);//高斯滤波器
	cvShowImage("lena GAUSSIAN：13",pImg5);

	IplImage *pImg6=cvLoadImage("C:\\Users\\shuan\\Desktop\\课件\\计算机视觉\\04_例子程序\\例子图像\\lena.jpg",1);  //0为灰度，1为彩色
	if(pImg6==NULL)
		return 1;
	cvSmooth(pImg6,pImg6,CV_BLUR,5);//均值滤波器
	cvShowImage("lena BLUR：5",pImg6);

	IplImage *pImg7=cvLoadImage("C:\\Users\\shuan\\Desktop\\课件\\计算机视觉\\04_例子程序\\例子图像\\lena.jpg",1);  //0为灰度，1为彩色
	if(pImg7==NULL)
		return 1;
	cvSmooth(pImg7,pImg7,CV_MEDIAN,9);//中值滤波器
	cvShowImage("lena MEDIAN：9",pImg7);

	cvReleaseImage(&pImg0);   //释放图像资源
	cvReleaseImage(&pImgDes0);//释放图像资源
	cvReleaseImage(&pImg);    //释放图像资源
	cvReleaseImage(&pImgDes); //释放图像资源
	cvReleaseImage(&pImg3);   //释放图像资源
	cvReleaseImage(&pImg4);   //释放图像资源
	cvReleaseImage(&pImg5);   //释放图像资源
	cvReleaseImage(&pImg6);   //释放图像资源
	cvReleaseImage(&pImg7);   //释放图像资源
*/



//Hough变换检测直线与圆
	 CvCapture* capture=cvCaptureFromCAM(0);   //视频提取
	  if(capture==NULL)
	{ 
	   printf("没有检测到摄像头");
	   cvWaitKey(0);
	   return 0;
	}
	cvNamedWindow("video",1);

	for(;;)
	{
	 IplImage* frame=cvQueryFrame(capture);
		if (!frame)
			break;
//		cvSmooth(frame,frame,CV_BLUR,5);   //使用高斯滤波，参数应为奇数，否则报错
		if(frame->origin!=IPL_ORIGIN_TL)
			cvFlip(frame,frame,0);
// 		    HoughLine(frame);   //检测直线
// 			HoughCircle(frame);   //检测圆
// 		    EqualizeGray(frame);   //直方图均衡化
		    //runLKOpenCV(frame);   //运动物体检测
		cvShowImage("video",frame);
		if(cvWaitKey(100)>=0)   //If you hit the escape key you will exit
			break;
	}
    cvReleaseCapture(&capture);
    cvDestroyWindow("video");
    cvWaitKey(0);
    return 0;
	

/*
     //图像镶嵌
 	IplImage *pImg1=cvLoadImage("C:\\Users\\shuan\\Desktop\\课件\\计算机视觉\\提高与测试\\lena.bmp",1);  //载入图像 0为灰度，1为彩色
 	if(pImg1==NULL)
 		return 1;   
	IplImage *pImg2=cvLoadImage("C:\\Users\\shuan\\Desktop\\课件\\计算机视觉\\提高与测试\\Monkey.JPG",1);  
	if(pImg2==NULL)
		return 1;   

	IplImage* pImg3=cvCreateImage(cvSize(pImg1->width/2,pImg1->height/2),IPL_DEPTH_8U,3);  //开辟新内存   其作用是不改变源图像指针，便于后续重利用
	IplImage* pImg4=cvCreateImage(cvSize(pImg2->width,pImg2->height),IPL_DEPTH_8U,3);   

	cvPyrDown(pImg1,pImg3,CV_GAUSSIAN_5x5);  //图像赋值
	pImg4=pImg2;  
	int a,b;   //定义图像偏移量
	a=100,b=100;
	for (int i=0;i<pImg3->height;i++)
	{
		for (int j=0;j<pImg3->width*pImg3->nChannels;j++)
		{
			pImg4->imageData[(a+i)*pImg4->widthStep+j+b*pImg4->nChannels]=pImg3->imageData[i*pImg3->widthStep+j];   //左上角为原点平移 
		    pImg4->imageData[(a+i)*pImg4->widthStep+pImg4->widthStep-b*pImg4->nChannels-j]=pImg3->imageData[i*pImg3->widthStep+pImg3->widthStep-j];   //右上角为原点平移 
 		    pImg4->imageData[(pImg4->height-a-i)*pImg4->widthStep+j+b*pImg4->nChannels]=pImg3->imageData[(pImg3->height-i)*pImg3->widthStep+j];   //左下角为原点平移
 			pImg4->imageData[(pImg4->height-a-i)*pImg4->widthStep+pImg4->widthStep-b*pImg4->nChannels-j]=pImg3->imageData[(pImg3->height-i)*pImg3->widthStep+pImg3->widthStep-j];   //右下角为原点平移
		}
	}
	cvShowImage("monkey 插入 lena",pImg4);  //显示图像
*/


    //颜色检测
    IplImage* pImg=cvLoadImage("F:\\计算机视觉\\计算机视觉\\提高与测试\\彩色图形.bmp",1);
	if (pImg==NULL)
		return 1;
//	cvFindContours2(pImg,237,28,36,10);   //提取红色五角星
//	cvFindContours2(pImg,255,242,0,4);    //提取黄色方块
	cvFindContours2(pImg,0,0,255,1000);   //提取蓝色圆圈
	cvShowImage("颜色识别 轮廓检测",pImg);
	cvWaitKey(0);
	cvReleaseImage(&pImg);
	return 0;

}

int type_num=0;

JNIEXPORT jlong JNICALL Java_com_example_carplate_CarPlateDetection_doImageShow
(JNIEnv *env, jclass clz, jlong imageGray)
{
	LOGD("          doImageShow         !");
	//cvSmooth(frame,frame,CV_BLUR,5);   //使用高斯滤波，参数应为奇数，否则报错
	//if(frame->origin!=IPL_ORIGIN_TL)
	//cvFlip(frame,frame,0);

	Mat imageMat = Mat(*(Mat*)imageGray);
	IplImage  temp_src = imageMat;
	IplImage* imageg = &temp_src;

	if(type_num==0){
	    //cvFindContours2(imageg,0,0,255,1000);   //提取蓝色圆圈
		//EqualizeGray(imageg);
		    IplImage* pGray=cvCreateImage(cvSize(imageg->width,imageg->height),IPL_DEPTH_8U,1);
			cvCvtColor(imageg, pGray,CV_BGR2GRAY);
			pGray->origin=imageg->origin;
			//cvShowImage("Gray Image",pGray);

			IplImage* pEqualizedImg=cvCreateImage(cvSize(imageg->width,imageg->height),IPL_DEPTH_8U,1);
			cvEqualizeHist(pGray, pEqualizedImg);

			type_num = 1;
			LOGD("          EqualizeGray      !");
			Mat *hist = new Mat(pEqualizedImg);
			return (jlong) hist;
	}else if(type_num==1){
		HoughCircle(imageg);
	}else if(type_num==2){
		HoughLine(imageg);
	}
	type_num++;
	if(type_num>=3){
		type_num = 0;
	}
	LOGD("          type_num = %d       !",type_num);
    //Mat mtx(imageg,0);
    Mat *hist = new Mat(imageg);
    return (jlong) hist;
}

#ifdef __cplusplus
}
#endif

