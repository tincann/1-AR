// INFOMCV-1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>


#define CAM_CONFIG "Config\\out_camera_data_custom.xml"
#define TEST_IMAGE "Config\\custom04.jpg"
#define SQUARE_SIZE 1

#define USE_CAMERA 1

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

void drawOverlay(Mat img, Mat rotation, Mat translation, Mat cameraMatrix, Mat distortion){
	
	vector<Point3f> axisPoints;
	axisPoints.push_back(Point3f(0, 0, 0));// middle point
	axisPoints.push_back(Point3f(3, 0, 0));// x axis
	axisPoints.push_back(Point3f(0, 3, 0));// y axis
	axisPoints.push_back(Point3f(0, 0, -3));// z axis
	
	//project the points that represent the 3d axis vectors to 2d
	vector<Point2f> imagePoints;
	projectPoints(axisPoints, rotation, translation, cameraMatrix, distortion, imagePoints); //always gives identical imagepoints -- strange

	//draw axis lines
	arrowedLine(img, imagePoints[0], imagePoints[1], CV_RGB(255, 0, 0), 3); //x
	arrowedLine(img, imagePoints[0], imagePoints[2], CV_RGB(0, 255, 0), 3); //y
	arrowedLine(img, imagePoints[0], imagePoints[3], CV_RGB(0, 0, 255), 3); //z

	vector<Point3f> boardPoints;
	boardPoints.push_back(Point3f(-1, -1, 0));
	boardPoints.push_back(Point3f(9, -1, 0));
	boardPoints.push_back(Point3f(9, 6, 0));
	boardPoints.push_back(Point3f(-1, 6, 0));
	
	vector<Point2f> pPoints;
	//projectPoints(boardPoints, rotation, translation, cameraMatrix, distortion, pPoints); //always gives identical imagepoints -- strange

	pPoints.clear();
	pPoints.push_back(Point(5, 5));
	pPoints.push_back(Point(200, 5));
	pPoints.push_back(Point(200, 200));
	pPoints.push_back(Point(5, 200));

	fillConvexPoly(img, pPoints, pPoints.size(), CV_RGB(0, 0, 0));
}

int _tmain(int argc, char* argv[])
{
	Size boardsize;
	boardsize.width = 6;
	boardsize.height = 9;
	auto points3d = Calculate3DPoints(boardsize.width, boardsize.height);

	//read camera config
	FileStorage fs(CAM_CONFIG, FileStorage::READ);
	Mat cameraMatrix, distortion;
	fs["Camera_Matrix"] >> cameraMatrix;
	fs["Distortion_Coefficients"] >> distortion;

#if USE_CAMERA
	VideoCapture cap(0);
	if (!cap.isOpened()){
		return -1;
	}

	namedWindow("Frame", 1);

	while (true){
		Mat frame;
		cap >> frame;
		
		
		//find chessboard corners
		vector<Point2f> points2d;
		bool found = findChessboardCorners(frame, boardsize, points2d, CV_CALIB_CB_ADAPTIVE_THRESH);

		if (found){
			Mat rvec, tvec;
			solvePnP(points3d, Mat(points2d), cameraMatrix, distortion, rvec, tvec);

			drawOverlay(frame, rvec, tvec, cameraMatrix, distortion);
		}
		imshow("Frame", frame);
		if (cv::waitKey(30) >= 0) break;
	}
#else
	//read image
	Mat img = imread(TEST_IMAGE, CV_LOAD_IMAGE_COLOR);
	//find chessboard corners
	vector<Point2f> points2d;
	bool found = findChessboardCorners(img, boardsize, points2d, CV_CALIB_CB_ADAPTIVE_THRESH);

	if (found){
		Mat rvec, tvec;
		solvePnP(points3d, Mat(points2d), cameraMatrix, distortion, rvec, tvec);

		drawOverlay(img, rvec, tvec, cameraMatrix, distortion);
	}
	imshow("Image", img);
	
	cv::waitKey(0);

#endif

	return 0;
}

