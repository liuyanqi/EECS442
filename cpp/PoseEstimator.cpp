#include "PoseEstimator.h"

PoseEstimator::PoseEstimator()
{
	/* Define the camera matrix */
	cameraMat = Mat::zeros(3, 3, CV_64FC1);
	cameraMat.at<double>(0, 0) = 6.5746697810243404e+002;
	cameraMat.at<double>(1, 0) = 0;
	cameraMat.at<double>(2, 0) = 0;
	cameraMat.at<double>(0, 1) = 0;
	cameraMat.at<double>(1, 1) = 1.7067753507885654e+003;
	cameraMat.at<double>(2, 1) = 0;
	cameraMat.at<double>(0, 2) = 3.1950000000000000e+002;
	cameraMat.at<double>(1, 2) = 2.3950000000000000e+002;
	cameraMat.at<double>(2, 2) = 1;

	/* Define the distortion vector */
	distCoeffs = Mat(5, 1, cv::DataType<double>::type);
	distCoeffs.at<double>(0) = -4.1802327018241026e-001;
	distCoeffs.at<double>(1) = 5.0715243805833121e-001;
	distCoeffs.at<double>(2) = 0;
	distCoeffs.at<double>(3) = 0;
	distCoeffs.at<double>(4) = -5.7843596847939704e-001;

	/* Initialize rotation matrix */
	rotationMat = Mat::zeros(3, 3, CV_64FC1);

	/* Initialize translation matrix */
	translationMat = Mat::zeros(3, 1, CV_64FC1); 

	/* Define the hand model */
	handModel.push_back(Point3f(95, 0, 5));
	handModel.push_back(Point3f(53, 90, 5));
	handModel.push_back(Point3f(0, 105, 5));
	handModel.push_back(Point3f(-49, 95, 5));
	handModel.push_back(Point3f(-89, 57, 5));
}

void PoseEstimator::UpdateProjectionMatrix(const vector<Point>& FingerTips)
{
	if (FingerTips.size() != 5)
		return;

	vector<Point2f> FingerTipsFloat;
	for (auto& Tip : FingerTips)
		FingerTipsFloat.push_back(Tip);

	solvePnP(handModel, FingerTipsFloat, cameraMat, distCoeffs, rotationMat, translationMat, false);
}


vector<Point2d> PoseEstimator::GetImageCoordinates(const vector<Point3d>& ModelCoordinates) const
{
	vector<Point2d> ImageCoordinates;
	projectPoints(ModelCoordinates, rotationMat, translationMat, cameraMat, distCoeffs, ImageCoordinates);
	return ImageCoordinates;
}