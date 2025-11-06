/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <cmath>
#include <vector>
#include <algorithm>

#include "VisvalingamWhyatt.h"

VisvalingamWhyatt::VisvalingamWhyatt()
	: fMinTriangleArea(0.001)
	, fMinPointCount(3)
{
}

VisvalingamWhyatt::~VisvalingamWhyatt()
{
}

std::vector<std::vector<double> >
VisvalingamWhyatt::SimplifyPath(const std::vector<std::vector<double> >& path, double tolerance)
{
	return SimplifyPath(path, tolerance, NULL);
}

std::vector<std::vector<double> >
VisvalingamWhyatt::SimplifyPath(const std::vector<std::vector<double> >& path,
								double tolerance, const std::vector<bool>* protectedPoints)
{
	if (!_IsValidPath(path) || path.size() <= fMinPointCount)
		return path;

	bool isClosedPath = false;
	int pointCount = path.size();

	if (pointCount > 3) {
		double dx = path[0][0] - path[pointCount - 1][0];
		double dy = path[0][1] - path[pointCount - 1][1];
		double distance = sqrt(dx * dx + dy * dy);
		isClosedPath = (distance < 2.0);
	}

	if (pointCount <= fMinPointCount)
		return path;

	std::vector<VWPoint> points;
	points.reserve(pointCount);

	int componentCount = path.empty() ? 0 : path[0].size();

	for (int i = 0; i < pointCount; i++) {
		points.push_back(VWPoint(path[i][0], path[i][1]));

		if (isClosedPath && i == 0) {
			points[i].area = 1e30;
		} else if (protectedPoints && i < (int)protectedPoints->size() && (*protectedPoints)[i]) {
			points[i].area = 1e30;
		}
	}

	double thresholdArea = tolerance * tolerance;

	for (int i = 0; i < static_cast<int>(points.size()); i++) {
		if (!points[i].removed && points[i].area < 1e29) {
			int prev = (i - 1 + points.size()) % points.size();
			int next = (i + 1) % points.size();

			while (points[prev].removed && prev != i) {
				prev = (prev - 1 + points.size()) % points.size();
			}
			while (points[next].removed && next != i) {
				next = (next + 1) % points.size();
			}

			if (prev != i && next != i && prev != next) {
				points[i].area = _CalculateTriangleArea(
					points[prev].x, points[prev].y,
					points[i].x, points[i].y,
					points[next].x, points[next].y
				);
			} else {
				points[i].area = 1e30;
			}
		}
	}

	bool foundPointToRemove = true;
	int remainingPoints = points.size();
	int minRequired = fMinPointCount;

	if (isClosedPath && minRequired < 4)
		minRequired = 4;

	while (foundPointToRemove && remainingPoints > minRequired) {
		foundPointToRemove = false;
		int minIndex = -1;
		double minArea = thresholdArea;

		for (int i = 0; i < static_cast<int>(points.size()); i++) {
			if (!points[i].removed && points[i].area < minArea) {
				minArea = points[i].area;
				minIndex = i;
				foundPointToRemove = true;
			}
		}

		if (foundPointToRemove && minIndex != -1) {
			points[minIndex].removed = true;
			remainingPoints--;

			for (int offset = -2; offset <= 2; offset++) {
				int idx = (minIndex + offset + points.size()) % points.size();
				if (!points[idx].removed && points[idx].area < 1e29) {
					int prev = idx;
					int next = idx;

					do {
						prev = (prev - 1 + points.size()) % points.size();
					} while (points[prev].removed && prev != idx);

					do {
						next = (next + 1) % points.size();
					} while (points[next].removed && next != idx);

					if (prev != idx && next != idx && prev != next) {
						points[idx].area = _CalculateTriangleArea(
							points[prev].x, points[prev].y,
							points[idx].x, points[idx].y,
							points[next].x, points[next].y
						);
					} else {
						points[idx].area = 1e30;
					}
				}
			}
		}
	}

	std::vector<std::vector<double> > result;
	for (int i = 0; i < static_cast<int>(points.size()); i++) {
		if (!points[i].removed) {
			std::vector<double> point(componentCount > 0 ? componentCount : 2);
			point[0] = points[i].x;
			point[1] = points[i].y;

			if (componentCount > 2) {
				for (int c = 2; c < componentCount; c++) {
					if (i < pointCount && c < (int)path[i].size()) {
						point[c] = path[i][c];
					} else {
						point[c] = 0.0;
					}
				}
			}

			result.push_back(point);
		}
	}

	return result;
}

std::vector<std::vector<std::vector<double> > >
VisvalingamWhyatt::BatchSimplifyInternodes(const std::vector<std::vector<std::vector<double> > >& internodes, double tolerance)
{
	std::vector<std::vector<std::vector<double> > > result;
	result.reserve(internodes.size());

	for (int i = 0; i < static_cast<int>(internodes.size()); i++) {
		result.push_back(SimplifyPath(internodes[i], tolerance));
	}

	return result;
}

std::vector<std::vector<std::vector<std::vector<double> > > >
VisvalingamWhyatt::BatchSimplifyLayerInternodes(const std::vector<std::vector<std::vector<std::vector<double> > > >& layerInternodes, double tolerance)
{
	std::vector<std::vector<std::vector<std::vector<double> > > > result;
	result.reserve(layerInternodes.size());

	for (int i = 0; i < static_cast<int>(layerInternodes.size()); i++) {
		result.push_back(BatchSimplifyInternodes(layerInternodes[i], tolerance));
	}

	return result;
}

double
VisvalingamWhyatt::_CalculateTriangleArea(double x1, double y1, double x2, double y2, double x3, double y3)
{
	double dx1 = x2 - x1;
	double dy1 = y2 - y1;
	double dx2 = x3 - x1;
	double dy2 = y3 - y1;

	double crossProduct = dx1 * dy2 - dx2 * dy1;
	return fabs(crossProduct) * 0.5;
}

bool
VisvalingamWhyatt::_IsValidPath(const std::vector<std::vector<double> >& path)
{
	if (path.empty())
		return false;

	for (int i = 0; i < static_cast<int>(path.size()); i++) {
		if (path[i].size() < 2) return false;
	}

	return true;
}

std::vector<std::vector<double> >
VisvalingamWhyatt::_SimplifyClosedPath(const std::vector<std::vector<double> >& path, double tolerance)
{
	return SimplifyPath(path, tolerance, NULL);
}

std::vector<std::vector<double> >
VisvalingamWhyatt::_SimplifyOpenPath(const std::vector<std::vector<double> >& path, double tolerance)
{
	return SimplifyPath(path, tolerance, NULL);
}
