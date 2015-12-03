#include "HandSeg.h"
#include "Hand.h"

struct intercept{
	int x;
	int y;
	double distance;
};

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

		/*
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
		
		for (auto& Component : ClustersUp) {
			for (auto& Point : Component) {
				circle(Frame, cvPoint(Point.x, Point.y), 5, CV_RGB(0, 0, 0), -1, 8, 0);
			}
		}
		
		for (auto& Component : ClustersDown) {
			for (auto& Point : Component) {
				circle(Frame, cvPoint(Point.x, Point.y), 5, CV_RGB(0, 0, 0), -1, 8, 0);
			}
		}
		*/
		
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

		for (int i = 0; i < Contours.size(); i++)
			drawContours(Frame, Hulls, i, Scalar(255, 255, 255), 1, 8);

		Point Sum;
		if (Hulls.size()) {
			for (int i = 0; i < Hulls[0].size(); i++) {
				Sum += Hulls[0][i];
			}
			Sum = Sum * (1 / static_cast<double>(Hulls[0].size()));
		}



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

		Hand h(FingerPoints);
		Point Thumb2d = h.getThumb();
		Point Centroid = h.getCentroid();

		Point Closest;
		double closestDist = std::numeric_limits<double>::max();
		for (int i = 0; i < FingerPoints.size(); i++) {
			if (norm(Centroid - FingerPoints[i]) < closestDist) {
				Closest = FingerPoints[i];
				closestDist = norm(Centroid - FingerPoints[i]);
			}
		}

		// (py – qy)x + (qx – px)y + (pxqy – qxpy) = 0

		//drawStraightLine(&Frame, Centroid, Closest, CV_RGB(100, 200, 50));

		Point3d line;
        
        int line_y = Centroid.y- Closest.y;
        int line_x = Centroid.x - Closest.x;

        
		line.x = Centroid.y - Closest.y;
		line.y = Closest.x - Centroid.x;
		line.z = Centroid.x * Closest.y - Closest.x * Centroid.y;

		Point3d Thumb(Thumb2d.x, Thumb2d.y, 0);

		//k = ((y2 - y1) * (x3 - x1) - (x2 - x1) * (y3 - y1)) / ((y2 - y1) ^ 2 + (x2 - x1) ^ 2)
		//	x4 = x3 - k * (y2 - y1)
		//	y4 = y3 + k * (x2 - x1)

		/*
		double k = ((Centroid.y - Closest.y) * (Thumb.x - Closest.x) - (Centroid.x - Closest.x) * (Thumb.y - Closest.y)) /
			(pow(norm(Centroid.y - Closest.y), 2) + pow(norm(Centroid.x - Closest.x), 2));
		double x_pos = Thumb.x - k * (Centroid.y - Closest.y);
		double y_pos = Thumb.y - k * (Centroid.x - Closest.x);
		Point Center((int)x_pos, (int)y_pos);
		cout << Center << endl;
		*/

		Point3d line2;
        int line2_x = Centroid.x- Thumb.x;
        int line2_y = Centroid.y - Thumb.y;

        
		line2.x = Centroid.y - Thumb.y;
		line2.y = Thumb.x - Centroid.x;
		line2.z = Centroid.x*Thumb.y - Thumb.x*Centroid.y;


		//Point3d proj = line2.dot(Line) / pow(norm(line2), 2) * Line;
        
        int scalar = (line_x * line2_x + line_y *line2_y)/pow(norm(line),2);
        
        Point center;
        center.x = scalar*line_x+ Centroid.x;
        center.y = scalar*line_y+ Centroid.y;
        
        //circle(Frame, cvPoint(Closest.x, Closest.y), 5, CV_RGB(100, 200, 50), -1, 8, 0);
        circle(Frame, cvPoint(center.x, center.y), 5, CV_RGB(255,0,0), -1, 8, 0);
        

		//circle(Frame, cvPoint(Closest.x, Closest.y), 5, CV_RGB(100, 200, 50), -1, 8, 0);

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


		
		
		
		//circle(Frame, cvPoint(proj.x, proj.y), 5, CV_RGB(255, 0, 0), -1, 8, 0);
		circle(Frame, cvPoint(Thumb2d.x, Thumb2d.y), 5, CV_RGB(0, 0, 255), -1, 8, 0);
		//circle(Frame, cvPoint(Sum.x, Sum.y), 5, CV_RGB(0, 255, 0), -1, 8, 0);
		//line(Frame, Center, Closest, CV_RGB(255, 0, 0));
		//line(Frame, Center, h.getThumb(), CV_RGB(255, 0, 0));

		//imshow("Webcam", Threshold);
		imshow("Webcam2", Frame);

		int key = cvWaitKey(10);
		if (char(key) == 27)
			break;      // Break if ESC is pressed
	}

	return 0;
}