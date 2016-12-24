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

#define PosSamNO 2400    //正样本个数
#define NegSamNO 12000    //负样本个数

#define TRAIN false    //是否进行训练,true表示重新训练，false表示读取xml文件中的SVM模型
#define CENTRAL_CROP true   //true:训练时，对96*160的INRIA正样本图片剪裁出中间的64*128大小人体

//HardExample：负样本个数。如果HardExampleNO大于0，表示处理完初始负样本集后，继续处理HardExample负样本集。
//不使用HardExample时必须设置为0，因为特征向量矩阵和特征类别矩阵的维数初始化时用到这个值
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

//继承自CvSVM的类，因为生成setSVMDetector()中用到的检测子参数时，需要用到训练好的SVM的decision_func参数，
//但通过查看CvSVM源码可知decision_func参数是protected类型变量，无法直接访问到，只能继承之后通过函数访问
class MySVM : public CvSVM
{
public:
	//获得SVM的决策函数中的alpha数组
	double * get_alpha_vector()
	{
		return this->decision_func->alpha;
	}

	//获得SVM的决策函数中的rho参数,即偏移量
	float get_rho()
	{
		return this->decision_func->rho;
	}
};

int train()
{
	//检测窗口(64,128),块尺寸(16,16),块步长(8,8),cell尺寸(8,8),直方图bin个数9
	HOGDescriptor hog(Size(64,128),Size(16,16),Size(8,8),Size(8,8),9);//HOG检测器，用来计算HOG描述子的
	int DescriptorDim;//HOG描述子的维数，由图片大小、检测窗口大小、块大小、细胞单元中直方图bin个数决定
	MySVM svm;//SVM分类器

	//若TRAIN为true，重新训练分类器
	if(TRAIN)
	{
		string ImgName;//图片名(绝对路径)
		ifstream finPos("INRIAPerson96X160PosList.txt");//正样本图片的文件名列表
		//ifstream finPos("PersonFromVOC2012List.txt");//正样本图片的文件名列表
		ifstream finNeg("NoPersonFromINRIAList.txt");//负样本图片的文件名列表

		Mat sampleFeatureMat;//所有训练样本的特征向量组成的矩阵，行数等于所有样本的个数，列数等于HOG描述子维数
		Mat sampleLabelMat;//训练样本的类别向量，行数等于所有样本的个数，列数等于1；1表示有人，-1表示无人


		//依次读取正样本图片，生成HOG描述子
		for(int num=0; num<PosSamNO && getline(finPos,ImgName); num++)
		{
			cout<<"处理："<<ImgName<<endl;
			//ImgName = "D:\\DataSet\\PersonFromVOC2012\\" + ImgName;//加上正样本的路径名
			ImgName = "D:\\DataSet\\INRIAPerson\\INRIAPerson\\96X160H96\\Train\\pos\\" + ImgName;//加上正样本的路径名
			Mat src = imread(ImgName);//读取图片
			if(CENTRAL_CROP)
				src = src(Rect(16,16,64,128));//将96*160的INRIA正样本图片剪裁为64*128，即剪去上下左右各16个像素
			//resize(src,src,Size(64,128));

			vector<float> descriptors;//HOG描述子向量
			hog.compute(src,descriptors,Size(8,8));//计算HOG描述子，检测窗口移动步长(8,8)
			//cout<<"描述子维数："<<descriptors.size()<<endl;

			//处理第一个样本时初始化特征向量矩阵和类别矩阵，因为只有知道了特征向量的维数才能初始化特征向量矩阵
			if( 0 == num )
			{
				DescriptorDim = descriptors.size();//HOG描述子的维数
				//初始化所有训练样本的特征向量组成的矩阵，行数等于所有样本的个数，列数等于HOG描述子维数sampleFeatureMat
				sampleFeatureMat = Mat::zeros(PosSamNO+NegSamNO+HardExampleNO, DescriptorDim, CV_32FC1);
				//初始化训练样本的类别向量，行数等于所有样本的个数，列数等于1；1表示有人，0表示无人
				sampleLabelMat = Mat::zeros(PosSamNO+NegSamNO+HardExampleNO, 1, CV_32FC1);
			}

			//将计算好的HOG描述子复制到样本特征矩阵sampleFeatureMat
			for(int i=0; i<DescriptorDim; i++)
				sampleFeatureMat.at<float>(num,i) = descriptors[i];//第num个样本的特征向量中的第i个元素
			sampleLabelMat.at<float>(num,0) = 1;//正样本类别为1，有人
		}

		//依次读取负样本图片，生成HOG描述子
		for(int num=0; num<NegSamNO && getline(finNeg,ImgName); num++)
		{
			cout<<"处理："<<ImgName<<endl;
			ImgName = "D:\\DataSet\\NoPersonFromINRIA\\" + ImgName;//加上负样本的路径名
			Mat src = imread(ImgName);//读取图片
			//resize(src,img,Size(64,128));

			vector<float> descriptors;//HOG描述子向量
			hog.compute(src,descriptors,Size(8,8));//计算HOG描述子，检测窗口移动步长(8,8)
			//cout<<"描述子维数："<<descriptors.size()<<endl;

			//将计算好的HOG描述子复制到样本特征矩阵sampleFeatureMat
			for(int i=0; i<DescriptorDim; i++)
				sampleFeatureMat.at<float>(num+PosSamNO,i) = descriptors[i];//第PosSamNO+num个样本的特征向量中的第i个元素
			sampleLabelMat.at<float>(num+PosSamNO,0) = -1;//负样本类别为-1，无人
		}

		//处理HardExample负样本
		if(HardExampleNO > 0)
		{
			ifstream finHardExample("HardExample_2400PosINRIA_12000NegList.txt");//HardExample负样本的文件名列表
			//依次读取HardExample负样本图片，生成HOG描述子
			for(int num=0; num<HardExampleNO && getline(finHardExample,ImgName); num++)
			{
				cout<<"处理："<<ImgName<<endl;
				ImgName = "D:\\DataSet\\HardExample_2400PosINRIA_12000Neg\\" + ImgName;//加上HardExample负样本的路径名
				Mat src = imread(ImgName);//读取图片
				//resize(src,img,Size(64,128));

				vector<float> descriptors;//HOG描述子向量
				hog.compute(src,descriptors,Size(8,8));//计算HOG描述子，检测窗口移动步长(8,8)
				//cout<<"描述子维数："<<descriptors.size()<<endl;

				//将计算好的HOG描述子复制到样本特征矩阵sampleFeatureMat
				for(int i=0; i<DescriptorDim; i++)
					sampleFeatureMat.at<float>(num+PosSamNO+NegSamNO,i) = descriptors[i];//第PosSamNO+num个样本的特征向量中的第i个元素
				sampleLabelMat.at<float>(num+PosSamNO+NegSamNO,0) = -1;//负样本类别为-1，无人
			}
		}

		////输出样本的HOG特征向量矩阵到文件
		//ofstream fout("SampleFeatureMat.txt");
		//for(int i=0; i<PosSamNO+NegSamNO; i++)
		//{
		//	fout<<i<<endl;
		//	for(int j=0; j<DescriptorDim; j++)
		//		fout<<sampleFeatureMat.at<float>(i,j)<<"  ";
		//	fout<<endl;
		//}

		//训练SVM分类器
		//迭代终止条件，当迭代满1000次或误差小于FLT_EPSILON时停止迭代
		CvTermCriteria criteria = cvTermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS, 1000, FLT_EPSILON);
		//SVM参数：SVM类型为C_SVC；线性核函数；松弛因子C=0.01
		CvSVMParams param(CvSVM::C_SVC, CvSVM::LINEAR, 0, 1, 0, 0.01, 0, 0, 0, criteria);
		cout<<"开始训练SVM分类器"<<endl;
		svm.train(sampleFeatureMat, sampleLabelMat, Mat(), Mat(), param);//训练分类器
		cout<<"训练完成"<<endl;
		svm.save("SVM_HOG.xml");//将训练好的SVM模型保存为xml文件

	}
	else //若TRAIN为false，从XML文件读取训练好的分类器
	{
		LOGD("     svm.load     ");
		svm.load("/storage/emulated/0/SVM_HOG.xml");//从XML文件读取训练好的SVM模型
	}


	/*************************************************************************************************
	线性SVM训练完成后得到的XML文件里面，有一个数组，叫做support vector，还有一个数组，叫做alpha,有一个浮点数，叫做rho;
	将alpha矩阵同support vector相乘，注意，alpha*supportVector,将得到一个列向量。之后，再该列向量的最后添加一个元素rho。
	如此，变得到了一个分类器，利用该分类器，直接替换opencv中行人检测默认的那个分类器（cv::HOGDescriptor::setSVMDetector()），
	就可以利用你的训练样本训练出来的分类器进行行人检测了。
	***************************************************************************************************/
	DescriptorDim = svm.get_var_count();//特征向量的维数，即HOG描述子的维数
	int supportVectorNum = svm.get_support_vector_count();//支持向量的个数
	//cout<<"支持向量个数："<<supportVectorNum<<endl;
	LOGD("支持向量个数：%d", supportVectorNum);

	Mat alphaMat = Mat::zeros(1, supportVectorNum, CV_32FC1);//alpha向量，长度等于支持向量个数
	Mat supportVectorMat = Mat::zeros(supportVectorNum, DescriptorDim, CV_32FC1);//支持向量矩阵
	Mat resultMat = Mat::zeros(1, DescriptorDim, CV_32FC1);//alpha向量乘以支持向量矩阵的结果

	//将支持向量的数据复制到supportVectorMat矩阵中
	for(int i=0; i<supportVectorNum; i++)
	{
		const float * pSVData = svm.get_support_vector(i);//返回第i个支持向量的数据指针
		for(int j=0; j<DescriptorDim; j++)
		{
			//cout<<pData[j]<<" ";
			supportVectorMat.at<float>(i,j) = pSVData[j];
		}
	}

	//将alpha向量的数据复制到alphaMat中
	double * pAlphaData = svm.get_alpha_vector();//返回SVM的决策函数中的alpha向量
	for(int i=0; i<supportVectorNum; i++)
	{
		alphaMat.at<float>(0,i) = pAlphaData[i];
	}

	//计算-(alphaMat * supportVectorMat),结果放到resultMat中
	//gemm(alphaMat, supportVectorMat, -1, 0, 1, resultMat);//不知道为什么加负号？
	resultMat = -1 * alphaMat * supportVectorMat;

	//得到最终的setSVMDetector(const vector<float>& detector)参数中可用的检测子
	vector<float> myDetector;
	//将resultMat中的数据复制到数组myDetector中
	for(int i=0; i<DescriptorDim; i++)
	{
		myDetector.push_back(resultMat.at<float>(0,i));
	}
	//最后添加偏移量rho，得到检测子
	myDetector.push_back(svm.get_rho());
	//cout<<"检测子维数："<<myDetector.size()<<endl;
	LOGD("检测子维数：%d", myDetector.size());
	//设置HOGDescriptor的检测子
	HOGDescriptor myHOG;
	myHOG.setSVMDetector(myDetector);
	//myHOG.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());

	//保存检测子参数到文件
	ofstream fout("/storage/emulated/0/HOGDetectorForOpenCV.txt");//HOGDetectorForOpenCV.txt
	for(int i=0; i<myDetector.size(); i++)
	{
		fout<<myDetector[i]<<endl;
	}

	LOGD("     train finished!    ");
}

int detect(){
	/**************读入图片进行HOG行人检测******************/
	//Mat src = imread("00000.jpg");
	//Mat src = imread("2007_000423.jpg");
	Mat src = imread("1.png");
	vector<Rect> found, found_filtered;//矩形框数组
	cout<<"进行多尺度HOG人体检测"<<endl;
	myHOG.detectMultiScale(src, found, 0, Size(8,8), Size(32,32), 1.05, 2);//对图片进行多尺度行人检测
	cout<<"找到的矩形框个数："<<found.size()<<endl;

	//找出所有没有嵌套的矩形框r,并放入found_filtered中,如果有嵌套的话,则取外面最大的那个矩形框放入found_filtered中
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

	//画矩形框，因为hog检测出的矩形框比实际人体框要稍微大些,所以这里需要做一些调整
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

    /*这里设置了一个缩放尺寸，由于做静态手势检测，无需1920*1080P这么高的分辨率，所以先将图像缩放到原来的四分之一，当然，也可以在相机属性上设置，直接降低采样分辨率。*/
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

			vector<Rect> found, found_filtered;//矩形框数组

			//cout<<"进行多尺度HOG人体检测"<<endl;
			LOGD("HOG people test");
			myHOG.detectMultiScale(src, found, 0, Size(16,16), Size(64,64), 1.05, 2);//对图片进行多尺度行人检测
			//myHOG.detectMultiScale(src, found, 0, Size(8,8), Size(32,32), 1.05, 2);

			//cout<<"找到的矩形框个数："<<found.size()<<endl;
			LOGD("found rect = %d", found.size());

            rectangle(src, cvPoint(0,0),cvPoint(64,64), CV_RGB(0,255,0), 2);


			int scale = 4;
			if (found.size() > 0)
			{
			        for (int i=0; i<found.size(); i++)
			        {
			            /*得到的矩形框需要相应乘以4*/
			            Rect tempRect(found[i].x * scale, found[i].y * scale, found[i].width * scale, found[i].height * scale);
			            rectangle(src, cvPoint(tempRect.x,tempRect.y),cvPoint(tempRect.x+tempRect.width,tempRect.y+tempRect.height),CV_RGB(0,255,0), 5);
			        }
			 }

			/*
			//找出所有没有嵌套的矩形框r,并放入found_filtered中,如果有嵌套的话,则取外面最大的那个矩形框放入found_filtered中
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

			//画矩形框，因为hog检测出的矩形框比实际人体框要稍微大些,所以这里需要做一些调整
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

