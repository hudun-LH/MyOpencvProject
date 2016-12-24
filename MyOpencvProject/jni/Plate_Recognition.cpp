#include "Plate_Recognition.h"

const int numCharacters=30;

bool verifySizes(Mat r){
	//Char sizes 45x77
	float aspect=45.0f/77.0f;
	float charAspect= (float)r.cols/(float)r.rows;
	float error=0.35;
	float minHeight=15;
	float maxHeight=28;
	//We have a different aspect ratio for number 1, and it can be ~0.2
	float minAspect=0.2;
	float maxAspect=aspect+aspect*error;
	//area of pixels
	float area=countNonZero(r);
	//bb area
	float bbArea=r.cols*r.rows;
	// of pixel in area
	float percPixels=area/bbArea;

	if(percPixels < 0.8 && charAspect > minAspect && charAspect < maxAspect && r.rows >= minHeight && r.rows < maxHeight)
		return true;
	else
		return false;

}

Mat preprocessChar(Mat in){
	//Remap image
	int h=in.rows;
	int w=in.cols;
	int charSize=20;	//统一每个字符的大小
	Mat transformMat=Mat::eye(2,3,CV_32F); //指定使用的尺寸和数据类型
	int m=max(w,h);
	transformMat.at<float>(0,2)=m/2 - w/2;
	transformMat.at<float>(1,2)=m/2 - h/2;

	Mat warpImage(m,m, in.type());
	warpAffine(in, warpImage, transformMat, warpImage.size(), INTER_LINEAR, BORDER_CONSTANT, Scalar(0) );

	Mat out;
	resize(warpImage, out, Size(charSize, charSize) ); 

	return out;
}

//create the accumulation histograms,img is a binary image, t is 水平或垂直
Mat ProjectedHistogram(Mat img, int t)
{
	int sz=(t)?img.rows:img.cols;
	Mat mhist=Mat::zeros(1,sz,CV_32F);//MATLAB风格的初始化函数：zeros()， ones()，指定使用的尺寸和数据类型
    //Mat C = (Mat_<double>(3,3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);
    /*
	2D 点
	Point2f P(5, 1);
	3D 点
	Point3f P3f(2, 6, 7);
	*/
	for(int j=0; j<sz; j++){
		Mat data=(t)?img.row(j):img.col(j);
		mhist.at<float>(j)=countNonZero(data);	//统计这一行或一列中，非零元素的个数，并保存到mhist中
	}

	//Normalize histogram
	double min, max;
	minMaxLoc(mhist, &min, &max);

	if(max>0)
		//void Mat::convertTo( Mat& m, int rtype, double alpha=1, double beta=0 )
		/*
 输入参数：
m 目标矩阵。如果m的大小与原矩阵不一样，或者数据类型与参数不匹配，那么在函数convertTo内部会先给m重新分配空间。
rtype 指定从原矩阵进行转换后的数据类型，即目标矩阵m的数据类型。当然，矩阵m的通道数应该与原矩阵一样的。
如果rtype是负数，那么m矩阵的数据类型应该与原矩阵一样。
alpha 缩放因子。默认值是1。即把原矩阵中的每一个元素都乘以alpha。
beta 增量。默认值是0。即把原矩阵中的每一个元素都乘以alpha，再加上beta。
	    */
		mhist.convertTo(mhist,-1 , 1.0f/max, 0);//用mhist直方图中的最大值，归一化直方图

	return mhist;
}

Mat features(Mat in, int sizeData){
	//Histogram features
	Mat vhist=ProjectedHistogram(in,VERTICAL);
	Mat hhist=ProjectedHistogram(in,HORIZONTAL);

	//Low data feature
	Mat lowData;
	/*
	void resize(InputArray src, OutputArray dst, Size dsize, double fx=0, double fy=0, int interpolation=INTER_LINEAR )
    //resize(img1,img2,Size(32,32),0,0,CV_INTER_LINEAR);

	pyrDown与pyrUp分别表示向下降采样与向上升采样，但是二者并不是互为逆操作；
	void pyrDown(InputArray src, OutputArray dst, const Size& dstsize=Size(), int border-Type=BORDER_DEFAULT )
	void pyrUp(InputArray src, OutputArray dst, const Size& dstsize=Size(), int border-Type=BORDER_DEFAULT )
	*/
	resize(in, lowData, Size(sizeData, sizeData) );

	//Last 10 is the number of moments components
	int numCols=vhist.cols+hhist.cols+lowData.cols*lowData.cols;

	Mat out=Mat::zeros(1,numCols,CV_32F);
	//Asign values to feature,ANN的样本特征为水平、垂直直方图和低分辨率图像所组成的矢量
	int j=0;
	for(int i=0; i<vhist.cols; i++)
	{
		out.at<float>(j)=vhist.at<float>(i);
		j++;
	}
	for(int i=0; i<hhist.cols; i++)
	{
		out.at<float>(j)=hhist.at<float>(i);
		j++;
	}
	for(int x=0; x<lowData.cols; x++)
	{
		for(int y=0; y<lowData.rows; y++){
			out.at<float>(j)=(float)lowData.at<unsigned char>(x,y);
			j++;
		}
	}

	return out;
}


int classify(Mat f,CvANN_MLP *ann){
	int result=-1;
	Mat output(1, 30, CV_32FC1); //西班牙车牌只有30种字符
	(*ann).predict(f, output);
	Point maxLoc;
	double maxVal;
	minMaxLoc(output, 0, &maxVal, 0, &maxLoc);
	return maxLoc.x;
}

void train(Mat TrainData, Mat classes,CvANN_MLP *ann,int nlayers){
	Mat layers(1,3,CV_32SC1);
	layers.at<int>(0)= TrainData.cols;
	layers.at<int>(1)= nlayers;
	layers.at<int>(2)= 30;
	(*ann).create(layers, CvANN_MLP::SIGMOID_SYM, 1, 1);

	//Prepare trainClases
	//Create a mat with n trained data by m classes
	Mat trainClasses;
	trainClasses.create( TrainData.rows, 30, CV_32FC1 );
	for( int i = 0; i <  trainClasses.rows; i++ )
	{
		for( int k = 0; k < trainClasses.cols; k++ )
		{
			//If class of data i is same than a k class
			if( k == classes.at<int>(i) )
				trainClasses.at<float>(i,k) = 1;
			else
				trainClasses.at<float>(i,k) = 0;
		}
	}
	Mat weights( 1, TrainData.rows, CV_32FC1, Scalar::all(1) );

	//Learn classifier
	(*ann).train( TrainData, trainClasses, weights );
}
