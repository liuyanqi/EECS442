#ifndef HAND_H
#define HAND_H

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

class Hand
{
public:
	Hand(void);
	Hand(vector<Point>& Clusters);

	void Update(const vector<Point>& Clusters);
	Point getCentroid(void);
	Point getThumbAxis(void);
	Point getFingerAxis(void);
	Point getThumb(void);

private:
	Point computeCentroid(const vector<Point>& Clusters);
	Point computeFingerAxis(void);
	Point computeThumbAxis(void);

	Point Fingers[4];
	Point Thumb;
	Point Centroid;
	Point FingerAxis;
	Point ThumbAxis;
};


#endif