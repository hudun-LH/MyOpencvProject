#include<com_example_carplate_CarPlateDetection.h>
#include "Plate.h"
#include "Plate_Segment.h"
#include "Plate_Recognition.h"
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

#define LOG_TAG "System.out"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

/*char* jstring2str(JNIEnv* env, jstring jstr)
{
    char*   rtn   =   NULL;
    jclass   clsstring   =   env->FindClass("java/lang/String");
    jstring   strencode   =   env->NewStringUTF("GB2312");
    jmethodID   mid   =   env->GetMethodID(clsstring,   "getBytes",   "(Ljava/lang/String;)[B");
    jbyteArray   barr=   (jbyteArray)env->CallObjectMethod(jstr,mid,strencode);
    jsize   alen   =   env->GetArrayLength(barr);
    jbyte*   ba   =   env->GetByteArrayElements(barr,JNI_FALSE);
    if(alen   >   0)
    {
        rtn   =   (char*)malloc(alen+1);
        memcpy(rtn,ba,alen);
        rtn[alen]=0;
    }
    env->ReleaseByteArrayElements(barr,ba,0);
    return  rtn;
}*/
#ifdef __cplusplus
extern "C" {
#endif

int result_cols;
int result_rows;
double minVal;
double maxVal;
Point minLoc;
Point maxLoc;
Point matchLoc;
int match_method = CV_TM_SQDIFF;

JNIEXPORT jstring JNICALL Java_com_example_carplate_CarPlateDetection_ImageProc
  (JNIEnv *env, jclass obj, jintArray buf, jint w, jint h,jstring dir){
	jint *cbuf;
    cbuf = env->GetIntArrayElements(buf, false);
    //char* path = jstring2str(env,dir);

    Size size;
    size.width = w;
    size.height = h;

    Mat imageData,input;
    imageData = Mat(size, CV_8UC4, (unsigned char*)cbuf);
    input = Mat(size, CV_8UC3);
    cvtColor(imageData,input,CV_BGRA2BGR);

	vector<Plate> posible_regions = segment(input);

	const char strCharacters[] = {'0','1','2','3','4','5','6','7','8','9','B',
			'C', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'M', 'N', 'P', 'R', 'S', 'T', 'V', 'W', 'X', 'Y', 'Z'};
	CvANN_MLP ann;
	//SVM for each plate region to get valid car plates,Read file storage.
	FileStorage fs;
	//strcat(path,"/SVM.xml");
	fs.open("/storage/emulated/0/SVM.xml", FileStorage::READ); //"/storage/sdcard/SVM.xml"
	Mat SVM_TrainingData;
	Mat SVM_Classes;
	fs["TrainingData"] >> SVM_TrainingData;
	fs["classes"] >> SVM_Classes;
	if(fs.isOpened())
		LOGD("read success!");

	//Set SVM params
	LOGD("size:%d",SVM_TrainingData.rows);
	SVM_TrainingData.convertTo(SVM_TrainingData, CV_32FC1);
	SVM_Classes.convertTo(SVM_Classes, CV_32FC1);

	CvSVMParams SVM_params;
	SVM_params.svm_type = CvSVM::C_SVC;
	SVM_params.kernel_type = CvSVM::LINEAR; //CvSVM::LINEAR;
	SVM_params.degree = 0;
	SVM_params.gamma = 1;
	SVM_params.coef0 = 0;
	SVM_params.C = 1;
	SVM_params.nu = 0;
	SVM_params.p = 0;
	SVM_params.term_crit = cvTermCriteria(CV_TERMCRIT_ITER, 1000, 0.01);
	LOGD("Everything is ready");
	//Train SVM
	LOGD("START TO ENTER SVM PREDICT");
	CvSVM svmClassifier(SVM_TrainingData, SVM_Classes, Mat(), Mat(), SVM_params);
	//For each possible plate, classify with svm if it's a plate or no
	vector<Plate> plates;
	for(int i=0; i< posible_regions.size(); i++)
	{
		Mat img=posible_regions[i].plateImg;
		Mat p= img.reshape(1, 1);
		p.convertTo(p, CV_32FC1);

		int response = (int)svmClassifier.predict( p );
		if(response==1)
			plates.push_back(posible_regions[i]);
	}
	LOGD("SVM PREDICT FINISH");
	//图像分割：采用一系列不同的滤波器、形态学操作、轮廓算法和验证算法，提取图像中可能包含车牌的区域
	//图像分类：对每个图像块使用支持向量机SVM分类，并由代码自动创建正负样本
	fs.release();
	//Read file storage.
	FileStorage fs2;
	fs2.open("/storage/emulated/0/OCR.xml", FileStorage::READ); ///storage/sdcard/OCR.xml"
	Mat TrainingData;
	Mat Classes;
	fs2["TrainingDataF15"] >> TrainingData;
	fs2["classes"] >> Classes;
	LOGD("size:%d",TrainingData.rows);
	LOGD("START TO TRAIN MLP");
	//训练神经网络
	train(TrainingData, Classes,&ann,10);
	LOGD("FINISH TRAIN MLP");
	Mat inputs=plates[0].plateImg;
	Plate mplate;
	//dealing image and save each character image into vector<CharSegment>
	//Threshold input image
	Mat img_threshold;
	threshold(inputs, img_threshold, 60, 255, CV_THRESH_BINARY_INV);

	Mat img_contours;
	img_threshold.copyTo(img_contours);
	//Find contours of possibles characters
	vector< vector< Point> > contours;
	findContours(img_contours,
		contours, // a vector of contours
		CV_RETR_EXTERNAL, // retrieve the external contours
		CV_CHAIN_APPROX_NONE); // all pixels of each contours
	//Start to iterate to each contour founded
	vector<vector<Point> >::iterator itc= contours.begin();
	LOGD("Before extracting hist and low-resolution image");
	//Remove patch that are no inside limits of aspect ratio and area.
	while (itc!=contours.end()) {

		//Create bounding rect of object
		Rect mr= boundingRect(Mat(*itc)); //opencv中利用函数boundingRect来对指定的点集进行包含，使得形成一个最合适的正向矩形框把当前指定的点集都框住
		//Crop image
		Mat auxRoi(img_threshold, mr); // ROI Mat
		if(verifySizes(auxRoi)){
			auxRoi=preprocessChar(auxRoi);
			LOGD("FINISH extracting features");
			//对每一个小方块，提取直方图特征
			Mat f=features(auxRoi,15);
			//For each segment feature Classify
			LOGD("START TO CLASSIFY IN MLP");
			int character=classify(f,&ann);
			mplate.chars.push_back(strCharacters[character]);
			LOGD("FINISH CLASSIFY");
			mplate.charsPos.push_back(mr);
			//printf("%c ",strCharacters[character]);
		}
		++itc;
	}
	fs2.release();
	string licensePlate=mplate.str();
	//const char *result;
	//result=licensePlate.c_str();
	env->ReleaseIntArrayElements(buf, cbuf, 0);

	return env->NewStringUTF(licensePlate.c_str());
}

void drawDetectLines(Mat& image,const vector<Vec4i>& lines,Scalar & color)
{
    // 将检测到的直线在图上画出来
    vector<Vec4i>::const_iterator it=lines.begin();
    while(it!=lines.end())
    {
        Point pt1((*it)[0],(*it)[1]);
        Point pt2((*it)[2],(*it)[3]);
        line(image,pt1,pt2,color,2); //  线条宽度设置为2
        ++it;
    }
}

#define UNKNOWN_FLOW_THRESH 1e9

void makecolorwheel(vector<Scalar> &colorwheel)
{
    int RY = 15;
    int YG = 6;
    int GC = 4;
    int CB = 11;
    int BM = 13;
    int MR = 6;

    int i;

    for (i = 0; i < RY; i++) colorwheel.push_back(Scalar(255,       255*i/RY,     0));
    for (i = 0; i < YG; i++) colorwheel.push_back(Scalar(255-255*i/YG, 255,       0));
    for (i = 0; i < GC; i++) colorwheel.push_back(Scalar(0,         255,      255*i/GC));
    for (i = 0; i < CB; i++) colorwheel.push_back(Scalar(0,         255-255*i/CB, 255));
    for (i = 0; i < BM; i++) colorwheel.push_back(Scalar(255*i/BM,      0,        255));
    for (i = 0; i < MR; i++) colorwheel.push_back(Scalar(255,       0,        255-255*i/MR));
}

void motionToColor(Mat flow, Mat &color)
{
    if (color.empty())
        color.create(flow.rows, flow.cols, CV_8UC3);

    static vector<Scalar> colorwheel; //Scalar r,g,b
    if (colorwheel.empty())
        makecolorwheel(colorwheel);

    // determine motion range:
    float maxrad = -1;

    // Find max flow to normalize fx and fy
    for (int i= 0; i < flow.rows; ++i)
    {
        for (int j = 0; j < flow.cols; ++j)
        {
            Vec2f flow_at_point = flow.at<Vec2f>(i, j);
            float fx = flow_at_point[0];
            float fy = flow_at_point[1];
            if ((fabs(fx) >  UNKNOWN_FLOW_THRESH) || (fabs(fy) >  UNKNOWN_FLOW_THRESH))
                continue;
            float rad = sqrt(fx * fx + fy * fy);
            maxrad = maxrad > rad ? maxrad : rad;
        }
    }

    for (int i= 0; i < flow.rows; ++i)
    {
        for (int j = 0; j < flow.cols; ++j)
        {
            uchar *data = color.data + color.step[0] * i + color.step[1] * j;
            Vec2f flow_at_point = flow.at<Vec2f>(i, j);

            float fx = flow_at_point[0] / maxrad;
            float fy = flow_at_point[1] / maxrad;
            if ((fabs(fx) >  UNKNOWN_FLOW_THRESH) || (fabs(fy) >  UNKNOWN_FLOW_THRESH))
            {
                data[0] = data[1] = data[2] = 0;
                continue;
            }
            float rad = sqrt(fx * fx + fy * fy);

            float angle = atan2(-fy, -fx) / CV_PI;
            float fk = (angle + 1.0) / 2.0 * (colorwheel.size()-1);
            int k0 = (int)fk;
            int k1 = (k0 + 1) % colorwheel.size();
            float f = fk - k0;
            //f = 0; // uncomment to see original color wheel

            for (int b = 0; b < 3; b++)
            {
                float col0 = colorwheel[k0][b] / 255.0;
                float col1 = colorwheel[k1][b] / 255.0;
                float col = (1 - f) * col0 + f * col1;
                if (rad <= 1)
                    col = 1 - rad * (1 - col); // increase saturation with radius
                else
                    col *= .75; // out of range
                data[2 - b] = (int)(255.0 * col);
            }
        }
    }
}

JNIEXPORT jlong JNICALL Java_com_example_carplate_CarPlateDetection_OpticalFlow
  (JNIEnv *env, jclass obj, jlong imageGray, jlong tempGray)
{
	LOGD("            OpticalFlow        !");

	//Mat& img_display_0  = *(Mat*)imageGray;
	Mat I1;
	Mat I2;
	Mat input1;
	Mat input2;
	Mat flow;
	I1 = Mat(*(Mat*)imageGray);
	I2 = Mat(*(Mat*)imageGray);

	input1.create(I1.size(),CV_8UC1);//CV_32FC1
	cvtColor(I1,input1,CV_BGR2GRAY);

	input2.create(I2.size(),CV_8UC1);//CV_32FC1
	cvtColor(I2,input2,CV_BGR2GRAY);

	calcOpticalFlowFarneback(input1,input1,flow,0.5,3,20,3,5,1.2,0);
	LOGD("      calcOpticalFlowFarneback      !");

	 //flow = abs(flow);
	Mat motion2color;
    motionToColor(flow, motion2color);

	LOGD("      motionToColor      !");

	Mat *hist = new Mat(motion2color);
	return (jlong) hist;
	//LOGD(" imageData:  %d  %d , input: %d  %d !\n",contours.);
}

//Mat& img_display_0 = *(Mat*)imageGray;
//IplImage* frame = &IplImage(*((Mat*)imageGray));

//Mat类型侧重于计算，数学性较高，openCV对Mat类型的计算也进行了优化。而CvMat和IplImage类型更侧重于“图像”
//CvMat之上还有一个更抽象的基类----CvArr
//1\ 直接调用相应的构造函数就行
//Mat m_mat转IplImage
//IplImage iplimage = m_mat;
//IplImage *src=&IplImage(m_mat);
//IplImage *m_iplImage转Mat
//Mat src(m_iplImage);

//2\
//Mat转IplImage(直接转就行，比如)：
//IplImage a;
//Mat b;
//a = b;
//Mat -> IplImage
//Mat M
//IplImage iplimage = M;

//3\
//Mat frame;
//VideoCapture capture(0);
//bool bsucess=capture.read(frame);
//IplImage* pImage=cvCreateImage(cvSize(frame.cols,frame.rows),8,3);
//pImage->imageData=(char*)frame.data;
//Mat frame(pImage,0);
//IplImage* img = cvLoadImage("greatwave.jpg", 1);
//Mat mtx(img,0); // convert IplImage* -> Mat;
//IplImage* iplImg = cvLoadImage("greatwave.jpg", 1);
//Mat mtx(iplImg);

//4\
//CvMat -> Mat
//Mat::Mat(const CvMat* m, bool copyData=false);
//Mat -> CvMat
//例子(假设Mat类型的imgMat图像数据存在)：
//CvMat cvMat = imgMat;/*Mat -> CvMat, 类似转换到IplImage，不复制数据只创建矩阵头

void DrawBox(CvBox2D box,IplImage* img)
{
    CvPoint2D32f points[4];
    cvBoxPoints(box,points);

    CvPoint pt[4];
    for (int i=0; i<4; i++)
    {
        pt[i].x = (int)points[i].x;
        pt[i].y = (int)points[i].y;
    }
    cvLine( img, pt[0], pt[1],CV_RGB(255,0,0), 2, 8, 0 );
    cvLine( img, pt[1], pt[2],CV_RGB(255,0,0), 2, 8, 0 );
    cvLine( img, pt[2], pt[3],CV_RGB(255,0,0), 2, 8, 0 );
    cvLine( img, pt[3], pt[0],CV_RGB(255,0,0), 2, 8, 0 );
}

JNIEXPORT jlong JNICALL Java_com_example_carplate_CarPlateDetection_contourProc
  (JNIEnv *env, jclass obj, jlong imageGray, jlong tempGray)
{
	LOGD("            contourProc        !");
	Mat image;
	image = Mat(*(Mat*)imageGray);//Mat(size, CV_8UC4, (unsigned char*)cbuf);
	//Mat &image = *(Mat*)imageGray;
	//LOGD("      contourProc  %d, %d       !", image.cols,image.rows);

	CvSeq* contour = NULL;
	double minarea = 60.0;//100.0;
	double tmparea = 0.0;

	CvMemStorage* storage = cvCreateMemStorage(0);

    //IplImage *src=&IplImage(m_mat);
	IplImage  temp_src = image;
	IplImage* img_src = &temp_src;
    //IplImage* img_src= cvLoadImage(dlg.GetPathName(),CV_LOAD_IMAGE_ANYCOLOR);

	//IplImage* img_Clone=cvCloneImage(img_src);
	//访问二值图像每个点的值
	uchar *pp;

    IplImage* img_dst = cvCreateImage(cvGetSize(img_src),IPL_DEPTH_8U,1);
    //IplImage* img_dst =cvCreateImage(cvGetSize(img_src),8,1);

    cvCvtColor(img_src,img_dst,CV_BGR2GRAY);
    cvThreshold(img_dst,img_dst,180,255,CV_THRESH_BINARY); //240

	//------------搜索二值图中的轮廓，并从轮廓树中删除面积小于某个阈值minarea的轮廓-------------//
	CvScalar color = cvScalar(255,0,0);//CV_RGB(128,0,0);
	CvContourScanner scanner = NULL;
	LOGD("            cvStartFindContours before        !");

	scanner = cvStartFindContours(img_dst,storage,sizeof(CvContour),CV_RETR_CCOMP,CV_CHAIN_APPROX_NONE,cvPoint(0,0));
	LOGD("            cvStartFindContours         !");

	IplImage* img_Clone=cvCloneImage(img_src);


	//开始遍历轮廓树
	CvRect rect;
	while (contour=cvFindNextContour(scanner))
	{
	     tmparea = fabs(cvContourArea(contour)); //轮廓面积
	     rect = cvBoundingRect(contour,0); //轮廓的边界框
	     if (tmparea < minarea) {// ||tmparea>4900
	        //当连通域的中心点为黑色时，而且面积较小则用白色进行填充
	        pp=(uchar*)(img_Clone->imageData + img_Clone->widthStep*(rect.y+rect.height/2)+rect.x+rect.width/2);
	        if (pp[0]==0) {
	            for(int y = rect.y;y<rect.y+rect.height;y++) {
	                for(int x =rect.x;x<rect.x+rect.width;x++) {
	                    pp=(uchar*)(img_Clone->imageData + img_Clone->widthStep*y+x);
	                    if (pp[0]==0) {
	                            pp[0]=255;
	                    }
	                }
	            }
	        }
	     }
	     //cvRectangle( CvArr* img, CvPoint pt1, CvPoint pt2,
         //CvScalar color, int thickness CV_DEFAULT(1),int line_type CV_DEFAULT(8), int shift CV_DEFAULT(0));
	     cvRectangleR(img_Clone, rect, color, 3); //Scalar::all(255)   Scalar(0,255,0), 3)

	     //CvBox2D box1 =cvMinAreaRect2(contour);//最小矩形
	     //DrawBox(box1,img_Clone);
	}

	Mat mtx(img_Clone,0);
	//Mat mtx(img_Clone);
	Mat *hist = new Mat(mtx);
	return (jlong) hist;
	//LOGD(" imageData:  %d  %d , input: %d  %d !\n",contours.);
}

//对minAreaRect获得的最小外接矩形，用纵横比进行判断
bool verifySizes_0(RotatedRect mr)
{
	float error=0.4;
	//Spain car plate size: 52x11 aspect 4,7272
	float aspect=2;//4.7272;
	//Set a min and max area. All other patchs are discarded
	int min= 15*aspect*15; // minimum area
	int max= 125*aspect*125; // maximum area
	//Get only patchs that match to a respect ratio.
	float rmin= aspect-aspect*error;
	float rmax= aspect+aspect*error;

	int area= mr.size.height * mr.size.width;
	float r= (float)mr.size.width / (float)mr.size.height;
	if(r<1)
		r= (float)mr.size.height / (float)mr.size.width;

	if(( area < min || area > max )){//|| ( r < rmin || r > rmax )){
		return false;
	}else{
		return true;
	}
}

JNIEXPORT jlong JNICALL Java_com_example_carplate_CarPlateDetection_contourProc1
  (JNIEnv *env, jclass obj, jlong imageGray, jlong tempGray)
{
	LOGD("            contourProc1        !");
	LOGD("            contourProc1        !");

	//Mat& img_display_0  = *(Mat*)imageGray;
    int width;
    int height;
	Mat image;
	Mat input;
	image = Mat(*(Mat*)imageGray);//Mat(size, CV_8UC4, (unsigned char*)cbuf);
	width  = image.cols;
	height = image.rows;
	LOGD("      contourProc  %d, %d       !", width,height);
	input.create(width, height,CV_8UC1);//CV_32FC1

	cvtColor(image,input,CV_BGR2GRAY);
	threshold(input,input,180,255,THRESH_BINARY);

    vector< vector<Point> > contours;
	// find
	findContours(input,contours,CV_RETR_LIST,CV_CHAIN_APPROX_NONE); //CV_RETR_EXTERNAL

	vector< vector<Point> >::iterator itc= contours.begin();

	while (itc!=contours.end()) {
		//Create bounding rect of object
		RotatedRect mr= minAreaRect(Mat(*itc));
		if( !verifySizes_0(mr)){
			itc= contours.erase(itc);
		}else{
		    ++itc;
		}
	}

    // draw
	Mat result(image.size(),CV_8UC1, Scalar(0));//CV_8U
	//drawContours(result,contours,-1,Scalar(255,255,255),2);
	drawContours(image,contours,-1,Scalar(0,255,0),2);

	Mat *hist = new Mat(image);
	return (jlong) hist;
	//LOGD(" imageData:  %d  %d , input: %d  %d !\n",contours.);
}

JNIEXPORT jlong JNICALL Java_com_example_carplate_CarPlateDetection_contourProc0
  (JNIEnv *env, jclass obj, jlong imageGray, jlong tempGray)
{
	LOGD("            contourProc0        !");
	Mat image;
    image = Mat(*(Mat*)imageGray);//Mat(size, CV_8UC4, (unsigned char*)cbuf);
    CvMemStorage* storage = cvCreateMemStorage(0);

	//IplImage *src=&IplImage(m_mat);
    IplImage  temp_src = image;
    IplImage* img_src = &temp_src;
	//IplImage* img_src= cvLoadImage(dlg.GetPathName(),CV_LOAD_IMAGE_ANYCOLOR);

	//IplImage* img_Clone=cvCloneImage(img_src);
	IplImage* img_dst = cvCreateImage(cvGetSize(img_src),IPL_DEPTH_8U,1);
	//IplImage* img_dst =cvCreateImage(cvGetSize(img_src),8,1);
	IplImage *contoursImage = cvCreateImage(cvGetSize(img_src), 8, 1);

	cvCvtColor(img_src,img_dst,CV_BGR2GRAY);
	IplImage* imgColor=cvCloneImage(img_dst);
	cvThreshold(img_dst,img_dst,180,255,CV_THRESH_BINARY); //240


	CvSeq *contours = 0, *contoursTemp = 0;
	cvZero(contoursImage);

	//cvCvtColor(img, img_dst, CV_GRAY2BGR);
	//cvThreshold(img_dst, img_dst, 100, 255, CV_THRESH_BINARY);  // 二值化操作
	//cvCvtColor(img, imgColor, CV_GRAY2BGR);

	int totals = cvFindContours(img_dst, storage,&contours, sizeof(CvContour),    //img必须是一个二值图像 storage 用来存储的contours指向存储的第一个轮廓
	        CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE, cvPoint(0,0));

	contoursTemp = contours;
	int count = 0;
	int i;
	for(;contoursTemp != 0; contoursTemp = contoursTemp -> h_next)  /// 这样可以访问每一个轮廓  ====横向轮廓
	{
	        for(i = 0; i < contoursTemp -> total; i++)    // 提取一个轮廓的所有坐标点
	        {
	            CvPoint *pt = (CvPoint*) cvGetSeqElem(contoursTemp, i);   // 得到一个轮廓中一个点的函数cvGetSeqElem
	            cvSetReal2D(contoursImage, pt->y, pt->x, 255.0);
	            cvSet2D(imgColor, pt->y, pt->x, cvScalar(0,0,255,0));
	        }
	        count ++;
	        CvSeq *InterCon = contoursTemp->v_next;     // 访问每个轮廓的纵向轮廓
	        for(; InterCon != 0; InterCon = InterCon ->h_next)
	        {
	            for(i = 0; i < InterCon->total; i++ )
	            {
	                CvPoint *pt = (CvPoint*)cvGetSeqElem(InterCon, i);
	                cvSetReal2D(contoursImage, pt->y, pt->x, 255.0);
	                cvSet2D(imgColor, pt->y, pt->x, cvScalar(0, 255, 0, 0));
	            }
	        }
	}

	Mat mtx(imgColor,0);
	Mat *hist = new Mat(mtx);
	return (jlong) hist;
	//LOGD(" imageData:  %d  %d , input: %d  %d !\n",contours.);
}

JNIEXPORT jlong JNICALL Java_com_example_carplate_CarPlateDetection_contourProc00
  (JNIEnv *env, jclass obj, jlong imageGray, jlong tempGray)
{
	LOGD("            contourProc0        !");

	//Mat& img_display_0  = *(Mat*)imageGray;
    int width;
    int height;
    int type;

	Mat imageData;
	Mat input;
	imageData = Mat(*(Mat*)imageGray);//Mat(size, CV_8UC4, (unsigned char*)cbuf);
	width  = imageData.cols;
	height = imageData.rows;
	LOGD("      contourProc  %d, %d       !", width,height);
	input.create(width, height,CV_8UC1); //CV_32FC1
	//input = new Mat(width, height, CV_8UC3);//Mat(size, CV_8UC3);
	cvtColor(imageData,input,CV_BGR2GRAY);  //CV_BGRA2BGR
	//threshold(input,input,128,255,THRESH_BINARY); // 2 method

	Mat contours;
	Canny(input,contours,125,350);
	threshold(contours,contours,128,255,THRESH_BINARY); // 1 method

	//vector<Vec4i> lines;
	// 检测直线，最小投票为90，线条不短于50，间隙不小于10
	//HoughLinesP(contours,lines,1,CV_PI/180,80,50,10);
	//drawDetectLines(contours, &lines,Scalar(0,255,0));

	Mat *hist = new Mat(contours);
	return (jlong) hist;
	//LOGD(" imageData:  %d  %d , input: %d  %d !\n",contours.);
}

JNIEXPORT jlong JNICALL Java_com_example_carplate_CarPlateDetection_matchTemplate
  (JNIEnv *env, jclass obj, jlong imageGray, jlong tempGray)
{
	LOGD("            matchTemplate ss       !");
	//LOGD("            matchTemplate        !");
	//Mat& img_display_0  = *(Mat*)imageGray;
	double minValue, maxValue;
	CvPoint minLoc, maxLoc;
	int srcW, srcH, templatW, templatH, resultH, resultW;
	IplImage *result;

    Mat img_display_o =  Mat(*(Mat*)imageGray);
    Mat img_o =  Mat(*(Mat*)tempGray);

    IplImage img_display_i = img_display_o;
    IplImage img_i = img_o;
    IplImage *img_display = &img_display_i;
    IplImage *img = &img_i;

    IplImage* imgColor=cvCloneImage(img_display);

    srcW = img_display->width;
    srcH = img_display->height;
    templatW = img->width;
    templatH = img->height;
    if(srcW < templatW || srcH < templatH){
    	return 0;
    }
    resultW = srcW - templatW + 1;
    resultH = srcH - templatH + 1;
    result = cvCreateImage(cvSize(resultW, resultH), 32, 1);

    cvMatchTemplate(img_display, img, result, CV_TM_SQDIFF);
    //cvNormalize(img_display, img_display);
    cvMinMaxLoc(result, &minValue, &maxValue, &minLoc, &maxLoc);
    cvRectangle(imgColor, minLoc, cvPoint(minLoc.x + img->width, minLoc.y+ img->height), cvScalar(0,0,255));

    LOGD("            matchTemplate ee        !");
    Mat mtx(imgColor,0);
    Mat *hist = new Mat(mtx);
    return (jlong) hist;
}

JNIEXPORT jlong JNICALL Java_com_example_carplate_CarPlateDetection_matchTemplate11
  (JNIEnv *env, jclass obj, jlong imageGray, jlong tempGray)
{
	LOGD("            matchTemplate        !");
	//LOGD("            matchTemplate        !");
	Mat& img_display_0  = *(Mat*)imageGray;
	//Mat& img          = *(Mat*)tempGray;

    //Mat *img_display  =  (Mat*)imageGray
	//Mat *img          =  (Mat*)tempGray;

	//Mat& img_display =  *(Mat*)imageGray;
	//Mat& img =  *(Mat*)tempGray;

	Mat img_display =  Mat(*(Mat*)imageGray);
	Mat img  =  Mat(*(Mat*)tempGray);

	//LOGD("00 img_display:  %d  %d ,  %d %d  !\n",img_display.cols, img_display.rows,img.cols, img.rows);

    /// Do the Matching and Normalize
	//1\ void matchTemplate( InputArray image, InputArray templ,
	//                       OutputArray result, int method );
    matchTemplate(img_display, img, img_display, match_method);
    //2\void normalize( InputArray src, OutputArray dst, double alpha=1, double beta=0,
    //                  int norm_type=NORM_L2, int dtype=-1, InputArray mask=noArray());

    //normalize(img_display, img_display, 0, 1, NORM_MINMAX, -1, new Mat());
    normalize(img_display, img_display);
    // Localizing the best match with minMaxLoc

    //3\ void minMaxLoc(InputArray src, CV_OUT double* minVal, CV_OUT double* maxVal=0, CV_OUT Point* minLoc=0,
    //   CV_OUT Point* maxLoc=0, InputArray mask=noArray());

    minMaxLoc(img_display, &minVal, &maxVal, &minLoc, &maxLoc);

    /// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
    if(match_method == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED ){
        matchLoc = minLoc;
        //LOGD("         11   %d, %d        !",matchLoc.x, matchLoc.y);
    }else{
        matchLoc = maxLoc;
    }

    //Mat M(100, 100, CV_8U);
    //Mat_<float>& M1 = (Mat_<float>&)M;

    //*(Mat*)imageGray = &img_display;//(Mat &)img_display;
    //rectangle(img_display_0, cvPoint(0,0),cvPoint(64,64), CV_RGB(255,0,0), 3);

    //Mat& src  = *(Mat*)addrGray;
    //4\ void rectangle(CV_IN_OUT Mat& img, Point pt1, Point pt2, const Scalar& color, int thickness=1, int lineType=8, int shift=0);

    rectangle(img_display_0, matchLoc, Point(matchLoc.x + img.cols , matchLoc.y + img.rows ), CV_RGB(0,255,0), 3); //Scalar::all(255)   Scalar(0,255,0), 3)

    /*
    Mat img_display_o =  Mat(*(Mat*)imageGray);
    Mat img_o =  Mat(*(Mat*)tempGray);
    IplImage *img_display = &img_display_o;
    IplImage *img = &img_o;

    cvMatchTemplate(img_display, img, img_display, CV_TM_SQDIFF);
    double minValue, maxValue;
    CvPoint minLoc, maxLoc;
    //cvNormalize(img_display, img_display);
    cvMinMaxLoc(img_display, &minValue, &maxValue, &minLoc, &maxLoc);
    cvRectangle(img_display, minLoc, cvPoint(minLoc.x + img.cols, minLoc.y+ img.rows), cvScalar(0,0,255));

    Mat *hist = new Mat(img_display);
    return (jlong) hist;
    */

    LOGD("            matchTemplate  ee      !");

    //Mat *hist = new Mat(img_display);
    //return (jlong) hist;
    return 0;
}

JNIEXPORT jlong JNICALL Java_com_example_carplate_CarPlateDetection_matchTemplate00
  (JNIEnv *env, jclass obj, jlong imageGray, jlong tempGray)
{
	LOGD("            matchTemplate        !");
	LOGD("            matchTemplate        !");
	Mat& img_display_0  = *(Mat*)imageGray;
	//Mat& img          = *(Mat*)tempGray;

    //Mat *img_display  =  (Mat*)imageGray
	//Mat *img          =  (Mat*)tempGray;

	//Mat& img_display =  *(Mat*)imageGray;
	//Mat& img =  *(Mat*)tempGray;

	Mat img_display =  Mat(*(Mat*)imageGray);
	Mat img  =  Mat(*(Mat*)tempGray);

	LOGD("00 img_display:  %d  %d ,  %d %d  !\n",img_display.cols, img_display.rows,img.cols, img.rows);

    /// Do the Matching and Normalize
	//1\ void matchTemplate( InputArray image, InputArray templ,
	//                       OutputArray result, int method );
    matchTemplate(img_display, img, img_display, match_method);
    //2\void normalize( InputArray src, OutputArray dst, double alpha=1, double beta=0,
    //                  int norm_type=NORM_L2, int dtype=-1, InputArray mask=noArray());

    //normalize(img_display, img_display, 0, 1, NORM_MINMAX, -1, new Mat());
    normalize(img_display, img_display);
    // Localizing the best match with minMaxLoc

    //3\ void minMaxLoc(InputArray src, CV_OUT double* minVal, CV_OUT double* maxVal=0, CV_OUT Point* minLoc=0,
    //   CV_OUT Point* maxLoc=0, InputArray mask=noArray());

    minMaxLoc(img_display, &minVal, &maxVal, &minLoc, &maxLoc);

    /// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
    if(match_method == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED ){
        matchLoc = minLoc;
        LOGD("         11   %d, %d        !",matchLoc.x, matchLoc.y);
    }else{
        matchLoc = maxLoc;
    }

    //Mat M(100, 100, CV_8U);
    //Mat_<float>& M1 = (Mat_<float>&)M;

    //*(Mat*)imageGray = &img_display;//(Mat &)img_display;
    //rectangle(img_display_0, cvPoint(0,0),cvPoint(64,64), CV_RGB(255,0,0), 3);

    //Mat& src  = *(Mat*)addrGray;
    //4\ void rectangle(CV_IN_OUT Mat& img, Point pt1, Point pt2, const Scalar& color, int thickness=1, int lineType=8, int shift=0);

    rectangle(img_display_0, matchLoc, Point(matchLoc.x + img.cols , matchLoc.y + img.rows ), CV_RGB(0,255,0), 3); //Scalar::all(255)   Scalar(0,255,0), 3)

    return 0;
}

JNIEXPORT jlong JNICALL Java_com_example_carplate_CarPlateDetection_grayProc01
  (JNIEnv *env, jclass obj, jlong imageGray, jlong tempGray)
{
	LOGD("            matchTemplate        !");
	LOGD("            matchTemplate        !");
	//Mat& img_display = *(Mat*)imageGray;
	//Mat& img  = *(Mat*)tempGray;
    //Mat *img_display = (Mat*) imageGray
	Mat& img_display_0 =  *(Mat*)imageGray;

	//Mat(const Mat& m)
	Mat img_display =  Mat(*(Mat*)imageGray);
	Mat img  =  Mat(*(Mat*)tempGray);

	LOGD("          00  %d, %d        !",img.cols, img.rows);

    /// Do the Matching and Normalize
	//1\ void matchTemplate( InputArray image, InputArray templ,
	//                       OutputArray result, int method );
    matchTemplate(img_display, img, img_display, match_method);
    //2\void normalize( InputArray src, OutputArray dst, double alpha=1, double beta=0,
    //                  int norm_type=NORM_L2, int dtype=-1, InputArray mask=noArray());

    //normalize(img_display, img_display, 0, 1, NORM_MINMAX, -1, new Mat());
    normalize(img_display, img_display);
    // Localizing the best match with minMaxLoc

    //3\ void minMaxLoc(InputArray src, CV_OUT double* minVal, CV_OUT double* maxVal=0, CV_OUT Point* minLoc=0,
    //   CV_OUT Point* maxLoc=0, InputArray mask=noArray());

    minMaxLoc(img_display, &minVal, &maxVal, &minLoc, &maxLoc);
    /// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
    if(match_method == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED ){
        matchLoc = minLoc;
        LOGD("         11   %d, %d        !",matchLoc.x, matchLoc.y);
    }else{
        matchLoc = maxLoc;
    }

    //Mat M(100, 100, CV_8U);
    //Mat_<float>& M1 = (Mat_<float>&)M;

    //*(Mat*)imageGray = &img_display;//(Mat &)img_display;
    rectangle(img_display_0, cvPoint(0,0),cvPoint(64,64), CV_RGB(255,0,0), 3);

    //Mat& src  = *(Mat*)addrGray;
    //4\ void rectangle(CV_IN_OUT Mat& img, Point pt1, Point pt2, const Scalar& color, int thickness=1, int lineType=8, int shift=0);

    //rectangle(img_display, matchLoc, Point(matchLoc.x + img.cols , matchLoc.y + img.rows ), CV_RGB(0,255,0), 3); //Scalar::all(255)   Scalar(0,255,0), 3)

    //Mat *hist = new Mat(img_display_0);
    //return (jlong) hist;

    Mat *hist = new Mat(img_display);
    return (jlong) hist;

    //return (jlong) &img_display;
}

/** @function main */
JNIEXPORT jlong JNICALL Java_com_example_carplate_CarPlateDetection_featureTest
(JNIEnv *env, jclass clz, jlong imgObject, jlong imgScene)
{
  Mat img_object =  Mat(*(Mat*)imgObject);
  Mat img_scene  =  Mat(*(Mat*)imgScene);

  LOGD("    featureTest  %d, %d      !",img_object.cols, img_object.rows);
  //-- Step 1: Detect the keypoints using SURF Detector
  int minHessian = 400;
  OrbFeatureDetector detector;

  vector<KeyPoint> keypoints_object, keypoints_scene;

  detector.detect(img_object, keypoints_object);
  detector.detect(img_scene, keypoints_scene);

  LOGD("     FastFeatureDetector    \n");
  //-- Step 2: Calculate descriptors (feature vectors)
  OrbDescriptorExtractor extractor;
  Mat descriptors_object, descriptors_scene;

  extractor.compute(img_object, keypoints_object, descriptors_object);
  extractor.compute(img_scene, keypoints_scene, descriptors_scene);

  LOGD("     BriefDescriptorExtractor    \n");

  //-- Step 3: Matching descriptor vectors using FLANN matcher
  FlannBasedMatcher matcher;
  //DescriptorMatcher matcher;
  //GenericDescriptorMatcher matcher;

  vector<DMatch> matches;
  matcher.match(descriptors_object, descriptors_scene, matches);

  LOGD("     FlannBasedMatcher    \n");
  double max_dist = 0; double min_dist = 100;

  //-- Quick calculation of max and min distances between keypoints
  for( int i = 0; i < descriptors_object.rows; i++ )
  { double dist = matches[i].distance;
    if( dist < min_dist ) min_dist = dist;
    if( dist > max_dist ) max_dist = dist;
  }

  LOGD("-- Max dist : %f \n", max_dist );
  LOGD("-- Min dist : %f \n", min_dist );

  //-- Draw only "good" matches (i.e. whose distance is less than 3*min_dist )
  vector< DMatch > good_matches;

  for( int i = 0; i < descriptors_object.rows; i++ )
  { if( matches[i].distance < 3*min_dist )
     { good_matches.push_back( matches[i]); }
  }

  Mat img_matches;
  drawMatches( img_object, keypoints_object, img_scene, keypoints_scene,
               good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
               vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );

  LOGD("     drawMatches    \n");
  //-- Localize the object
  vector<Point2f> obj;
  vector<Point2f> scene;

  for( int i = 0; i < good_matches.size(); i++ )
  {
    //-- Get the keypoints from the good matches
    obj.push_back( keypoints_object[ good_matches[i].queryIdx ].pt );
    scene.push_back( keypoints_scene[ good_matches[i].trainIdx ].pt );
  }

  Mat H = findHomography( obj, scene, CV_RANSAC );

  //-- Get the corners from the image_1 ( the object to be "detected" )
  vector<Point2f> obj_corners(4);
  obj_corners[0] = cvPoint(0,0); obj_corners[1] = cvPoint( img_object.cols, 0 );
  obj_corners[2] = cvPoint( img_object.cols, img_object.rows ); obj_corners[3] = cvPoint( 0, img_object.rows );
  vector<Point2f> scene_corners(4);

  perspectiveTransform( obj_corners, scene_corners, H);

  //-- Draw lines between the corners (the mapped object in the scene - image_2 )
  line( img_matches, scene_corners[0] + Point2f( img_object.cols, 0), scene_corners[1] + Point2f( img_object.cols, 0), Scalar(0, 255, 0), 4 );
  line( img_matches, scene_corners[1] + Point2f( img_object.cols, 0), scene_corners[2] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 4 );
  line( img_matches, scene_corners[2] + Point2f( img_object.cols, 0), scene_corners[3] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 4 );
  line( img_matches, scene_corners[3] + Point2f( img_object.cols, 0), scene_corners[0] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 4 );

  Mat *hist = new Mat(img_matches);
  return (jlong) hist;
}

//对minAreaRect获得的最小外接矩形，用纵横比进行判断
bool verifySizes_1(RotatedRect mr)
{
	float error=0.4;
	//Spain car plate size: 52x11 aspect 4,7272
	float aspect=4.7272;
	//Set a min and max area. All other patchs are discarded
	int min= 15*aspect*15; // minimum area
	int max= 125*aspect*125; // maximum area
	//Get only patchs that match to a respect ratio.
	float rmin= aspect-aspect*error;
	float rmax= aspect+aspect*error;

	int area= mr.size.height * mr.size.width;
	float r= (float)mr.size.width / (float)mr.size.height;
	if(r<1)
		r= (float)mr.size.height / (float)mr.size.width;

	if(( area < min || area > max ) || ( r < rmin || r > rmax )){
		return false;
	}else{
		return true;
	}
}

Mat histeq_1(Mat in)
{
	Mat out(in.size(), in.type());
	if(in.channels()==3){
		Mat hsv;
		vector<Mat> hsvSplit;
		cvtColor(in, hsv, CV_BGR2HSV);
		split(hsv, hsvSplit);
		equalizeHist(hsvSplit[2], hsvSplit[2]);
		merge(hsvSplit, hsv);
		cvtColor(hsv, out, CV_HSV2BGR);
	}else if(in.channels()==1){
		equalizeHist(in, out);
	}

	return out;
}

JNIEXPORT jlong JNICALL Java_com_example_carplate_CarPlateDetection_contourProc2
  (JNIEnv *env, jclass obj, jlong imageGray, jlong tempGray)
{
	LOGD("            contourProc2        !");
    LOGD("            contourProc2        !");
	//apply a Gaussian blur of 5 x 5 and remove noise
	Mat input_0 = Mat(*(Mat*)imageGray);
	//Mat input = Mat(input_0.size(), CV_8UC3);
	//Mat input(input_0.size(),CV_8UC3);
	Mat input;
	input.create(input_0.size(), CV_8UC3);
	cvtColor(input_0, input, CV_BGRA2BGR);

	Mat img_gray;
	cvtColor(input, img_gray, CV_BGR2GRAY);
	blur(img_gray, img_gray, Size(5,5));

	//Finde vertical edges. Car plates have high density of vertical lines
	Mat img_sobel;
	Sobel(img_gray, img_sobel, CV_8U, 1, 0, 3, 1, 0, BORDER_DEFAULT);//xorder=1,yorder=0,kernelsize=3

	//apply a threshold filter to obtain a binary image through Otsu's method
	Mat img_threshold;
	threshold(img_sobel, img_threshold, 0, 255, CV_THRESH_OTSU+CV_THRESH_BINARY);

	//Morphplogic operation close:remove blank spaces and connect all regions that have a high number of edges
	Mat element = getStructuringElement(MORPH_RECT, Size(17, 3) );
	morphologyEx(img_threshold, img_threshold, CV_MOP_CLOSE, element);

	//Find 轮廓 of possibles plates
	vector< vector< Point> > contours;
	findContours(img_threshold,
		contours, // a vector of contours
		CV_RETR_EXTERNAL, // 提取外部轮廓
		CV_CHAIN_APPROX_NONE); // all pixels of each contours

	//Start to iterate to each contour founded
	vector<vector<Point> >::iterator itc= contours.begin();
	vector<RotatedRect> rects;

	LOGD("         contours.size() = %d      !", contours.size());

	//Remove patch that are no inside limits of aspect ratio and area.
	while (itc!=contours.end()) {
		//Create bounding rect of object
		RotatedRect mr= minAreaRect(Mat(*itc));
		if( !verifySizes_1(mr)){
			itc= contours.erase(itc);
		}else{
			++itc;
			rects.push_back(mr);
		}
	}

	drawContours(input,contours,-1,Scalar(0,0,255),2);

	LOGD("         contours.size() = %d      !", contours.size());
	LOGD("         rects.size() = %d      !", rects.size());
	//cv::Mat result;
	//input.copyTo(result);

	for(int i=0; i< rects.size(); i++)
	{
		//get the min size between width and height
		float minSize=(rects[i].size.width < rects[i].size.height)?rects[i].size.width:rects[i].size.height;
		minSize=minSize-minSize*0.5;
		//initialize rand and get 5 points around center for floodfill algorithm
		srand ( time(NULL) );
		//Initialize floodfill parameters and variables
		Mat mask;
		mask.create(input.rows + 2, input.cols + 2, CV_8UC1);
		mask= Scalar::all(0);
		int loDiff = 30;
		int upDiff = 250;//30;
		int connectivity = 4;
		int newMaskVal = 255;
		int NumSeeds = 10;
		Rect ccomp;
		int flags = connectivity + (newMaskVal << 8 ) + CV_FLOODFILL_FIXED_RANGE + CV_FLOODFILL_MASK_ONLY;
		LOGD("         NumSeeds = %d      !", NumSeeds);
		for(int j=0; j<NumSeeds; j++){
			Point seed;
			seed.x=rects[i].center.x+rand()%(int)minSize-(minSize/2);
			seed.y=rects[i].center.y+rand()%(int)minSize-(minSize/2);
			int area = floodFill(input, mask, seed, Scalar(255,0,0), &ccomp, Scalar(loDiff, loDiff, loDiff), Scalar(upDiff, upDiff, upDiff), flags);
		}

		/*
		//Check new floodfill mask match for a correct patch.
		//Get all points detected for get Minimal rotated Rect
		vector<Point> pointsInterest;
		Mat_<uchar>::iterator itMask= mask.begin<uchar>();
		Mat_<uchar>::iterator end= mask.end<uchar>();
		for( ; itMask!=end; ++itMask)
			if(*itMask==255)
				pointsInterest.push_back(itMask.pos());

		RotatedRect minRect = minAreaRect(pointsInterest);

		if(verifySizes_1(minRect)){
			// rotated rectangle drawing
			Point2f rect_points[4]; minRect.points( rect_points );

			//Get rotation matrix
			float r= (float)minRect.size.width / (float)minRect.size.height;
			float angle=minRect.angle;
			if(r<1)
				angle=90+angle;
			Mat rotmat= getRotationMatrix2D(minRect.center, angle,1);

			//Create and rotate image
			Mat img_rotated;
			//void warpAffine( InputArray src, OutputArray dst,InputArray M, Size dsize,
            //                 int flags=INTER_LINEAR,int borderMode=BORDER_CONSTANT,const Scalar& borderValue=Scalar());
			warpAffine(input, img_rotated, rotmat, input.size(), CV_INTER_CUBIC);

			//Crop image
			Size rect_size=minRect.size;
			if(r < 1)
				swap(rect_size.width, rect_size.height);
			Mat img_crop;
			//void getRectSubPix( InputArray image, Size patchSize,
			//                    Point2f center, OutputArray patch, int patchType=-1 );
			getRectSubPix(img_rotated, rect_size, minRect.center, img_crop);

			Mat resultResized;
			resultResized.create(33,144, CV_8UC3);
			resize(img_crop, resultResized, resultResized.size(), 0, 0, INTER_CUBIC);
			//Equalize croped image
			Mat grayResult;
			cvtColor(resultResized, grayResult, CV_BGR2GRAY);
			blur(grayResult, grayResult, Size(3,3));
			grayResult=histeq_1(grayResult);
		}
		*/
	}

	Mat *hist = new Mat(input);
	return (jlong) hist;
}

#ifdef __cplusplus
}
#endif
