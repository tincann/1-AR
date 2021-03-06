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


//list of cube vertices
static vector<Point3f> cubePoints;

//cube position and speed
static Point3f cubePos, cubeSpeed;

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

void drawCubeWireframe(Mat img, Point3f loc, Scalar color, Mat rvec, Mat tvec, Mat cameraMatrix, Mat distCoeffs){
	
	//add location to every cube point
	vector<Point3f> points;
	for (int i = 0; i < cubePoints.size(); i++){
		points.push_back(cubePoints[i] + loc);
	}

	vector<Point2f> p;
	projectPoints(points, rvec, tvec, cameraMatrix, distCoeffs, p);
	
	line(img, p[0], p[1], color);
	line(img, p[1], p[3], color);
	line(img, p[3], p[2], color);
	line(img, p[2], p[0], color);
	line(img, p[3], p[7], color);
	line(img, p[7], p[5], color);
	line(img, p[5], p[1], color);
	line(img, p[6], p[4], color);
	line(img, p[2], p[3], color);
	line(img, p[7], p[6], color);
	line(img, p[6], p[2], color);
	line(img, p[5], p[4], color);
	line(img, p[4], p[0], color);
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

	points3d.clear();
	points2d.clear();

	//calculate cube position
	vector<double> r;
	r.assign((double*)rotation.datastart, (double*)rotation.dataend);
	cubeSpeed.x = r[0]; //x
	cubeSpeed.y = r[1]; //x
	cubePos += cubeSpeed;

	if (cubePos.x < -1 || cubePos.x > 6){
		cubePos.x = 3;
	}
	if (cubePos.y < -1 || cubePos.y > 9){
		cubePos.y = 4;
	}

	cout << cubePos << endl;

	//draw cube
	drawCubeWireframe(img, cubePos, CV_RGB(255, 0, 0), rotation, translation, cameraMatrix, distortion);
}

void Init(){
	cubePos = Point3f(4, 3, 0);
	cubeSpeed = Point3f(0, 0, 0);

	//draw cube
	cubePoints.push_back(Point3f(0, 0, 0));
	cubePoints.push_back(Point3f(1, 0, 0));
	cubePoints.push_back(Point3f(0, 1, 0));
	cubePoints.push_back(Point3f(1, 1, 0));
	cubePoints.push_back(Point3f(0, 0, -1));
	cubePoints.push_back(Point3f(1, 0, -1));
	cubePoints.push_back(Point3f(0, 1, -1));
	cubePoints.push_back(Point3f(1, 1, -1));
}

int _tmain(int argc, char* argv[])
{
	Init();
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

