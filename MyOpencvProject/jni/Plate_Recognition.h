#ifndef rec_h
#define rec_h
#include <cv.h>
#include <highgui.h>
#include <cvaux.h>
#include <ml.h>

#include <iostream>
#include <vector>
#define HORIZONTAL    1
#define VERTICAL    0

using namespace std;
using namespace cv;

bool verifySizes(Mat r);
Mat preprocessChar(Mat in);
Mat ProjectedHistogram(Mat img, int t);
Mat features(Mat in, int sizeData);
int classify(Mat f,CvANN_MLP *ann);
void train(Mat TrainData, Mat classes,CvANN_MLP *ann,int nlayers);
#endif