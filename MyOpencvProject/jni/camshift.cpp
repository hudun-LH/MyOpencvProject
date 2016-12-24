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
//���˶�����ĸ��٣�
//��������̶�,����֡� Ȼ���ڼ�������ͨ�� �����С��ȥ������
//���������һ,����Ҫ���ٵ�������ɫ�ͱ���ɫ�нϴ����� ���û�����ɫ�ĸ��� ��CAMSHIFT ³���Զ��ǽϺõ�
//�����������,�米�����к�ǰ��һ������ɫ ����Ҫ�õ�һЩ����Ԥ���Ե��㷨 �翨�����˲��� ���Ժ�CAMSHIFT���
cvCreateImage������ͷ����������
IplImage* cvCreateImage( CvSize size, int depth, int channels );
����˵����
size ͼ�����.
depth ͼ��Ԫ�ص�λ��ȣ����������������֮һ��
IPL_DEPTH_8U - �޷���8λ����
IPL_DEPTH_8S - �з���8λ����
IPL_DEPTH_16U - �޷���16λ����
IPL_DEPTH_16S - �з���16λ����
IPL_DEPTH_32S - �з���32λ����
IPL_DEPTH_32F - �����ȸ�����
IPL_DEPTH_64F - ˫���ȸ�����
channels��
ÿ��Ԫ�أ����أ�ͨ����.������ 1, 2, 3 �� 4.ͨ���ǽ����ȡ�ģ�����ͨ���Ĳ�ɫͼ�����������ǣ�b0 g0 r0 b1 g1 r1 ...
��Ȼͨ�� IPL ͼ���ʽ���Դ����ǽ����ȡ��ͼ�񣬲���һЩOpenCV Ҳ�ܴ�����, �����������ֻ�ܴ��������ȡͼ��.

//Meanshift�����㷨���ص�Box��
typedef struct CvBox2D{
   CvPoint2D32f center; // ���ӵ�����
   CvSize2D32f size; // ���ӵĳ��Ϳ�
   float angle; // ˮƽ�����һ���ߵļнǣ��û��ȱ�ʾ
}CvBox2D;

typedef struct CvConnectedComp{
   double area; // ��ͨ������
   float value; // �ָ���ĻҶ�����ֵ
   CvRect rect; // �ָ���� ROI
} CvConnectedComp;
*/
IplImage  *image = 0;
IplImage  *hsv = 0;
IplImage  *hue = 0;
IplImage  *mask = 0;
IplImage  *backproject = 0;
IplImage  *histimg = 0;     //��HSV�е�Hue�������и���
CvConnectedComp track_comp; //���Ӳ���
CvHistogram *hist = 0;      //ֱ��ͼ��
int backproject_mode = 0;
int select_object = 0;
signed int track_object = 0;
int bin_w;
CvPoint origin;
CvRect  selection;
CvRect  track_window;
CvBox2D track_box;

int hdims = 16;//����ֱ��ͼbins�ĸ�����Խ��Խ��ȷ
float hranges_arr[] = {0,180};//����ֵ�ķ�Χ
float* hranges = hranges_arr;//���ڳ�ʼ��CvHistogram��
int vmin = 10, vmax = 256, smin = 30;//�������û�����
CvScalar hsv2rgb( float hue )//���ڽ�Hue��ת����RGB��
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

	if( !image )//imageΪ0,�����տ�ʼ��δ��image������,�Ƚ���һЩ������
	{
		 LOGD("            doCamshift init        !");
		 image = cvCreateImage( cvGetSize(frame), 8, 3 );
		 image->origin = frame->origin;
		 hsv = cvCreateImage( cvGetSize(frame), 8, 3 );
		 hue = cvCreateImage( cvGetSize(frame), 8, 1 );
		 mask = cvCreateImage( cvGetSize(frame), 8, 1 );
		 //������Ĥͼ��ռ�
		 backproject = cvCreateImage( cvGetSize(frame), 8, 1 );
		 //���䷴��ͶӰͼ�ռ�,��Сһ��,��ͨ��
		 hist = cvCreateHist( 1, &hdims, CV_HIST_ARRAY, &hranges, 1 );
		 //����ֱ��ͼ�ռ�
		 histimg = cvCreateImage( cvSize(320,200), 8, 3 );
		 //��������ֱ��ͼ��ʾ�Ŀռ�
		 cvZero( histimg ); //�ñ���Ϊ��ɫ
	}
	//cvCopy( frame, image, 0 );
	cvCvtColor(frame, image, CV_BGRA2BGR);
	cvCvtColor( image, hsv, CV_BGR2HSV ); //��ͼ���RGB��ɫϵתΪHSV��ɫϵ
	if(track_object>0) //track_object����,��ʾ����Ҫ���ٵ�����
	{
		 LOGD("       -------  doCamshift ss-------     !");
	     int _vmin = vmin, _vmax = vmax;
	     cvInRangeS( hsv, cvScalar(0,smin,MIN(_vmin,_vmax),0),
	                        cvScalar(180,256,MAX(_vmin,_vmax),0), mask );
	     //������Ĥ�壬ֻ��������ֵΪH��0~180��S��smin~256��V��vmin~vmax֮��Ĳ���
	     cvSplit( hsv, hue, 0, 0, 0 );//����H����

	     cvCalcBackProject( &hue, backproject, hist );//����hue�ķ���ͶӰͼ
	     cvAnd( backproject, mask, backproject, 0 );//�õ���Ĥ�ڵķ���ͶӰ
	     cvCamShift( backproject, track_window,
	                        cvTermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ),
	                        &track_comp, &track_box );
	     //ʹ��MeanShift�㷨��backproject�е����ݽ�������,���ظ��ٽ��
	     if(track_comp.rect.width>0 && track_comp.rect.height>0){
	        track_window = track_comp.rect;//�õ����ٽ���ľ��ο�
	     }

	     if( backproject_mode )
	         cvCvtColor( backproject, image, CV_GRAY2BGR );
	     if( image->origin )
	         track_box.angle = -track_box.angle;
	     cvEllipseBox( image, track_box, CV_RGB(255,0,0), 3, CV_AA, 0 );//�������ٽ����λ��
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
	 else if(track_object < 0)//�����Ҫ���ٵ����廹û�н���������ȡ�������ѡȡ�����ͼ��������ȡ
	 {
		LOGD("            doCamshift select  ss         !");
	    float max_val = 0.f;
	    cvSetImageROI( hue, selection );//����ԭѡ���ΪROI
	    cvSetImageROI( mask, selection );//������Ĥ��ѡ���ΪROI
	    cvCalcHist( &hue, hist, 0, mask );//�õ�ѡ�������������Ĥ���ڵ�ֱ��ͼ
	    cvGetMinMaxHistValue( hist, 0, &max_val, 0, 0 );
	    cvConvertScale( hist->bins, hist->bins, max_val ? 255. / max_val : 0., 0 );// ��ֱ��ͼ����ֵתΪ0~255
	    cvResetImageROI( hue );//ȥ��ROI
	    cvResetImageROI( mask );//ȥ��ROI
	    track_window = selection;
	    track_object = 1;//��track_objectΪ1,����������ȡ���
	    cvZero( histimg );
	    bin_w = histimg->width / hdims;
	    for( i = 0; i < hdims; i++ )//��ֱ��ͼ��ͼ��ռ�
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
	if( select_object && selection.width > 0 && selection.height > 0 ) //�������������ѡ�񣬻���ѡ���
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

    /* if the flag is specified, down-scale the ����ͼ�� to get a performance boost w/o loosing quality (perhaps)
	    �����ʶ��do_puramids����ָ����,������ͼ����С���������һ�����ܵ����� without ���ȶ�������*/
    if( do_pyramids )
    {
        small_image = cvCreateImage( cvSize(image->width/2,image->height/2), IPL_DEPTH_8U, 3 ); //sceneMat.type()
        cvPyrDown( image, small_image, CV_GAUSSIAN_5x5 );
		/*���ܣ�����cvPyrDownʹ��Gaussian�������ֽ������ͼ�����²�����
		��ʽ��void cvPyrDown(const CvArr*src,CvArr*dst,int filter=CV_GAUSSIAN_5x5);
		������src ����ͼ��
			  dst ���ͼ�����Ⱥ͸߶�Ӧ������ͼ���һ�롣
			  filter ����˲������ͣ�Ŀǰ��֧��CV_GAUSSIAN_5x5��*/
        scale = 2;
    }

    /* use the fastest variant */
	/*ʹ��������ٵļ�ⷽʽ*/
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
//  .exe testImageAddress ��������ַ��xml��
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
