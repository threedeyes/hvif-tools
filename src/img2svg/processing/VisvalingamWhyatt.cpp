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

std::vector<std::vector<double>>
VisvalingamWhyatt::SimplifyPath(const std::vector<std::vector<double>>& path, double tolerance)
{
	if (!_IsValidPath(path) || path.size() <= fMinPointCount) {
		return path;
	}

	// Check if path is closed (first and last points are close)
	bool isClosedPath = false;
	if (path.size() > 3) {
		double dx = path[0][0] - path[path.size() - 1][0];
		double dy = path[0][1] - path[path.size() - 1][1];
		double distance = sqrt(dx * dx + dy * dy);
		isClosedPath = (distance < 2.0); // Close enough to be considered closed
	}

	if (isClosedPath) {
		return _SimplifyClosedPath(path, tolerance);
	} else {
		return _SimplifyOpenPath(path, tolerance);
	}
}

std::vector<std::vector<std::vector<double>>>
VisvalingamWhyatt::BatchSimplifyInternodes(const std::vector<std::vector<std::vector<double>>>& internodes, double tolerance)
{
	std::vector<std::vector<std::vector<double>>> result;
	result.reserve(internodes.size());

	for (int i = 0; i < static_cast<int>(internodes.size()); i++) {
		result.push_back(SimplifyPath(internodes[i], tolerance));
	}

	return result;
}

std::vector<std::vector<std::vector<std::vector<double>>>>
VisvalingamWhyatt::BatchSimplifyLayerInternodes(const std::vector<std::vector<std::vector<std::vector<double>>>>& layerInternodes, double tolerance)
{
	std::vector<std::vector<std::vector<std::vector<double>>>> result;
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
VisvalingamWhyatt::_IsValidPath(const std::vector<std::vector<double>>& path)
{
	if (path.empty()) return false;

	for (int i = 0; i < static_cast<int>(path.size()); i++) {
		if (path[i].size() < 2) return false;
	}

	return true;
}

std::vector<std::vector<double>>
VisvalingamWhyatt::_SimplifyClosedPath(const std::vector<std::vector<double>>& path, double tolerance)
{
	if (path.size() <= fMinPointCount) {
		return path;
	}

	std::vector<VWPoint> points;
	points.reserve(path.size());

	for (int i = 0; i < static_cast<int>(path.size()); i++) {
		points.push_back(VWPoint(path[i][0], path[i][1]));
	}

	double thresholdArea = tolerance * tolerance; // Square the tolerance for area comparison

	// Initial areas for all points
	for (int i = 0; i < static_cast<int>(points.size()); i++) {
		if (!points[i].removed) {
			int prev = (i - 1 + points.size()) % points.size();
			int next = (i + 1) % points.size();

			points[i].area = _CalculateTriangleArea(
				points[prev].x, points[prev].y,
				points[i].x, points[i].y,
				points[next].x, points[next].y
			);
		}
	}

	// Remove points iteratively
	bool foundPointToRemove = true;
	int remainingPoints = points.size();

	while (foundPointToRemove && remainingPoints > fMinPointCount) {
		foundPointToRemove = false;
		int minIndex = -1;
		double minArea = thresholdArea;

		// Find point with smallest area below threshold
		for (int i = 0; i < static_cast<int>(points.size()); i++) {
			if (!points[i].removed && points[i].area < minArea) {
				minArea = points[i].area;
				minIndex = i;
				foundPointToRemove = true;
			}
		}

		if (foundPointToRemove && minIndex != -1) {
			// Remove the point
			points[minIndex].removed = true;
			remainingPoints--;

			// Recalculate areas for neighbors
			for (int offset = -2; offset <= 2; offset++) {
				int idx = (minIndex + offset + points.size()) % points.size();
				if (!points[idx].removed) {
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
					}
				}
			}
		}
	}

	std::vector<std::vector<double>> result;
	for (int i = 0; i < static_cast<int>(points.size()); i++) {
		if (!points[i].removed) {
			std::vector<double> point(2);
			point[0] = points[i].x;
			point[1] = points[i].y;
			result.push_back(point);
		}
	}

	return result;
}

std::vector<std::vector<double>>
VisvalingamWhyatt::_SimplifyOpenPath(const std::vector<std::vector<double>>& path, double tolerance)
{
	if (path.size() <= fMinPointCount) {
		return path;
	}

	std::vector<VWPoint> points;
	points.reserve(path.size());

	for (int i = 0; i < static_cast<int>(path.size()); i++) {
		points.push_back(VWPoint(path[i][0], path[i][1]));
	}

	double thresholdArea = tolerance * tolerance; // Square the tolerance for area comparison

	// Calculate initial areas (skip first and last points)
	for (int i = 1; i < static_cast<int>(points.size()) - 1; i++) {
		points[i].area = _CalculateTriangleArea(
			points[i-1].x, points[i-1].y,
			points[i].x, points[i].y,
			points[i+1].x, points[i+1].y
		);
	}

	// Mark endpoints as non-removable
	if (!points.empty()) {
		points[0].area = 1e30;
		points[points.size() - 1].area = 1e30;
	}

	// Remove points iteratively
	bool foundPointToRemove = true;
	int remainingPoints = points.size();

	while (foundPointToRemove && remainingPoints > fMinPointCount) {
		foundPointToRemove = false;
		int minIndex = -1;
		double minArea = thresholdArea;

		// Find point with smallest area below threshold (skip endpoints)
		for (int i = 1; i < static_cast<int>(points.size()) - 1; i++) {
			if (!points[i].removed && points[i].area < minArea) {
				minArea = points[i].area;
				minIndex = i;
				foundPointToRemove = true;
			}
		}

		if (foundPointToRemove && minIndex != -1) {
			// Remove the point
			points[minIndex].removed = true;
			remainingPoints--;

			// Recalculate areas for neighbors
			for (int offset = -2; offset <= 2; offset++) {
				int idx = minIndex + offset;
				if (idx > 0 && idx < static_cast<int>(points.size()) - 1 && !points[idx].removed) {
					int prev = idx - 1;
					int next = idx + 1;

					while (prev >= 0 && points[prev].removed) prev--;
					while (next < static_cast<int>(points.size()) && points[next].removed) next++;

					if (prev >= 0 && next < static_cast<int>(points.size())) {
						points[idx].area = _CalculateTriangleArea(
							points[prev].x, points[prev].y,
							points[idx].x, points[idx].y,
							points[next].x, points[next].y
						);
					}
				}
			}
		}
	}

	std::vector<std::vector<double>> result;
	for (int i = 0; i < static_cast<int>(points.size()); i++) {
		if (!points[i].removed) {
			std::vector<double> point(2);
			point[0] = points[i].x;
			point[1] = points[i].y;
			result.push_back(point);
		}
	}

	return result;
}
