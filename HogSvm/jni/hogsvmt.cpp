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

#define PosSamNO 2400    //����������
#define NegSamNO 12000    //����������

#define TRAIN false    //�Ƿ����ѵ��,true��ʾ����ѵ����false��ʾ��ȡxml�ļ��е�SVMģ��
#define CENTRAL_CROP true   //true:ѵ��ʱ����96*160��INRIA������ͼƬ���ó��м��64*128��С����

//HardExample�����������������HardExampleNO����0����ʾ�������ʼ���������󣬼�������HardExample����������
//��ʹ��HardExampleʱ��������Ϊ0����Ϊ������������������������ά����ʼ��ʱ�õ����ֵ
#define HardExampleNO 4435

#define LOG_TAG "libhogsvm"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

using namespace cv;  
using namespace std;  

HOGDescriptor myHOG;
bool read_train;

#ifdef __cplusplus
extern "C" {
#endif

//�̳���CvSVM���࣬��Ϊ����setSVMDetector()���õ��ļ���Ӳ���ʱ����Ҫ�õ�ѵ���õ�SVM��decision_func������
//��ͨ���鿴CvSVMԴ���֪decision_func������protected���ͱ������޷�ֱ�ӷ��ʵ���ֻ�ܼ̳�֮��ͨ����������
class MySVM : public CvSVM
{
public:
	//���SVM�ľ��ߺ����е�alpha����
	double * get_alpha_vector()
	{
		return this->decision_func->alpha;
	}

	//���SVM�ľ��ߺ����е�rho����,��ƫ����
	float get_rho()
	{
		return this->decision_func->rho;
	}
};

int train()
{
	//��ⴰ��(64,128),��ߴ�(16,16),�鲽��(8,8),cell�ߴ�(8,8),ֱ��ͼbin����9
	HOGDescriptor hog(Size(64,128),Size(16,16),Size(8,8),Size(8,8),9);//HOG���������������HOG�����ӵ�
	int DescriptorDim;//HOG�����ӵ�ά������ͼƬ��С����ⴰ�ڴ�С�����С��ϸ����Ԫ��ֱ��ͼbin��������
	MySVM svm;//SVM������

	//��TRAINΪtrue������ѵ��������
	if(TRAIN)
	{
		string ImgName;//ͼƬ��(����·��)
		ifstream finPos("INRIAPerson96X160PosList.txt");//������ͼƬ���ļ����б�
		//ifstream finPos("PersonFromVOC2012List.txt");//������ͼƬ���ļ����б�
		ifstream finNeg("NoPersonFromINRIAList.txt");//������ͼƬ���ļ����б�

		Mat sampleFeatureMat;//����ѵ������������������ɵľ��������������������ĸ�������������HOG������ά��
		Mat sampleLabelMat;//ѵ����������������������������������ĸ�������������1��1��ʾ���ˣ�-1��ʾ����


		//���ζ�ȡ������ͼƬ������HOG������
		for(int num=0; num<PosSamNO && getline(finPos,ImgName); num++)
		{
			cout<<"����"<<ImgName<<endl;
			//ImgName = "D:\\DataSet\\PersonFromVOC2012\\" + ImgName;//������������·����
			ImgName = "D:\\DataSet\\INRIAPerson\\INRIAPerson\\96X160H96\\Train\\pos\\" + ImgName;//������������·����
			Mat src = imread(ImgName);//��ȡͼƬ
			if(CENTRAL_CROP)
				src = src(Rect(16,16,64,128));//��96*160��INRIA������ͼƬ����Ϊ64*128������ȥ�������Ҹ�16������
			//resize(src,src,Size(64,128));

			vector<float> descriptors;//HOG����������
			hog.compute(src,descriptors,Size(8,8));//����HOG�����ӣ���ⴰ���ƶ�����(8,8)
			//cout<<"������ά����"<<descriptors.size()<<endl;

			//�����һ������ʱ��ʼ�����������������������Ϊֻ��֪��������������ά�����ܳ�ʼ��������������
			if( 0 == num )
			{
				DescriptorDim = descriptors.size();//HOG�����ӵ�ά��
				//��ʼ������ѵ������������������ɵľ��������������������ĸ�������������HOG������ά��sampleFeatureMat
				sampleFeatureMat = Mat::zeros(PosSamNO+NegSamNO+HardExampleNO, DescriptorDim, CV_32FC1);
				//��ʼ��ѵ����������������������������������ĸ�������������1��1��ʾ���ˣ�0��ʾ����
				sampleLabelMat = Mat::zeros(PosSamNO+NegSamNO+HardExampleNO, 1, CV_32FC1);
			}

			//������õ�HOG�����Ӹ��Ƶ�������������sampleFeatureMat
			for(int i=0; i<DescriptorDim; i++)
				sampleFeatureMat.at<float>(num,i) = descriptors[i];//��num�����������������еĵ�i��Ԫ��
			sampleLabelMat.at<float>(num,0) = 1;//���������Ϊ1������
		}

		//���ζ�ȡ������ͼƬ������HOG������
		for(int num=0; num<NegSamNO && getline(finNeg,ImgName); num++)
		{
			cout<<"����"<<ImgName<<endl;
			ImgName = "D:\\DataSet\\NoPersonFromINRIA\\" + ImgName;//���ϸ�������·����
			Mat src = imread(ImgName);//��ȡͼƬ
			//resize(src,img,Size(64,128));

			vector<float> descriptors;//HOG����������
			hog.compute(src,descriptors,Size(8,8));//����HOG�����ӣ���ⴰ���ƶ�����(8,8)
			//cout<<"������ά����"<<descriptors.size()<<endl;

			//������õ�HOG�����Ӹ��Ƶ�������������sampleFeatureMat
			for(int i=0; i<DescriptorDim; i++)
				sampleFeatureMat.at<float>(num+PosSamNO,i) = descriptors[i];//��PosSamNO+num�����������������еĵ�i��Ԫ��
			sampleLabelMat.at<float>(num+PosSamNO,0) = -1;//���������Ϊ-1������
		}

		//����HardExample������
		if(HardExampleNO > 0)
		{
			ifstream finHardExample("HardExample_2400PosINRIA_12000NegList.txt");//HardExample���������ļ����б�
			//���ζ�ȡHardExample������ͼƬ������HOG������
			for(int num=0; num<HardExampleNO && getline(finHardExample,ImgName); num++)
			{
				cout<<"����"<<ImgName<<endl;
				ImgName = "D:\\DataSet\\HardExample_2400PosINRIA_12000Neg\\" + ImgName;//����HardExample��������·����
				Mat src = imread(ImgName);//��ȡͼƬ
				//resize(src,img,Size(64,128));

				vector<float> descriptors;//HOG����������
				hog.compute(src,descriptors,Size(8,8));//����HOG�����ӣ���ⴰ���ƶ�����(8,8)
				//cout<<"������ά����"<<descriptors.size()<<endl;

				//������õ�HOG�����Ӹ��Ƶ�������������sampleFeatureMat
				for(int i=0; i<DescriptorDim; i++)
					sampleFeatureMat.at<float>(num+PosSamNO+NegSamNO,i) = descriptors[i];//��PosSamNO+num�����������������еĵ�i��Ԫ��
				sampleLabelMat.at<float>(num+PosSamNO+NegSamNO,0) = -1;//���������Ϊ-1������
			}
		}

		////���������HOG�������������ļ�
		//ofstream fout("SampleFeatureMat.txt");
		//for(int i=0; i<PosSamNO+NegSamNO; i++)
		//{
		//	fout<<i<<endl;
		//	for(int j=0; j<DescriptorDim; j++)
		//		fout<<sampleFeatureMat.at<float>(i,j)<<"  ";
		//	fout<<endl;
		//}

		//ѵ��SVM������
		//������ֹ��������������1000�λ����С��FLT_EPSILONʱֹͣ����
		CvTermCriteria criteria = cvTermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS, 1000, FLT_EPSILON);
		//SVM������SVM����ΪC_SVC�����Ժ˺������ɳ�����C=0.01
		CvSVMParams param(CvSVM::C_SVC, CvSVM::LINEAR, 0, 1, 0, 0.01, 0, 0, 0, criteria);
		cout<<"��ʼѵ��SVM������"<<endl;
		svm.train(sampleFeatureMat, sampleLabelMat, Mat(), Mat(), param);//ѵ��������
		cout<<"ѵ�����"<<endl;
		svm.save("SVM_HOG.xml");//��ѵ���õ�SVMģ�ͱ���Ϊxml�ļ�

	}
	else //��TRAINΪfalse����XML�ļ���ȡѵ���õķ�����
	{
		LOGD("     svm.load     ");
		svm.load("/storage/emulated/0/SVM_HOG.xml");//��XML�ļ���ȡѵ���õ�SVMģ��
	}


	/*************************************************************************************************
	����SVMѵ����ɺ�õ���XML�ļ����棬��һ�����飬����support vector������һ�����飬����alpha,��һ��������������rho;
	��alpha����ͬsupport vector��ˣ�ע�⣬alpha*supportVector,���õ�һ����������֮���ٸ���������������һ��Ԫ��rho��
	��ˣ���õ���һ�������������ø÷�������ֱ���滻opencv�����˼��Ĭ�ϵ��Ǹ���������cv::HOGDescriptor::setSVMDetector()����
	�Ϳ����������ѵ������ѵ�������ķ������������˼���ˡ�
	***************************************************************************************************/
	DescriptorDim = svm.get_var_count();//����������ά������HOG�����ӵ�ά��
	int supportVectorNum = svm.get_support_vector_count();//֧�������ĸ���
	//cout<<"֧������������"<<supportVectorNum<<endl;
	LOGD("֧������������%d", supportVectorNum);

	Mat alphaMat = Mat::zeros(1, supportVectorNum, CV_32FC1);//alpha���������ȵ���֧����������
	Mat supportVectorMat = Mat::zeros(supportVectorNum, DescriptorDim, CV_32FC1);//֧����������
	Mat resultMat = Mat::zeros(1, DescriptorDim, CV_32FC1);//alpha��������֧����������Ľ��

	//��֧�����������ݸ��Ƶ�supportVectorMat������
	for(int i=0; i<supportVectorNum; i++)
	{
		const float * pSVData = svm.get_support_vector(i);//���ص�i��֧������������ָ��
		for(int j=0; j<DescriptorDim; j++)
		{
			//cout<<pData[j]<<" ";
			supportVectorMat.at<float>(i,j) = pSVData[j];
		}
	}

	//��alpha���������ݸ��Ƶ�alphaMat��
	double * pAlphaData = svm.get_alpha_vector();//����SVM�ľ��ߺ����е�alpha����
	for(int i=0; i<supportVectorNum; i++)
	{
		alphaMat.at<float>(0,i) = pAlphaData[i];
	}

	//����-(alphaMat * supportVectorMat),����ŵ�resultMat��
	//gemm(alphaMat, supportVectorMat, -1, 0, 1, resultMat);//��֪��Ϊʲô�Ӹ��ţ�
	resultMat = -1 * alphaMat * supportVectorMat;

	//�õ����յ�setSVMDetector(const vector<float>& detector)�����п��õļ����
	vector<float> myDetector;
	//��resultMat�е����ݸ��Ƶ�����myDetector��
	for(int i=0; i<DescriptorDim; i++)
	{
		myDetector.push_back(resultMat.at<float>(0,i));
	}
	//������ƫ����rho���õ������
	myDetector.push_back(svm.get_rho());
	//cout<<"�����ά����"<<myDetector.size()<<endl;
	LOGD("�����ά����%d", myDetector.size());
	//����HOGDescriptor�ļ����
	HOGDescriptor myHOG;
	myHOG.setSVMDetector(myDetector);
	//myHOG.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());

	//�������Ӳ������ļ�
	ofstream fout("/storage/emulated/0/HOGDetectorForOpenCV.txt");//HOGDetectorForOpenCV.txt
	for(int i=0; i<myDetector.size(); i++)
	{
		fout<<myDetector[i]<<endl;
	}

	LOGD("     train finished!    ");
}

int detect(){
	/**************����ͼƬ����HOG���˼��******************/
	//Mat src = imread("00000.jpg");
	//Mat src = imread("2007_000423.jpg");
	Mat src = imread("1.png");
	vector<Rect> found, found_filtered;//���ο�����
	cout<<"���ж�߶�HOG������"<<endl;
	myHOG.detectMultiScale(src, found, 0, Size(8,8), Size(32,32), 1.05, 2);//��ͼƬ���ж�߶����˼��
	cout<<"�ҵ��ľ��ο������"<<found.size()<<endl;

	//�ҳ�����û��Ƕ�׵ľ��ο�r,������found_filtered��,�����Ƕ�׵Ļ�,��ȡ���������Ǹ����ο����found_filtered��
	for(int i=0; i < found.size(); i++)
	{
		Rect r = found[i];
		int j=0;
		for(; j < found.size(); j++)
			if(j != i && (r & found[j]) == r)
				break;
		if( j == found.size())
			found_filtered.push_back(r);
	}

	//�����ο���Ϊhog�����ľ��ο��ʵ�������Ҫ��΢��Щ,����������Ҫ��һЩ����
	for(int i=0; i<found_filtered.size(); i++)
	{
		Rect r = found_filtered[i];
		r.x += cvRound(r.width*0.1);
		r.width = cvRound(r.width*0.8);
		r.y += cvRound(r.height*0.07);
		r.height = cvRound(r.height*0.8);
		rectangle(src, r.tl(), r.br(), Scalar(0,255,0), 3);
	}

	imwrite("ImgProcessed.jpg",src);
	namedWindow("src",0);
	imshow("src",src);
}

vector<float>  get_detector(const char* hogSVMDetectorPath)
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



JNIEXPORT void JNICALL Java_org_opencv_samples_tutorial2_Tutorial2Activity_HogSvmTrain(JNIEnv *env, jclass obj)
{
	 read_train = false;
	 train();
	 read_train = true;
	 //LOGD("       HogSvmTrain  start    ");
	 //vector<float> detector = get_detector("/storage/emulated/0/HOGDetectorForOpenCV.txt");
	 //myHOG.setSVMDetector(detector);

	 LOGD("       HogSvmTrain     ");
}

IplImage *desc;
double scale = 0.5f;
Size sz;
Mat image2;

JNIEXPORT void JNICALL Java_org_opencv_samples_tutorial2_Tutorial2Activity_HogFeatures(JNIEnv *env, jclass obj, jlong addrGray) //, jlong addrRgba)
{
    //Mat& src  = *(Mat*)addrGray;
    //Mat& mRgb = *(Mat*)addrRgba;

    /*����������һ�����ųߴ磬��������̬���Ƽ�⣬����1920*1080P��ô�ߵķֱ��ʣ������Ƚ�ͼ�����ŵ�ԭ�����ķ�֮һ����Ȼ��Ҳ������������������ã�ֱ�ӽ��Ͳ����ֱ��ʡ�*/
    //int scale = 4 ;
    //Mat tmp(mGr.rows /scale, mGr.cols/scale, CV_8UC1);
    //resize(mGr, tmp, Size(tmp.cols, tmp.rows));

	if(read_train)
	{
		    Mat& src  = *(Mat*)addrGray;
		    // public static void resize(Mat src, Mat dst, Size dsize, double fx, double fy, int interpolation)

		    //sz.width = src.cols*scale;
		    //sz.height = src.rows*scale;
		    //desc = cvCreateImage(sz, IPL_DEPTH_8U, 1);
		    //image2 = new Mat(sz,CvType.CV_8UC1);
		    //resize(src, image2, sz, 0, 0, CV_INTER_CUBIC);

			vector<Rect> found, found_filtered;//���ο�����

			//cout<<"���ж�߶�HOG������"<<endl;
			LOGD("HOG people test");
			myHOG.detectMultiScale(src, found, 0, Size(16,16), Size(64,64), 1.05, 2);//��ͼƬ���ж�߶����˼��
			//myHOG.detectMultiScale(src, found, 0, Size(8,8), Size(32,32), 1.05, 2);

			//cout<<"�ҵ��ľ��ο������"<<found.size()<<endl;
			LOGD("found rect = %d", found.size());

            rectangle(src, cvPoint(0,0),cvPoint(64,64), CV_RGB(0,255,0), 2);


			int scale = 4;
			if (found.size() > 0)
			{
			        for (int i=0; i<found.size(); i++)
			        {
			            /*�õ��ľ��ο���Ҫ��Ӧ����4*/
			            Rect tempRect(found[i].x * scale, found[i].y * scale, found[i].width * scale, found[i].height * scale);
			            rectangle(src, cvPoint(tempRect.x,tempRect.y),cvPoint(tempRect.x+tempRect.width,tempRect.y+tempRect.height),CV_RGB(0,255,0), 5);
			        }
			 }

			/*
			//�ҳ�����û��Ƕ�׵ľ��ο�r,������found_filtered��,�����Ƕ�׵Ļ�,��ȡ���������Ǹ����ο����found_filtered��
			for(int i=0; i < found.size(); i++)
			{
				Rect r = found[i];
				int j=0;
				for(; j < found.size(); j++)
					if(j != i && (r & found[j]) == r)
						break;
				if( j == found.size())
					found_filtered.push_back(r);
			}

			//�����ο���Ϊhog�����ľ��ο��ʵ�������Ҫ��΢��Щ,����������Ҫ��һЩ����
			for(int i=0; i<found_filtered.size(); i++)
			{
				Rect r = found_filtered[i];
				r.x += cvRound(r.width*0.1);
				r.width = cvRound(r.width*0.8);
				r.y += cvRound(r.height*0.07);
				r.height = cvRound(r.height*0.8);
				rectangle(src, r.tl(), r.br(), Scalar(0,255,0), 3);
			}
			*/
	}
}

#ifdef __cplusplus
}
#endif

