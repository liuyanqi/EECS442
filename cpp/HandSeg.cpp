#include "HandSeg.h"

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
		/*
		int pp1_x = Contour[i - Diff].x - Contour[i].x;
		int pp1_y = Contour[i - Diff].y - Contour[i].y;
		int pp2_x = Contour[i + Diff].x - Contour[i].x;
		int pp2_y = Contour[i + Diff].y - Contour[i].y;
		*/
		int pp1_x = Contour[i - Diff].x - Contour[i].x;
		int pp1_y = Contour[i - Diff].y - Contour[i].y;
		int pp2_x = Contour[i + Diff].x - Contour[i].x;
		int pp2_y = Contour[i + Diff].y - Contour[i].y;


		double Angle = (pp1_x * pp2_x + pp1_y * pp2_y)
			/ (sqrt((float)pp1_x * pp1_x + pp1_y * pp1_y) * sqrt((float)pp2_x * pp2_x + pp2_y * pp2_y));

		if (Angle > (M_PI / 180) * Theta) {
			int dir = pp1_x * pp2_y - pp1_y * pp2_x;
			if (Angle < M_PI && dir > 0){
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

	KernelSize = 2;
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
			if (Count > 5) {
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
