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

void HoughLine(IplImage* pImg)  //Hough�任��ȡֱ��
{
	IplImage* pGray=cvCreateImage(cvSize(pImg->width,pImg->height),IPL_DEPTH_8U,1);
	cvCvtColor(pImg,pGray,CV_BGR2GRAY);//��ȡͼ���Ե��
	cvCanny(pGray,pGray,10,150);
	CvMemStorage* pMem=cvCreateMemStorage(0);//����洢ֱ�ߵ��ڴ�
	CvSeq* pLineSeq=cvHoughLines2(pGray,pMem,CV_HOUGH_PROBABILISTIC,1,CV_PI/180,100,100,30);//Hough�任��ȡֱ��
	if (pLineSeq)
	{
		int i;
		for (i=0;i<MIN(pLineSeq->total,100);i++)//��ʾ������100����ֱ��
		{
			CvPoint* line=(CvPoint*)cvGetSeqElem(pLineSeq,i);//��ȡֱ�߲���
			cvLine(pImg,line[0],line[1],CV_RGB(255,0,0),2);//��ʾֱ��
		}
	}
	cvReleaseMemStorage(&pMem);//�ͷ��ڴ�
	//cvShowImage("Canny Edge",pGray);///��ʾ��Ե��ͼ��
	cvReleaseImage(&pGray);
}


void HoughCircle(IplImage* pImg)   //Hough�任��ȡԲ
{
    IplImage* pGray=cvCreateImage(cvSize(pImg->width,pImg->height),IPL_DEPTH_8U,1);
    cvCvtColor(pImg,pGray,CV_BGR2GRAY);   //ת��Ϊ�Ҷ�ͼ��
    cvSmooth(pGray,pGray,CV_GAUSSIAN);   //��˹ƽ���˲�
    int nCannyThresholdMax=150;   //canny���ӵĸ���ֵ
    CvMemStorage* pMem=cvCreateMemStorage(0);   //�洢Բ�������ڴ�
    CvSeq* pCircleSeq=cvHoughCircles(pGray,pMem,CV_HOUGH_GRADIENT,2,pGray->width/10,nCannyThresholdMax,100,pGray->width/10,pGray->width/3);
	                             						 
    for (int i=0;i<MIN(10,pCircleSeq->total);i++)   //���洢ʮ��Բ
    {
	 float* p=(float*)cvGetSeqElem(pCircleSeq,i);
	 cvCircle(pImg,cvPoint(p[0]+0.5,p[1]+0.5),(int)(p[2]+0.5),CV_RGB(255,0,255),3);
	}

    cvReleaseMemStorage(&pMem);   //�ͷ��ڴ�
    cvCanny(pGray,pGray,nCannyThresholdMax/2,nCannyThresholdMax);
    //cvShowImage("Canny Circle",pGray);   //ͬ����ʾ��Ե��
    cvReleaseImage(&pGray);
}


void EqualizeGray(IplImage* pImg)  //ֱ��ͼ���⻯
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


void cvFindContours2(IplImage* pImg,int r,int g,int b,int n)   //��ɫʶ�� �������
{
	IplImage* pImg1=cvCreateImage(cvSize(pImg->width,pImg->height),IPL_DEPTH_8U,3);
	IplImage* pGray=cvCreateImage(cvSize(pImg->width,pImg->height),IPL_DEPTH_8U,1);
	int rr,gg,bb;
	int i,j;
	for (i=0;i<pImg1->height;i++)
		for (j=0;j<pImg1->width;j++)
		{
			bb=(BYTE)(pImg->imageData[i*pImg->widthStep+j*pImg->nChannels]);   //��pImg����ɫͨ��ֵ�ֱ𸳸�bb��gg��rr
			gg=(BYTE)(pImg->imageData[i*pImg->widthStep+j*pImg->nChannels+1]);
			rr=(BYTE)(pImg->imageData[i*pImg->widthStep+j*pImg->nChannels+2]);  
		    if (rr==r&&gg==g&&bb==b)
			{
				pImg1->imageData[i*pImg1->widthStep+j*pImg1->nChannels]=0;   //���bb��gg��rr������b��g��r��ֵ��ȣ���Ϊָ����ɫ����ʹ����
			    pImg1->imageData[i*pImg1->widthStep+j*pImg1->nChannels+1]=0;
		     	pImg1->imageData[i*pImg1->widthStep+j*pImg1->nChannels+2]=0;
		    }
		    else 
		    {
			    pImg1->imageData[i*pImg1->widthStep+j*pImg1->nChannels]=-1;   //���򣨼�ͼ����ɫ��������ɫ��ͬ������������
			    pImg1->imageData[i*pImg1->widthStep+j*pImg1->nChannels+1]=-1;
			    pImg1->imageData[i*pImg1->widthStep+j*pImg1->nChannels+2]=-1;
		    }
        }
	cvCvtColor(pImg1,pGray,CV_BGR2GRAY);
	//cvShowImage("gray",pGray);   //��ʾ��ɫ���Ľ��

	//�������
	CvMemStorage* storage=cvCreateMemStorage(0);   	//����������
	CvSeq* contour=NULL;
	int number_of_c=cvFindContours(pGray,storage,&contour);

	//������������
	for (int i=0;i<number_of_c;i++,contour=contour->h_next)
	{
		if (contour==NULL)
			break;
		//����CV_SEQ_KIND_CURVE�͵�
		if (!CV_IS_SEQ_CURVE(contour))
			continue;
		//����αƽ�
/*
CVAPI(CvSeq*)  cvApproxPoly( const void* src_seq,
                             int header_size, CvMemStorage* storage,
                             int method, double parameter,
                             int parameter2 CV_DEFAULT(0));
ʹ�ö����ȥ�ƽ�������ʹ������Ŀ���١�
CvSeq*             ����ֵ��Ӧ��һ�� ����������h_next��v_next��������������
src_seq            Ҫ�����Ŀ������
header_size    Ϊ���ؽ��ָ��ͷ�ṹ��С
storage             Ϊ���ؽ��ָ���µ��ڴ�洢��
method             �㷨��Ŀǰֻ��CV_POLY_APPOX_DP
parameter        �ƽ��㷨������ָ���ƽ����ȣ�����1��n
parameter2      �ƽ��㷨��������Ϊ0��ֻ����src_seqָ���������1��������˫�������е���������
*/
		CvSeq* pResult=cvApproxPoly(contour,sizeof(CvContour),storage,CV_POLY_APPROX_DP,cvContourPerimeter(contour)*0.03,0);

		int nPtNumPerPolygon=n;//�ߵ���Ŀ�Ƕ���������ж�֮һ   �ѱߵ���Ŀn����������ں������ã���߳��������Ժ�³����
		if (pResult->total!=nPtNumPerPolygon)
			continue;
		//���ƶ���� 
		for (int j=0;j<nPtNumPerPolygon;j++)
		{
			CvPoint ptS,ptE;
			ptS=*(CvPoint*)cvGetSeqElem(pResult,j);//ֱ�����
			ptE=*(CvPoint*)cvGetSeqElem(pResult,(j+1)%nPtNumPerPolygon);//ֱ���յ�
			cvLine(pImg,ptS,ptE,CV_RGB(0,255,255),3);   //��ԭͼ���ϻ�����߽�
		}
	}
	//�ͷ��ڴ棬ע���Ⱥ�˳��
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
	//ͼ����
    IplImage *pImg0=cvLoadImage("C:\\Users\\shuan\\Desktop\\�μ�\\������Ӿ�\\04_���ӳ���\\����ͼ��\\32.jpg",1);  //0Ϊ�Ҷȣ�1Ϊ��ɫ
	if(pImg0==NULL)
		return 1;
	EqualizeGray(pImg0);
	cvShowImage("lena ԭʼ",pImg0);
	IplImage *pImgDes0 = cvCreateImage(cvSize(pImg0->width,pImg0->height),IPL_DEPTH_16S,pImg0->nChannels);
	cvSobel(pImg0,pImgDes0,0,1,3);
	cvConvertScaleAbs(pImgDes0,pImg0);
	cvShowImage("lena Sobel��0,1,3",pImg0);

	IplImage *pImg=cvLoadImage("C:\\Users\\shuan\\Desktop\\�μ�\\������Ӿ�\\04_���ӳ���\\����ͼ��\\shan.bmp",1);  //0Ϊ�Ҷȣ�1Ϊ��ɫ
	if(pImg==NULL)
		return 1;
	cvShowImage("shan ԭʼ",pImg);
	IplImage *pImgDes = cvCreateImage(cvSize(pImg->width,pImg->height),IPL_DEPTH_16S,pImg->nChannels);
	cvSobel(pImg,pImgDes,0,1,3);
	cvConvertScaleAbs(pImgDes,pImg);
	cvShowImage("shan Sobel��0,1,3",pImg);

	IplImage *pImg3=cvLoadImage("C:\\Users\\shuan\\Desktop\\�μ�\\������Ӿ�\\04_���ӳ���\\����ͼ��\\shan.bmp",0);  //0Ϊ�Ҷȣ�1Ϊ��ɫ
	if(pImg3==NULL)
		return 1;
	cvCanny(pImg3,pImg3,10,150,3);   //canny����ֻ�ܴ���Ҷ�ͼ��3�����ֱ�Ϊsigma��dRatHigh��dRatLow
	cvShowImage("shan Canny��10,150,3",pImg3);

	IplImage *pImg4=cvLoadImage("C:\\Users\\shuan\\Desktop\\�μ�\\������Ӿ�\\04_���ӳ���\\����ͼ��\\lena.jpg",0);  //0Ϊ�Ҷȣ�1Ϊ��ɫ
	if(pImg4==NULL)
		return 1;
	cvCanny(pImg4,pImg4,10,150,3);
	cvShowImage("lena Canny��10,150,3",pImg4);

	IplImage *pImg5=cvLoadImage("C:\\Users\\shuan\\Desktop\\�μ�\\������Ӿ�\\04_���ӳ���\\����ͼ��\\lena.jpg",1);  //0Ϊ�Ҷȣ�1Ϊ��ɫ
	if(pImg5==NULL)
		return 1;
	cvSmooth(pImg5,pImg5,CV_GAUSSIAN,13);//��˹�˲���
	cvShowImage("lena GAUSSIAN��13",pImg5);

	IplImage *pImg6=cvLoadImage("C:\\Users\\shuan\\Desktop\\�μ�\\������Ӿ�\\04_���ӳ���\\����ͼ��\\lena.jpg",1);  //0Ϊ�Ҷȣ�1Ϊ��ɫ
	if(pImg6==NULL)
		return 1;
	cvSmooth(pImg6,pImg6,CV_BLUR,5);//��ֵ�˲���
	cvShowImage("lena BLUR��5",pImg6);

	IplImage *pImg7=cvLoadImage("C:\\Users\\shuan\\Desktop\\�μ�\\������Ӿ�\\04_���ӳ���\\����ͼ��\\lena.jpg",1);  //0Ϊ�Ҷȣ�1Ϊ��ɫ
	if(pImg7==NULL)
		return 1;
	cvSmooth(pImg7,pImg7,CV_MEDIAN,9);//��ֵ�˲���
	cvShowImage("lena MEDIAN��9",pImg7);

	cvReleaseImage(&pImg0);   //�ͷ�ͼ����Դ
	cvReleaseImage(&pImgDes0);//�ͷ�ͼ����Դ
	cvReleaseImage(&pImg);    //�ͷ�ͼ����Դ
	cvReleaseImage(&pImgDes); //�ͷ�ͼ����Դ
	cvReleaseImage(&pImg3);   //�ͷ�ͼ����Դ
	cvReleaseImage(&pImg4);   //�ͷ�ͼ����Դ
	cvReleaseImage(&pImg5);   //�ͷ�ͼ����Դ
	cvReleaseImage(&pImg6);   //�ͷ�ͼ����Դ
	cvReleaseImage(&pImg7);   //�ͷ�ͼ����Դ
*/



//Hough�任���ֱ����Բ
	 CvCapture* capture=cvCaptureFromCAM(0);   //��Ƶ��ȡ
	  if(capture==NULL)
	{ 
	   printf("û�м�⵽����ͷ");
	   cvWaitKey(0);
	   return 0;
	}
	cvNamedWindow("video",1);

	for(;;)
	{
	 IplImage* frame=cvQueryFrame(capture);
		if (!frame)
			break;
//		cvSmooth(frame,frame,CV_BLUR,5);   //ʹ�ø�˹�˲�������ӦΪ���������򱨴�
		if(frame->origin!=IPL_ORIGIN_TL)
			cvFlip(frame,frame,0);
// 		    HoughLine(frame);   //���ֱ��
// 			HoughCircle(frame);   //���Բ
// 		    EqualizeGray(frame);   //ֱ��ͼ���⻯
		    //runLKOpenCV(frame);   //�˶�������
		cvShowImage("video",frame);
		if(cvWaitKey(100)>=0)   //If you hit the escape key you will exit
			break;
	}
    cvReleaseCapture(&capture);
    cvDestroyWindow("video");
    cvWaitKey(0);
    return 0;
	

/*
     //ͼ����Ƕ
 	IplImage *pImg1=cvLoadImage("C:\\Users\\shuan\\Desktop\\�μ�\\������Ӿ�\\��������\\lena.bmp",1);  //����ͼ�� 0Ϊ�Ҷȣ�1Ϊ��ɫ
 	if(pImg1==NULL)
 		return 1;   
	IplImage *pImg2=cvLoadImage("C:\\Users\\shuan\\Desktop\\�μ�\\������Ӿ�\\��������\\Monkey.JPG",1);  
	if(pImg2==NULL)
		return 1;   

	IplImage* pImg3=cvCreateImage(cvSize(pImg1->width/2,pImg1->height/2),IPL_DEPTH_8U,3);  //�������ڴ�   �������ǲ��ı�Դͼ��ָ�룬���ں���������
	IplImage* pImg4=cvCreateImage(cvSize(pImg2->width,pImg2->height),IPL_DEPTH_8U,3);   

	cvPyrDown(pImg1,pImg3,CV_GAUSSIAN_5x5);  //ͼ��ֵ
	pImg4=pImg2;  
	int a,b;   //����ͼ��ƫ����
	a=100,b=100;
	for (int i=0;i<pImg3->height;i++)
	{
		for (int j=0;j<pImg3->width*pImg3->nChannels;j++)
		{
			pImg4->imageData[(a+i)*pImg4->widthStep+j+b*pImg4->nChannels]=pImg3->imageData[i*pImg3->widthStep+j];   //���Ͻ�Ϊԭ��ƽ�� 
		    pImg4->imageData[(a+i)*pImg4->widthStep+pImg4->widthStep-b*pImg4->nChannels-j]=pImg3->imageData[i*pImg3->widthStep+pImg3->widthStep-j];   //���Ͻ�Ϊԭ��ƽ�� 
 		    pImg4->imageData[(pImg4->height-a-i)*pImg4->widthStep+j+b*pImg4->nChannels]=pImg3->imageData[(pImg3->height-i)*pImg3->widthStep+j];   //���½�Ϊԭ��ƽ��
 			pImg4->imageData[(pImg4->height-a-i)*pImg4->widthStep+pImg4->widthStep-b*pImg4->nChannels-j]=pImg3->imageData[(pImg3->height-i)*pImg3->widthStep+pImg3->widthStep-j];   //���½�Ϊԭ��ƽ��
		}
	}
	cvShowImage("monkey ���� lena",pImg4);  //��ʾͼ��
*/


    //��ɫ���
    IplImage* pImg=cvLoadImage("F:\\������Ӿ�\\������Ӿ�\\��������\\��ɫͼ��.bmp",1);
	if (pImg==NULL)
		return 1;
//	cvFindContours2(pImg,237,28,36,10);   //��ȡ��ɫ�����
//	cvFindContours2(pImg,255,242,0,4);    //��ȡ��ɫ����
	cvFindContours2(pImg,0,0,255,1000);   //��ȡ��ɫԲȦ
	cvShowImage("��ɫʶ�� �������",pImg);
	cvWaitKey(0);
	cvReleaseImage(&pImg);
	return 0;

}

int type_num=0;

JNIEXPORT jlong JNICALL Java_com_example_carplate_CarPlateDetection_doImageShow
(JNIEnv *env, jclass clz, jlong imageGray)
{
	LOGD("          doImageShow         !");
	//cvSmooth(frame,frame,CV_BLUR,5);   //ʹ�ø�˹�˲�������ӦΪ���������򱨴�
	//if(frame->origin!=IPL_ORIGIN_TL)
	//cvFlip(frame,frame,0);

	Mat imageMat = Mat(*(Mat*)imageGray);
	IplImage  temp_src = imageMat;
	IplImage* imageg = &temp_src;

	if(type_num==0){
	    //cvFindContours2(imageg,0,0,255,1000);   //��ȡ��ɫԲȦ
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

