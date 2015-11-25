#ifndef HANDSEG_H
#define HANDSEG_H

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

double ComputeAngle(const Point& a, const Point& b);

pair<vector<Point>, vector<Point>> CurvedPoints(const vector<Point>& Contour, int Diff, double Theta);

vector<vector<Point>> GetContours(const Mat& Frame);

Mat SkinSegment(const Mat& Frame);

vector<Point> Cluster(const vector<Point>& Points, double DistThreshold);

#endif