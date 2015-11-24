// INFOMCV-1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>


#define CAM_CONFIG "Config\\out_camera_data.xml"
#define TEST_IMAGE "Config\\left04.jpg"
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

//polygon draw function that can receive vector<Point2f>
void drawPoly(Mat img, vector<Point2f> pPoints, Scalar color){
	vector<Point> dst;
	Mat(pPoints).convertTo(dst, Mat(dst).type());
	fillConvexPoly(img, dst, color);
}

void drawPoly(Mat img, Point2f points[], int ptCount, Scalar color){
	vector<Point> dst;
	for (int i = 0; i < ptCount; i++){
		dst.push_back(points[i]);
	}
	fillConvexPoly(img, dst, CV_RGB(0, 0, 0));
}

//draw a 3d cube
void drawBox(Mat img, vector<Point2f> p, Scalar color){
	Point2f points1[4] = { p[0], p[1], p[3], p[2] };  // 0 1 3 2
	drawPoly(img, points1, 4, CV_RGB(100, 100, 100));
	Point2f points2[4] = { p[0], p[1], p[5], p[4] };  // 0 1 5 4
	drawPoly(img, points2, 4, CV_RGB(100, 100, 100));
	Point2f points3[4] = { p[4], p[5], p[7], p[6] };  // 4 5 7 6
	drawPoly(img, points3, 4, CV_RGB(100, 100, 100));
	Point2f points4[4] = { p[2], p[3], p[7], p[6] };  // 2 3 7 6
	drawPoly(img, points4, 4, CV_RGB(100, 100, 100));
	Point2f points5[4] = { p[0], p[2], p[6], p[4] };  // 0 2 6 4
	drawPoly(img, points5, 4, CV_RGB(100, 100, 100));
	Point2f points6[4] = { p[1], p[3], p[7], p[5] };  // 1 3 7 5
	drawPoly(img, points6, 4, CV_RGB(100, 100, 100));
}

//draw shapes on top of the image
void drawOverlay(Mat img, Mat rotation, Mat translation, Mat cameraMatrix, Mat distortion){
	vector<Point3f> points3d;
	vector<Point2f> points2d;

	//fill board with polygon
	
	points3d.push_back(Point3f(-1, -1, 0));
	points3d.push_back(Point3f(-1, 9, 0));
	points3d.push_back(Point3f(6, 9, 0));
	points3d.push_back(Point3f(6, -1, 0));

	projectPoints(points3d, rotation, translation, cameraMatrix, distortion, points2d);
	drawPoly(img, points2d, CV_RGB(255, 255, 255));

	//draw cube
	points3d.clear();
	points2d.clear();
	
	points3d.push_back(Point3f(0, 0, 0));
	points3d.push_back(Point3f(1, 0, 0));
	points3d.push_back(Point3f(0, 1, 0));
	points3d.push_back(Point3f(1, 1, 0));
	points3d.push_back(Point3f(0, 0, -1));
	points3d.push_back(Point3f(1, 0, -1));
	points3d.push_back(Point3f(0, 1, -1));
	points3d.push_back(Point3f(1, 1, -1));

	projectPoints(points3d, rotation, translation, cameraMatrix, distortion, points2d);

	drawBox(img, points2d, CV_RGB(0, 0, 0));

	points3d.clear();
	points2d.clear();

	points3d.push_back(Point3f(0, 0, 0)); // middle point
	points3d.push_back(Point3f(3, 0, 0)); // x axis
	points3d.push_back(Point3f(0, 3, 0)); // y axis
	points3d.push_back(Point3f(0, 0, -3));// z axis
	
	//project the points that represent the 3d axis vectors to 2d
	projectPoints(points3d, rotation, translation, cameraMatrix, distortion, points2d);

	//draw axis lines
	arrowedLine(img, points2d[0], points2d[1], CV_RGB(255, 0, 0), 3); //x
	arrowedLine(img, points2d[0], points2d[2], CV_RGB(0, 255, 0), 3); //y
	arrowedLine(img, points2d[0], points2d[3], CV_RGB(0, 0, 255), 3); //z
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

