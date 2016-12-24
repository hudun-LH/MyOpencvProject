#ifndef seg_h
#define seg_h

#include<iostream>
#include <cv.h>
#include <highgui.h>
#include <cvaux.h>
#include "Plate.h"

using namespace std;
using namespace cv;

bool verifySizes(RotatedRect mr);
Mat histeq(Mat in);
vector<Plate> segment(Mat input);

#endif

