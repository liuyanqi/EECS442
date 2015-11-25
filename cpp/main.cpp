#include "HandSeg.h"

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