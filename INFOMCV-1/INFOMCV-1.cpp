// INFOMCV-1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;

int _tmain(int argc, char* argv[])
{
	argv = new char*[2];
	argv[1] = "..\\Config\\left01.jpg";
	Mat img = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);

	Size boardsize;
	boardsize.width = 6;
	boardsize.height = 9;
	vector<Point2f> ptvec;
	bool found = findChessboardCorners(img, boardsize, ptvec, CV_CALIB_CB_ADAPTIVE_THRESH);

	return 0;
}

