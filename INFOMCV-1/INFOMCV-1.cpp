// INFOMCV-1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

#define SQUARE_SIZE 20

using namespace cv;
using namespace std;

//builds the checkerboard in 3d space
vector<Point3f> Calculate3DPoints(int width, int height){
	vector<Point3f> boardPoints;
	for (int y = 0; y < height; y++) for (int x = 0; x < width; x++)
	{
		boardPoints.push_back(Point3f(x * SQUARE_SIZE, y * SQUARE_SIZE, 0));
	}

	return boardPoints;
}

int _tmain(int argc, char* argv[])
{
	argv = new char*[2];
	argv[1] = "Config\\left01.jpg";
	Mat img = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);

	//find chessboard corners
	Size boardsize;
	boardsize.width = 6;
	boardsize.height = 9;
	vector<Point2f> points2d;
	bool found = findChessboardCorners(img, boardsize, points2d, CV_CALIB_CB_ADAPTIVE_THRESH);


	auto points3d = Calculate3DPoints(boardsize.width, boardsize.height);

	//read camera config
	FileStorage fs("Config\\out_camera_data.xml", FileStorage::READ);
	Mat cameraMatrix, distortion;
	fs["Camera_Matrix"] >> cameraMatrix;
	fs["Distortion_Coefficients"] >> distortion;
	

	vector<Point2f> rvec, tvec;
	solvePnP(points3d, points2d, cameraMatrix, distortion, rvec, tvec, true);


	return 0;
}

