#include "Hand.h"



Hand::Hand(vector<Point>& Clusters)
{
	Update(Clusters);
}


void Hand::Update(const vector<Point>& Clusters)
{
	cout << Clusters.size() << endl;
	if (Clusters.size() != 5) return;

	Centroid = computeCentroid(Clusters);

	vector<pair<const Point*, double>> Distances;

	for (int i = 0; i < Clusters.size(); i++) {
		pair<const Point*, double> Dist;
		Dist.first = &Clusters[i];
		Dist.second = norm(Clusters[i] - Centroid);
		Distances.push_back(Dist);
	}

	auto SortPred = [](pair<const Point*, double>& a, pair<const Point*, double>& b) { return a.second > b.second; };
	sort(Distances.begin(), Distances.end(), SortPred);

	Thumb = *Distances[0].first;
	for (int i = 0; i < 4; i++) {
		Fingers[i] = *Distances[i + 1].first;
	}

	FingerAxis = computeFingerAxis();
	ThumbAxis = computeThumbAxis();
}


Point Hand::computeCentroid(const vector<Point>& Clusters)
{
	Point Sum;
	for (int i = 0; i < Clusters.size(); i++)
		Sum += Clusters[i];

	return Sum * (1 / static_cast<double>(Clusters.size()));
}


Point Hand::computeThumbAxis(void)
{
	Point Axis;
	Axis.x = -FingerAxis.y;
	Axis.y = FingerAxis.x;
	return Axis;
}

Point Hand::computeFingerAxis()
{
	Point Sum;
	for (int i = 0; i < 4; i++)
		Sum += Fingers[i];

	return (Sum * .25) - Centroid;
}

Point Hand::getFingerAxis()
{
	return FingerAxis;
}

Point Hand::getThumbAxis()
{
	return ThumbAxis;
}

Point Hand::getCentroid()
{
	return Centroid;
}

Point Hand::getThumb(void)
{
	return Thumb;
}