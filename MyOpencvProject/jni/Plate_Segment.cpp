#include "Plate_Segment.h"

//对minAreaRect获得的最小外接矩形，用纵横比进行判断
bool verifySizes(RotatedRect mr)
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

Mat histeq(Mat in)
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

vector<Plate> segment(Mat input){
	vector<Plate> output;

	//apply a Gaussian blur of 5 x 5 and remove noise
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

	//Remove patch that are no inside limits of aspect ratio and area.    
	while (itc!=contours.end()) {
		//Create bounding rect of object
		RotatedRect mr= minAreaRect(Mat(*itc));
		if( !verifySizes(mr)){
			itc= contours.erase(itc);
		}else{
			++itc;
			rects.push_back(mr);
		}
	}

	cv::Mat result;
	input.copyTo(result);

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
		int upDiff = 30;
		int connectivity = 4;
		int newMaskVal = 255;
		int NumSeeds = 10;
		Rect ccomp;
		int flags = connectivity + (newMaskVal << 8 ) + CV_FLOODFILL_FIXED_RANGE + CV_FLOODFILL_MASK_ONLY;
		for(int j=0; j<NumSeeds; j++){
			Point seed;
			seed.x=rects[i].center.x+rand()%(int)minSize-(minSize/2);
			seed.y=rects[i].center.y+rand()%(int)minSize-(minSize/2);
			int area = floodFill(input, mask, seed, Scalar(255,0,0), &ccomp, Scalar(loDiff, loDiff, loDiff), Scalar(upDiff, upDiff, upDiff), flags);
		}

		//Check new floodfill mask match for a correct patch.
		//Get all points detected for get Minimal rotated Rect
		vector<Point> pointsInterest;
		Mat_<uchar>::iterator itMask= mask.begin<uchar>();
		Mat_<uchar>::iterator end= mask.end<uchar>();
		for( ; itMask!=end; ++itMask)
			if(*itMask==255)
				pointsInterest.push_back(itMask.pos());

		RotatedRect minRect = minAreaRect(pointsInterest);

		if(verifySizes(minRect)){
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
			warpAffine(input, img_rotated, rotmat, input.size(), CV_INTER_CUBIC);

			//Crop image
			Size rect_size=minRect.size;
			if(r < 1)
				swap(rect_size.width, rect_size.height);
			Mat img_crop;
			getRectSubPix(img_rotated, rect_size, minRect.center, img_crop);

			Mat resultResized;
			resultResized.create(33,144, CV_8UC3);
			resize(img_crop, resultResized, resultResized.size(), 0, 0, INTER_CUBIC);
			//Equalize croped image
			Mat grayResult;
			cvtColor(resultResized, grayResult, CV_BGR2GRAY); 
			blur(grayResult, grayResult, Size(3,3));
			grayResult=histeq(grayResult);
			output.push_back(Plate(grayResult,minRect.boundingRect()));
		}
	}
	return output;
}