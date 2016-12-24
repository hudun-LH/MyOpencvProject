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
	int charSize=20;	//ͳһÿ���ַ��Ĵ�С
	Mat transformMat=Mat::eye(2,3,CV_32F); //ָ��ʹ�õĳߴ����������
	int m=max(w,h);
	transformMat.at<float>(0,2)=m/2 - w/2;
	transformMat.at<float>(1,2)=m/2 - h/2;

	Mat warpImage(m,m, in.type());
	warpAffine(in, warpImage, transformMat, warpImage.size(), INTER_LINEAR, BORDER_CONSTANT, Scalar(0) );

	Mat out;
	resize(warpImage, out, Size(charSize, charSize) ); 

	return out;
}

//create the accumulation histograms,img is a binary image, t is ˮƽ��ֱ
Mat ProjectedHistogram(Mat img, int t)
{
	int sz=(t)?img.rows:img.cols;
	Mat mhist=Mat::zeros(1,sz,CV_32F);//MATLAB���ĳ�ʼ��������zeros()�� ones()��ָ��ʹ�õĳߴ����������
    //Mat C = (Mat_<double>(3,3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);
    /*
	2D ��
	Point2f P(5, 1);
	3D ��
	Point3f P3f(2, 6, 7);
	*/
	for(int j=0; j<sz; j++){
		Mat data=(t)?img.row(j):img.col(j);
		mhist.at<float>(j)=countNonZero(data);	//ͳ����һ�л�һ���У�����Ԫ�صĸ����������浽mhist��
	}

	//Normalize histogram
	double min, max;
	minMaxLoc(mhist, &min, &max);

	if(max>0)
		//void Mat::convertTo( Mat& m, int rtype, double alpha=1, double beta=0 )
		/*
 ���������
m Ŀ��������m�Ĵ�С��ԭ����һ�����������������������ƥ�䣬��ô�ں���convertTo�ڲ����ȸ�m���·���ռ䡣
rtype ָ����ԭ�������ת������������ͣ���Ŀ�����m���������͡���Ȼ������m��ͨ����Ӧ����ԭ����һ���ġ�
���rtype�Ǹ�������ôm�������������Ӧ����ԭ����һ����
alpha �������ӡ�Ĭ��ֵ��1������ԭ�����е�ÿһ��Ԫ�ض�����alpha��
beta ������Ĭ��ֵ��0������ԭ�����е�ÿһ��Ԫ�ض�����alpha���ټ���beta��
	    */
		mhist.convertTo(mhist,-1 , 1.0f/max, 0);//��mhistֱ��ͼ�е����ֵ����һ��ֱ��ͼ

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

	pyrDown��pyrUp�ֱ��ʾ���½����������������������Ƕ��߲����ǻ�Ϊ�������
	void pyrDown(InputArray src, OutputArray dst, const Size& dstsize=Size(), int border-Type=BORDER_DEFAULT )
	void pyrUp(InputArray src, OutputArray dst, const Size& dstsize=Size(), int border-Type=BORDER_DEFAULT )
	*/
	resize(in, lowData, Size(sizeData, sizeData) );

	//Last 10 is the number of moments components
	int numCols=vhist.cols+hhist.cols+lowData.cols*lowData.cols;

	Mat out=Mat::zeros(1,numCols,CV_32F);
	//Asign values to feature,ANN����������Ϊˮƽ����ֱֱ��ͼ�͵ͷֱ���ͼ������ɵ�ʸ��
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
	Mat output(1, 30, CV_32FC1); //����������ֻ��30���ַ�
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
