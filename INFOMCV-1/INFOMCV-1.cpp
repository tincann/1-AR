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

void drawAxisSystem(Mat img, Mat rotation, Mat translation, Mat cameraMatrix, Mat distortion){
	
	vector<Point3f> axisPoints;
	axisPoints.push_back(Point3f(0, 0, 0));// middle point
	axisPoints.push_back(Point3f(1, 0, 0));// x axis
	axisPoints.push_back(Point3f(0, 1, 0));// y axis
	axisPoints.push_back(Point3f(0, 0, 1));// z axis

	//debug
	/*cv::Mat cam = Mat::zeros(3, 3, cv::DataType<double>::type);
	cam.at<double>(0, 0) = 1;
	cam.at<double>(1, 1) = 1;
	cam.at<double>(2, 2) = 1;

	cv::Mat distCoeffs(4, 1, cv::DataType<double>::type);
	distCoeffs.at<double>(0) = 0;
	distCoeffs.at<double>(1) = 0;
	distCoeffs.at<double>(2) = 0;
	distCoeffs.at<double>(3) = 0;

	Mat rot(3, 1, DataType<double>::type);
	rot.at<double>(0) = 0;
	rot.at<double>(1) = 0;
	rot.at<double>(2) = 0;

	Mat trans(3, 1, DataType<double>::type);
	rot.at<double>(0) = 0;
	rot.at<double>(1) = 0;
	rot.at<double>(2) = 0;
*/
	vector<Point2f> imagePoints;
	//project the points that represent the 3d axis vectors to 2d
	projectPoints(axisPoints, rotation, translation, cameraMatrix, distortion, imagePoints); //always gives identical imagepoints -- strange

	//draw the lines
	line(img, imagePoints[0], imagePoints[1], CV_RGB(255, 0, 0)); //x
	line(img, imagePoints[0], imagePoints[2], CV_RGB(0, 255, 0)); //y
	line(img, imagePoints[0], imagePoints[3], CV_RGB(0, 0, 255)); //z
}

int _tmain(int argc, char* argv[])
{
	//temporary
	argv = new char*[2];
	argv[1] = TEST_IMAGE;
	
	//read image
	Mat img = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);

	//find chessboard corners
	Size boardsize;
	boardsize.width = 6;
	boardsize.height = 9;
	vector<Point2f> points2d;
	bool found = findChessboardCorners(img, boardsize, points2d, CV_CALIB_CB_ADAPTIVE_THRESH);

	auto points3d = Calculate3DPoints(boardsize.width, boardsize.height);

	//read camera config
	FileStorage fs(CAM_CONFIG, FileStorage::READ);
	Mat cameraMatrix, distortion;
	fs["Camera_Matrix"] >> cameraMatrix;
	fs["Distortion_Coefficients"] >> distortion;

	Mat rvec, tvec;
	solvePnP(points3d, Mat(points2d), cameraMatrix, distortion, rvec, tvec, true);

	drawAxisSystem(img, rvec, tvec, cameraMatrix, distortion);

	imshow("Image", img);

	waitKey(0);

	return 0;
}

