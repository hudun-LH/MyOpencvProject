#include <string>  
#include <jni.h>
#include <android/log.h>
#include <iostream>
#include <fstream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/ml/ml.hpp>

#define LOG_TAG "libhogsvm"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

using namespace cv;  
using namespace std;  

#ifdef __cplusplus
extern "C" {
#endif

int init_run = 1;
HOGDescriptor hog(cvSize(64,64),cvSize(16,16),cvSize(8,8),cvSize(8,8),9); //��ⴰ��64*64

//��ȡSVM�����ӣ������ǰ�Ѿ�ѵ�����ˣ���txt�ļ���ʽ�����ˣ��ú������Ƕ�ȡ���ļ�����������vector��
vector<float>  get_detector0(const char* hogSVMDetectorPath)
{
    vector<float> detector;
    ifstream fileIn(hogSVMDetectorPath,ios::in);
    float val = 0.0f;
    while(!fileIn.eof())
    {
        fileIn>>val;
        detector.push_back(val);
    }
    fileIn.close();

    return detector;
}

void  hog_init0(void)
{
    vector<float> detector = get_detector0("/storage/emulated/0/HOGDetectorForOpenCV.txt");
    hog.setSVMDetector(detector);
}

JNIEXPORT void JNICALL Java_org_opencv_samples_tutorial2_Tutorial2Activity_HogFeatures_0(JNIEnv *env, jclass obj, jlong addrGray)
{
    //Mat& mGr  = *(Mat*)addrGray;
    Mat& mRgb = *(Mat*)addrGray;
    /*����������һ�����ųߴ磬��������̬���Ƽ�⣬����1920*1080P��ô�ߵķֱ��ʣ������Ƚ�ͼ�����ŵ�ԭ�����ķ�֮һ����Ȼ��Ҳ������������������ã�ֱ�ӽ��Ͳ����ֱ��ʡ�*/

    int scale=4;
    //Mat tmp(mGr.rows /scale, mGr.cols/scale, CV_8UC1);
    //resize(mGr, tmp, Size(tmp.cols, tmp.rows));

    vector<Rect>  found;

    if(init_run<25){
       if(init_run==1){
          hog_init0();
       }
       init_run++;
    }else{
       hog.detectMultiScale(mRgb, found, 0, cv::Size(8,8), cv::Size(32,32), 1.05, 2);//tmp
       if (found.size() > 0)
       {
    	  int i;
          for (i=0; i<found.size(); i++)
          {
             /*�õ��ľ��ο���Ҫ��Ӧ����4*/
             Rect tempRect(found[i].x * scale, found[i].y * scale, found[i].width * scale, found[i].height * scale);
             rectangle(mRgb, cvPoint(tempRect.x,tempRect.y),cvPoint(tempRect.x+tempRect.width,tempRect.y+tempRect.height),CV_RGB(0,255,0), 5);
          }
       }
    }
}

#ifdef __cplusplus
}
#endif

