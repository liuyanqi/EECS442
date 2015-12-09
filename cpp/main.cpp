#include "HandSeg.h"
#include "Hand.h"
#include "PoseEstimator.h"
#include "opencv2/calib3d/calib3d.hpp"

void draw(Mat img, Point2f center, vector<Point2d> imgpts){
    line(img, center, imgpts[1], Scalar(255,0,0),5);
    line(img, center, imgpts[2], Scalar(0,255,0),5);
    line(img, center, imgpts[3], Scalar(0,0,255),5);
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
			auto Pts = CurvedPoints(Contour, 15, 30);
			CurvesUp.push_back(Pts.second);
			CurvesDown.push_back(Pts.first);
			ClustersUp.push_back(Cluster(Pts.second, 20));
			ClustersDown.push_back(Cluster(Pts.first, 20));
		}

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

		for (int i = 0; i < Contours.size(); i++)
			drawContours(Frame, Hulls, i, Scalar(255, 255, 255), 1, 8);

		Point Sum;
		if (Hulls.size()) {
			for (int i = 0; i < Hulls[0].size(); i++) {
				Sum += Hulls[0][i];
			}
			Sum = Sum * (1 / static_cast<double>(Hulls[0].size()));
		}

		vector<Point> FingerPoints;

		Point Closest;
		vector<pair<Point, double>> Distances;
		double minDist = numeric_limits<double>::max();
		for (auto& Component : ClustersUp) {
			for (auto& Point : Component) {
				//circle(Frame, cvPoint(Point.x, Point.y), 5, CV_RGB(255, 0, 0), -1, 8, 0);
				for (auto& Hull : Hulls) {
					for (auto& HullPoint : Hull) {
						double Distance = pow((Point.x - HullPoint.x), 2) + pow((Point.y - HullPoint.y), 2);
						if (Distance < minDist){
							minDist = Distance;
							Closest = Point;
						}
					}
				}
				Distances.push_back(make_pair(Closest, minDist));
				minDist = numeric_limits<double>::max();
			}
		}

		auto ComparePoints = [](pair<Point, double>& a, pair<Point, double>& b) { return a.second < b.second; };
		sort(Distances.begin(), Distances.end(), ComparePoints);

		for (int i = 0; i < Distances.size(); i++){
			if (FingerPoints.size() == 5 || Distances[i].second > 2000) break;
			if (Distances[i].first.y > 450 || Distances[i].first.x < 20 || Distances[i].first.x > 620) continue;
			FingerPoints.push_back(Distances[i].first);
		}

		Hand h(FingerPoints);
		Point Thumb = h.getThumb();

		vector<pair<Point, double>> OrderedFingers;
		for (auto& Finger : FingerPoints) {
			OrderedFingers.push_back(make_pair(Finger, norm(Finger - Thumb)));
		}

		sort(OrderedFingers.begin(), OrderedFingers.end(), ComparePoints);

		for (int i = 0; i < FingerPoints.size(); i++) {
			FingerPoints[i] = OrderedFingers[i].first;
		}

		vector<Point3d> object;
		object.push_back(Point3d(0, 0, 0));
		object.push_back(Point3d(50, 0, 0));
		object.push_back(Point3d(0, 50, 0));
		object.push_back(Point3d(0, 0, 50));

		PoseEstimator Pose;
		Pose.UpdateProjectionMatrix(FingerPoints);
		vector<Point2d> ImageCoordinates = Pose.GetImageCoordinates(object);

		if (FingerPoints.size() == 5) {
			circle(Frame, cvPoint(FingerPoints[0].x, FingerPoints[0].y), 5, CV_RGB(0, 128, 0), -1, 8, 0);
			circle(Frame, cvPoint(FingerPoints[1].x, FingerPoints[1].y), 5, CV_RGB(0, 255, 0), -1, 8, 0);
			circle(Frame, cvPoint(FingerPoints[2].x, FingerPoints[2].y), 5, CV_RGB(0, 0, 128), -1, 8, 0);
			circle(Frame, cvPoint(FingerPoints[3].x, FingerPoints[3].y), 5, CV_RGB(0, 0, 255), -1, 8, 0);
			circle(Frame, cvPoint(FingerPoints[4].x, FingerPoints[4].y), 5, CV_RGB(255, 0, 0), -1, 8, 0);
		}
        
		if (!ImageCoordinates.empty()) {
			 draw(Frame, ImageCoordinates[0], ImageCoordinates);
		}
        
		//circle(Frame, cvPoint(Thumb.x, Thumb.y), 5, CV_RGB(0, 0, 255), -1, 8, 0);

		imshow("thresh", Threshold);
		imshow("Webcam", Frame);

		int key = cvWaitKey(10);
		if (char(key) == 27)
			break;      // Break if ESC is pressed
	}

	return 0;
}





/*

static bool init;
static vector<Point> fingertip_prev;
if(init == false){
int key = cvWaitKey(10);
if(char(key) == 105){
sort(FingerPoints.begin(),FingerPoints.end(),compare_x);
cout<<"initialized"<<endl;
init = true;
}

}

//track figner tip
if(init){
if(FingerPoints.size() ==5 && fingertip_prev.size() ==5){
bool matched[5] = {0};
for(int i =0; i <FingerPoints.size(); i++){
float minDist = 50;
int minJ = -1;
for(int j =0; j<fingertip_prev.size(); j++){
float dist = sqrt(pow((float)FingerPoints[i].x - fingertip_prev[i].x,2)+
pow((float)FingerPoints[i].y - fingertip_prev[i].y,2));
if(!matched[j] && dist< minDist){
minDist = dist;
minJ = j;
}
}
if(minJ >=0){
//if exist a match, update
matched[minJ] = true;
drawStraightLine(&Frame, FingerPoints[i], fingertip_prev[minJ], CV_RGB(255, 0, 0));
}
}
}


//current frame = previous frame
for(int i =0; i< FingerPoints.size(); i++){
fingertip_prev.push_back(FingerPoints[i]);
}
}


*/

/*
int min_dis = 255;
int point_x = 0;
int point_y = 0;
vector<intercept> all_dist;

for (auto& Component : ClustersUp) {
for (auto& Point : Component) {

for (auto& Hull : Hulls) {
for (auto& Point_Hull : Hull) {
double dist = pow((Point.x - Point_Hull.x), 2) + pow((Point.y - Point_Hull.y), 2);
if (dist < min_dis){
min_dis = dist;
point_x = Point.x;
point_y = Point.y;
}

}
}
intercept new_inter;
new_inter.x = point_x;
new_inter.y = point_y;
new_inter.distance = min_dis;
all_dist.push_back(new_inter);
min_dis = 255;
}
}
sort(all_dist.begin(), all_dist.end(), myobject);

vector<Point> FingerPoints;

if (!all_dist.empty()){
for (int i = 0; i< all_dist.size(); i++){
if (FingerPoints.size() == 5) break;

if (all_dist[i].y > 450 || all_dist[i].x <20 || all_dist[i].x > 620) continue;
FingerPoints.push_back(Point(all_dist[i].x, all_dist[i].y));
circle(Frame, cvPoint(all_dist[i].x, all_dist[i].y), 5, CV_RGB(0, 0, 0), -1, 8, 0);
}
}

*/

/*

struct intercept{
int x;
int y;
double distance;
};

struct compare_x{
bool operator() (Point i, Point j) { return (i.x > j.x); }
} compare_x;

struct myclass {
bool operator() (intercept i, intercept j) { return (i.distance<j.distance); }
} myobject;

void drawStraightLine(cv::Mat *img, cv::Point2f p1, cv::Point2f p2, cv::Scalar color)
{
Point2f p, q;
// Check if the line is a vertical line because vertical lines don't have slope
if (p1.x != p2.x)
{
p.x = 0;
q.x = img->cols;
// Slope equation (y1 - y2) / (x1 - x2)
float m = (p1.y - p2.y) / (p1.x - p2.x);
// Line equation:  y = mx + b
float b = p1.y - (m * p1.x);
p.y = m * p.x + b;
q.y = m * q.x + b;
}
else
{
p.x = q.x = p2.x;
p.y = 0;
q.y = img->rows;
}

cv::line(*img, p, q, color, 1);
}

*/