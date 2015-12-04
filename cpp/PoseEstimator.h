#ifndef POSE_ESTIMATOR_H
#define POSE_ESTIMATOR_H

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv/cv.h>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#include <utility>

using namespace cv;
using namespace std;

class PoseEstimator
{
public:
	PoseEstimator(void);

	void UpdateProjectionMatrix(const vector<Point>& FingerTips);
	vector<Point2d> GetImageCoordinates(const vector<Point3d>& ModelCoordinates) const;

private:
	Mat cameraMat;
	Mat rotationMat;
	Mat translationMat;
	Mat distCoeffs;

	vector<Point3f> handModel;
};


#endif