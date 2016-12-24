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

using namespace std;
using namespace cv;

#define LOG_TAG "FeatureTest"
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))

#ifdef __cplusplus
extern "C" {
#endif

/*
//对运动物体的跟踪：
//如果背景固定,可用帧差法 然后在计算下连通域 将面积小的去掉即可
//如果背景单一,即你要跟踪的物体颜色和背景色有较大区别 可用基于颜色的跟踪 如CAMSHIFT 鲁棒性都是较好的
//如果背景复杂,如背景中有和前景一样的颜色 就需要用到一些具有预测性的算法 如卡尔曼滤波等 可以和CAMSHIFT结合
cvCreateImage：创建头并分配数据
IplImage* cvCreateImage( CvSize size, int depth, int channels );
参数说明：
size 图像宽、高.
depth 图像元素的位深度，可以是下面的其中之一：
IPL_DEPTH_8U - 无符号8位整型
IPL_DEPTH_8S - 有符号8位整型
IPL_DEPTH_16U - 无符号16位整型
IPL_DEPTH_16S - 有符号16位整型
IPL_DEPTH_32S - 有符号32位整型
IPL_DEPTH_32F - 单精度浮点数
IPL_DEPTH_64F - 双精度浮点数
channels：
每个元素（像素）通道号.可以是 1, 2, 3 或 4.通道是交叉存取的，例如通常的彩色图像数据排列是：b0 g0 r0 b1 g1 r1 ...
虽然通常 IPL 图象格式可以存贮非交叉存取的图像，并且一些OpenCV 也能处理他, 但是这个函数只能创建交叉存取图像.

//Meanshift跟踪算法返回的Box类
typedef struct CvBox2D{
   CvPoint2D32f center; // 盒子的中心
   CvSize2D32f size; // 盒子的长和宽
   float angle; // 水平轴与第一个边的夹角，用弧度表示
}CvBox2D;

typedef struct CvConnectedComp{
   double area; // 连通域的面积
   float value; // 分割域的灰度缩放值
   CvRect rect; // 分割域的 ROI
} CvConnectedComp;
*/
IplImage  *image = 0;
IplImage  *hsv = 0;
IplImage  *hue = 0;
IplImage  *mask = 0;
IplImage  *backproject = 0;
IplImage  *histimg = 0;     //用HSV中的Hue分量进行跟踪
CvConnectedComp track_comp; //连接部件
CvHistogram *hist = 0;      //直方图类
int backproject_mode = 0;
int select_object = 0;
signed int track_object = 0;
int bin_w;
CvPoint origin;
CvRect  selection;
CvRect  track_window;
CvBox2D track_box;

int hdims = 16;//划分直方图bins的个数，越多越精确
float hranges_arr[] = {0,180};//像素值的范围
float* hranges = hranges_arr;//用于初始化CvHistogram类
int vmin = 10, vmax = 256, smin = 30;//用于设置滑动条
CvScalar hsv2rgb( float hue )//用于将Hue量转换成RGB量
{
	int rgb[3], p, sector;
	static const int sector_data[][3]={{0,2,1}, {1,2,0}, {1,0,2}, {2,0,1}, {2,1,0}, {0,1,2}};
	hue *= 0.033333333333333333333333333333333f;
	sector = cvFloor(hue);
	p = cvRound(255*(hue - sector));
	p ^= sector & 1 ? 255 : 0;
	rgb[sector_data[sector][0]] = 255;
	rgb[sector_data[sector][1]] = 0;
	rgb[sector_data[sector][2]] = p;
	return cvScalar(rgb[2], rgb[1], rgb[0],0);
}

JNIEXPORT int JNICALL Java_com_example_carplate_CarPlateDetection_doSelectRect(JNIEnv *env, jclass clz, jint imgW, jint imgH)
{
	LOGD("          doSelectRect start       !");
	selection.x = imgH/2;
	selection.y = imgW/2;
	selection.width  = 150;
	selection.height = 150;
	select_object = 1;
	track_object  = 0;
	LOGD("         doSelectRect %d, %d      !", imgW,imgH);
}

int select_run;
JNIEXPORT jlong JNICALL Java_com_example_carplate_CarPlateDetection_doCamshift(JNIEnv *env, jclass clz, jlong imageGray)
{
	int i;
	Mat imageMat = Mat(*(Mat*)imageGray);
	IplImage  temp_src = imageMat;
	IplImage* frame = &temp_src;
	//IplImage* frame = 0;
	//frame=&IplImage(*((Mat*)imageGray));

	if( !image )//image为0,表明刚开始还未对image操作过,先建立一些缓冲区
	{
		 LOGD("            doCamshift init        !");
		 image = cvCreateImage( cvGetSize(frame), 8, 3 );
		 image->origin = frame->origin;
		 hsv = cvCreateImage( cvGetSize(frame), 8, 3 );
		 hue = cvCreateImage( cvGetSize(frame), 8, 1 );
		 mask = cvCreateImage( cvGetSize(frame), 8, 1 );
		 //分配掩膜图像空间
		 backproject = cvCreateImage( cvGetSize(frame), 8, 1 );
		 //分配反向投影图空间,大小一样,单通道
		 hist = cvCreateHist( 1, &hdims, CV_HIST_ARRAY, &hranges, 1 );
		 //分配直方图空间
		 histimg = cvCreateImage( cvSize(320,200), 8, 3 );
		 //分配用于直方图显示的空间
		 cvZero( histimg ); //置背景为黑色
	}
	//cvCopy( frame, image, 0 );
	cvCvtColor(frame, image, CV_BGRA2BGR);
	cvCvtColor( image, hsv, CV_BGR2HSV ); //把图像从RGB表色系转为HSV表色系
	if(track_object>0) //track_object非零,表示有需要跟踪的物体
	{
		 LOGD("       -------  doCamshift ss-------     !");
	     int _vmin = vmin, _vmax = vmax;
	     cvInRangeS( hsv, cvScalar(0,smin,MIN(_vmin,_vmax),0),
	                        cvScalar(180,256,MAX(_vmin,_vmax),0), mask );
	     //制作掩膜板，只处理像素值为H：0~180，S：smin~256，V：vmin~vmax之间的部分
	     cvSplit( hsv, hue, 0, 0, 0 );//分离H分量

	     cvCalcBackProject( &hue, backproject, hist );//计算hue的反向投影图
	     cvAnd( backproject, mask, backproject, 0 );//得到掩膜内的反向投影
	     cvCamShift( backproject, track_window,
	                        cvTermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ),
	                        &track_comp, &track_box );
	     //使用MeanShift算法对backproject中的内容进行搜索,返回跟踪结果
	     if(track_comp.rect.width>0 && track_comp.rect.height>0){
	        track_window = track_comp.rect;//得到跟踪结果的矩形框
	     }

	     if( backproject_mode )
	         cvCvtColor( backproject, image, CV_GRAY2BGR );
	     if( image->origin )
	         track_box.angle = -track_box.angle;
	     cvEllipseBox( image, track_box, CV_RGB(255,0,0), 3, CV_AA, 0 );//画出跟踪结果的位置
	     select_run++;
	     if(select_run>25){
	        select_run = 0;
	        //LOGD("       -------  doCamshift -------     !");
	     }
	     LOGD("       -------  doCamshift ee -------     !");
	     Mat mtx(image,0);
	     Mat *hist = new Mat(mtx);
	     return (jlong) hist;
	 }
	 else if(track_object < 0)//如果需要跟踪的物体还没有进行属性提取，则进行选取框类的图像属性提取
	 {
		LOGD("            doCamshift select  ss         !");
	    float max_val = 0.f;
	    cvSetImageROI( hue, selection );//设置原选择框为ROI
	    cvSetImageROI( mask, selection );//设置掩膜板选择框为ROI
	    cvCalcHist( &hue, hist, 0, mask );//得到选择框内且满足掩膜板内的直方图
	    cvGetMinMaxHistValue( hist, 0, &max_val, 0, 0 );
	    cvConvertScale( hist->bins, hist->bins, max_val ? 255. / max_val : 0., 0 );// 对直方图的数值转为0~255
	    cvResetImageROI( hue );//去除ROI
	    cvResetImageROI( mask );//去除ROI
	    track_window = selection;
	    track_object = 1;//置track_object为1,表明属性提取完成
	    cvZero( histimg );
	    bin_w = histimg->width / hdims;
	    for( i = 0; i < hdims; i++ )//画直方图到图像空间
	    {
	        int val = cvRound( cvGetReal1D(hist->bins,i)*histimg->height/255 );
	        CvScalar color = hsv2rgb(i*180.f/hdims);
	        cvRectangle( histimg, cvPoint(i*bin_w,histimg->height),
	                                 cvPoint((i+1)*bin_w,histimg->height - val),color, -1, 8, 0 );
	    }
	    LOGD("            doCamshift select  ee          !");
        //Mat mtx(histimg,0);
        //Mat *hist = new Mat(mtx);
	    //return (jlong) hist;

	    return 1;
	}
	if( select_object && selection.width > 0 && selection.height > 0 ) //如果正处于物体选择，画出选择框
	{
	    LOGD("            doCamshift select 00       !");
	    cvSetImageROI( image, selection );
	    cvXorS( image, cvScalarAll(255), image, 0 );
	    cvResetImageROI( image );
	    select_run++;
	    if(select_run>25){
	       select_run = 0;
	       select_object = 0;
	       track_object  = -1;
	       track_window = selection;
	       LOGD("          doSelectRect %d       !",track_object);
	    }
	    Mat mtx(image,0);
	    Mat *hist = new Mat(mtx);
	    return (jlong) hist;
	 }
	 return 0;
}

CvHaarClassifierCascade* load_object_detector( const char* cascade_path )
{
    return (CvHaarClassifierCascade*)cvLoad( cascade_path );
}

void detect_and_draw_objects( IplImage* image,
                              CvHaarClassifierCascade* cascade,
                              int do_pyramids )
{
    IplImage* small_image = image;
    CvMemStorage* storage = cvCreateMemStorage(0);
    CvSeq* faces;
    int i, scale = 1;

    /* if the flag is specified, down-scale the 输入图像 to get a performance boost w/o loosing quality (perhaps)
	    如果标识（do_puramids）是指定的,把输入图像缩小比例来获得一个性能的提升 without 不稳定的质量*/
    if( do_pyramids )
    {
        small_image = cvCreateImage( cvSize(image->width/2,image->height/2), IPL_DEPTH_8U, 3 ); //sceneMat.type()
        cvPyrDown( image, small_image, CV_GAUSSIAN_5x5 );
		/*功能：函数cvPyrDown使用Gaussian金字塔分解对输入图像向下采样。
		格式：void cvPyrDown(const CvArr*src,CvArr*dst,int filter=CV_GAUSSIAN_5x5);
		参数：src 输入图像。
			  dst 输出图像，其宽度和高度应是输入图像的一半。
			  filter 卷积滤波器类型，目前仅支持CV_GAUSSIAN_5x5。*/
        scale = 2;
    }

    /* use the fastest variant */
	/*使用了最快速的检测方式*/
    faces = cvHaarDetectObjects( small_image, cascade, storage, 1.1, 2, CV_HAAR_DO_CANNY_PRUNING );

    /* draw all the rectangles */
    for( i = 0; i < faces->total; i++ )
    {
        /* extract the rectanlges only */
        //CvRect face_rect = *(CvRect*)cvGetSeqElem( faces, i, 0 );
		CvRect face_rect = *(CvRect*)cvGetSeqElem( faces, i );
        cvRectangle( image, cvPoint(face_rect.x*scale,face_rect.y*scale),
                     cvPoint((face_rect.x+face_rect.width)*scale,
                             (face_rect.y+face_rect.height)*scale),
                     CV_RGB(255,0,0), 3 );
    }

    if( small_image != image )
        cvReleaseImage( &small_image );
    cvReleaseMemStorage( &storage );
}
//  .exe testImageAddress 分类器地址（xml）
/* takes image filename and cascade path from the command line */
JNIEXPORT jlong JNICALL Java_com_example_carplate_CarPlateDetection_doHaarClassifier
(JNIEnv *env, jclass clz, jlong imageGray)
{
	LOGD("          doHaarClassifier         !");
	Mat imageMat = Mat(*(Mat*)imageGray);
	IplImage  temp_src = imageMat;
	IplImage* image = &temp_src;

	//const char* classPath = "/storage/emulated/0/OCR.xml";
    //CvHaarClassifierCascade* cascade=(CvHaarClassifierCascade*)cvLoad("/storage/emulated/0/haarcascade_frontalface_default.xml");
    CvHaarClassifierCascade* cascade = load_object_detector("/storage/emulated/0/haarcascade_fullbody.xml");//haarcascade_frontalface_default
    detect_and_draw_objects(image, cascade, 0 );
    cvReleaseHaarClassifierCascade( &cascade );
    //cvReleaseImage( &image );

    Mat mtx(image,0);
    Mat *hist = new Mat(mtx);
    return (jlong) hist;
}

#ifdef __cplusplus
}
#endif
