#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#include <utility>

using namespace cv;
using namespace std;

double ComputeAngle(const Point& a, const Point& b)
{
	double val = (a.dot(b) / (norm(Mat(a)) * norm(Mat(b))));

	if (val >= 1.0)
		return 0.0;
	else if (val <= -1.0)
		return 180;
	else
		return (180 / M_PI) * acos(val);
}

pair<vector<Point>, vector<Point>> CurvedPoints(const vector<Point>& Contour, int Diff, double Theta)
{
	pair<vector<Point>, vector<Point>> Curves;
	for (int i = Diff; i < Contour.size() - Diff; i++) {
		Point a = Contour[i] - Contour[i + Diff];
		Point b = Contour[i] - Contour[i - Diff];
		Point c = Contour[i] - Contour[i - Diff];
		Point d = Contour[i + Diff] - Contour[i];
		double Angle = ComputeAngle(a, b);
		if (Angle < Theta) {
			Point3d c3d(c);
			Point3d d3d(d);
			Point3d Cross = c3d.cross(d3d);
			if (Cross.z >= 0){
				Curves.first.push_back(Contour[i]);
			}
			else{
				Curves.second.push_back(Contour[i]);
			}
		}
	}
	return Curves;
}

vector<vector<Point>> GetContours(const Mat& Frame)
{
	vector<vector<Point>> Contours;
	vector<vector<Point>> Desired;
	vector<Vec4i> Hierarchy;
	Mat CannyOutput;

	Canny(Frame, CannyOutput, 100, 255);
	try {
		findContours(CannyOutput, Contours, Hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	} catch (Exception& e){
		cout << e.msg << endl;
	}

	for (int i = 0; i < Contours.size(); ++i){
		if (Contours[i].size() > 100 && Hierarchy[i][3] == -1) {
			Desired.push_back(Contours[i]);
			//drawContours(ContourImg, Contours, i, Color, 1, 8, Hierarchy, 0, Point());
		}
	}

	return Desired;
}

Mat SkinSegment(const Mat& Frame)
{
	Scalar LowerBound(0, 133, 77);
	Scalar UpperBound(255, 173, 127);

	Mat BlurredFrame;
	GaussianBlur(Frame, BlurredFrame, Size(3, 3), 100);

	Mat Converted;
	cvtColor(BlurredFrame, Converted, CV_BGR2YCrCb);

	Mat Thresh;
	inRange(Converted, LowerBound, UpperBound, Thresh);

	int KernelSize = 2;
	Mat ErosionKernel = getStructuringElement(MORPH_ELLIPSE,
		Size(2 * KernelSize + 1, 2 * KernelSize + 1),
		Point(KernelSize, KernelSize));

	Mat Eroded;
	erode(Thresh, Eroded, ErosionKernel);

	KernelSize = 4;
	Mat DilationKernel = getStructuringElement(MORPH_ELLIPSE,
		Size(2 * KernelSize + 1, 2 * KernelSize + 1),
		Point(KernelSize, KernelSize));

	Mat Dilated;
	dilate(Eroded, Dilated, DilationKernel);

	medianBlur(Dilated, BlurredFrame, 9);
	
	return BlurredFrame;
}


vector<Point> Cluster(const vector<Point>& Points, double DistThreshold)
{
	vector<Point> Centroids;

	if (Points.size() == 0) return Centroids;

	Point Sum = Points[0];
	int Count = 1;
	for (int i = 0; i < Points.size() - 1; i++) {
		double dist = norm(Points[i + 1] - Points[i]);
		if (dist < DistThreshold) {
			Sum += Points[i + 1];
			Count++;
		} else {
			if (Count > 0) {
				Point Cluster;
				Cluster.x = (double) Sum.x / Count;
				Cluster.y = (double) Sum.y / Count;

				Centroids.push_back(Cluster);
			}
			
			Sum = Points[i + 1];
			Count = 1;
		}
	}

	return Centroids;
}


int main(int argc, char** argv)
{
	VideoCapture VideoStream(CV_CAP_ANY);

	if (!VideoStream.isOpened()) {
		cout << "Error: Unable to open webcam.";
		return 1;
	}

	while (true) {
		Mat Frame;
		VideoStream.read(Frame);
		if (Frame.empty())
			continue;

		Mat Threshold = SkinSegment(Frame);
		vector<vector<Point>> Contours = GetContours(Threshold);
		vector<vector<Point>> CurvesUp;
		vector<vector<Point>> CurvesDown;
		vector<vector<Point>> ClustersUp;
		vector<vector<Point>> ClustersDown;
		for (auto& Contour : Contours) {
			auto Pts = CurvedPoints(Contour, 15, 60);
			CurvesUp.push_back(Pts.second);
			CurvesDown.push_back(Pts.first);
			ClustersUp.push_back(Cluster(Pts.second, 10));
			ClustersDown.push_back(Cluster(Pts.first, 10));
		}

		for (auto& Component : CurvesUp) {
			for (auto& Point : Component) {
				circle(Frame, cvPoint(Point.x, Point.y), 2, CV_RGB(255, 0, 0), -1, 8, 0);
			}
		}
		
		for (auto& Component : CurvesDown) {
			for (auto& Point : Component) {
				circle(Frame, cvPoint(Point.x, Point.y), 2, CV_RGB(0, 0, 255), -1, 8, 0);
			}
		}
		/*
		for (auto& Component : ClustersUp) {
			for (auto& Point : Component) {
				circle(Frame, cvPoint(Point.x, Point.y), 5, CV_RGB(0, 0, 0), -1, 8, 0);
			}
		}
		*/
		for (auto& Component : ClustersDown) {
			for (auto& Point : Component) {
				circle(Frame, cvPoint(Point.x, Point.y), 5, CV_RGB(0, 0, 0), -1, 8, 0);
			}
		}
		
		/*
		Mat ContourImg;
		for (int i = 0; i < Contours.size(); i++)
			drawContours(Frame, Contours, i, Scalar(255,255,255), 1, 8);
		*/


		vector<vector<Point>> Polys;
		Polys.resize(Contours.size());
		for (int i = 0; i < Contours.size(); i++) {
			approxPolyDP(Contours[i], Polys[i], 1, true);
		}

		vector<vector<Point>> Hulls;
		Hulls.resize(Polys.size());
		for (int i = 0; i < Polys.size(); i++) {
			convexHull(Polys[i], Hulls[i]);
		}

		//for (int i = 0; i < Contours.size(); i++)
		//	drawContours(Frame, Hulls, i, Scalar(255, 255, 255), 1, 8);
		/*
		for (auto& Hull : Hulls) {
			for (auto& Point : Hull) {
				circle(Frame, cvPoint(Point.x, Point.y), 5, CV_RGB(0, 255, 0), -1, 8, 0);
			}
		}
		*/

		//imshow("Webcam", Threshold);
		imshow("Webcam", Frame);

		int key = cvWaitKey(10);
		if (char(key) == 27)
			break;      // Break if ESC is pressed
	}

	return 0;
}