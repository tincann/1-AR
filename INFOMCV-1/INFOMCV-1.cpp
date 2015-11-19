// INFOMCV-1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "../CameraCalibration/CameraCalibration.h"

using namespace cv;
using namespace std;

Settings* readConfig(int argc, char* argv[]){
	Settings s;
	const string inputSettingsFile = argc > 1 ? argv[1] : "default.xml";
	FileStorage fs(inputSettingsFile, FileStorage::READ); // Read the settings
	if (!fs.isOpened())
	{
		cout << "Could not open the configuration file: \"" << inputSettingsFile << "\"" << endl;
		return nullptr;
	}
	fs["Settings"] >> s;

	fs.release();                                         // close Settings file

	if (!s.goodInput)
	{
		cout << "Invalid input detected. Application stopping. " << endl;
		return nullptr;
	}

	return &s;
}

int _tmain(int argc, char* argv[])
{
	Mat img = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);

	auto s = readConfig(argc, argv);
	vector<Point2f> ptvec;
	bool found = findChessboardCorners(img, s->boardSize, ptvec, CV_CALIB_CB_ADAPTIVE_THRESH);

	return 0;
}

