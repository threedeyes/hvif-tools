/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <cmath>
#include <algorithm>

#include "GeometryDetector.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

GeometryDetector::GeometryDetector()
{
}

GeometryDetector::~GeometryDetector()
{
}

double
GeometryDetector::_PerpendicularDistance(const std::vector<double>& point,
										const std::vector<double>& lineStart,
										const std::vector<double>& lineEnd)
{
	double deltaX = lineEnd[0] - lineStart[0];
	double deltaY = lineEnd[1] - lineStart[1];

	if (deltaX == 0 && deltaY == 0) {
		deltaX = point[0] - lineStart[0];
		deltaY = point[1] - lineStart[1];
		return sqrt(deltaX * deltaX + deltaY * deltaY);
	}

	double normalLength = sqrt(deltaX * deltaX + deltaY * deltaY);
	return fabs((point[0] - lineStart[0]) * deltaY - (point[1] - lineStart[1]) * deltaX) / normalLength;
}

bool
GeometryDetector::DetectLine(const std::vector<std::vector<double>>& path, float tolerance, Line& result)
{
	if (path.size() < 3) {
		if (path.size() == 2) {
			result = Line(path[0][0], path[0][1], path[1][0], path[1][1], 0.0);
			return true;
		}
		return false;
	}

	double maxError = 0.0;
	const std::vector<double>& start = path[0];
	const std::vector<double>& end = path[path.size() - 1];

	for (int i = 1; i < static_cast<int>(path.size()) - 1; i++) {
		double error = _PerpendicularDistance(path[i], start, end);
		maxError = std::max(maxError, error);
	}

	if (maxError <= tolerance) {
		result = Line(start[0], start[1], end[0], end[1], maxError);
		return true;
	}

	return false;
}

bool
GeometryDetector::DetectCircle(const std::vector<std::vector<double>>& path, 
							float tolerance, float minRadius, float maxRadius, Circle& result)
{
	if (path.size() < 6) {
		return false;
	}

	double centerX = 0, centerY = 0;
	int pointCount = path.size();

	// Calculate center as average of coordinates
	for (int i = 0; i < pointCount; i++) {
		centerX += path[i][0];
		centerY += path[i][1];
	}
	centerX /= pointCount;
	centerY /= pointCount;

	// Calculate radius as average distance to center
	double radiusSum = 0;
	for (int i = 0; i < pointCount; i++) {
		double deltaX = path[i][0] - centerX;
		double deltaY = path[i][1] - centerY;
		radiusSum += sqrt(deltaX * deltaX + deltaY * deltaY);
	}
	double radius = radiusSum / pointCount;

	if (radius < minRadius || radius > maxRadius)
		return false;

	// Calculate approximation error
	double maxError = 0;
	for (int i = 0; i < pointCount; i++) {
		double deltaX = path[i][0] - centerX;
		double deltaY = path[i][1] - centerY;
		double distance = sqrt(deltaX * deltaX + deltaY * deltaY);
		double error = fabs(distance - radius);
		maxError = std::max(maxError, error);
	}

	if (maxError <= tolerance) {
		result = Circle(centerX, centerY, radius, maxError);
		return true;
	}

	return false;
}

std::vector<std::vector<double>>
GeometryDetector::CreateLineSegment(const Line& line)
{
	std::vector<std::vector<double>> segment;

	std::vector<double> lineSegment(7);
	lineSegment[0] = 1.0; // linear type
	lineSegment[1] = line.startX;
	lineSegment[2] = line.startY;
	lineSegment[3] = line.endX;
	lineSegment[4] = line.endY;
	lineSegment[5] = 0.0;
	lineSegment[6] = 0.0;

	segment.push_back(lineSegment);
	return segment;
}

std::vector<std::vector<double>>
GeometryDetector::CreateCircleSegment(const Circle& circle)
{
	std::vector<std::vector<double>> segments;

	int numSegments = std::max(4, std::min(16, static_cast<int>(circle.radius / 5.0)));
	double angleStep = 2.0 * M_PI / numSegments;

	for (int i = 0; i < numSegments; i++) {
		double angle1 = i * angleStep;
		double angle2 = (i + 1) * angleStep;

		double x1 = circle.centerX + circle.radius * cos(angle1);
		double y1 = circle.centerY + circle.radius * sin(angle1);
		double x2 = circle.centerX + circle.radius * cos(angle2);
		double y2 = circle.centerY + circle.radius * sin(angle2);

		double midAngle = (angle1 + angle2) / 2.0;
		double controlRadius = circle.radius / cos(angleStep / 2.0);
		double controlX = circle.centerX + controlRadius * cos(midAngle);
		double controlY = circle.centerY + controlRadius * sin(midAngle);

		std::vector<double> segment(7);
		segment[0] = 2.0; // quadratic type
		segment[1] = x1;
		segment[2] = y1;
		segment[3] = controlX;
		segment[4] = controlY;
		segment[5] = x2;
		segment[6] = y2;

		segments.push_back(segment);
	}

	return segments;
}

std::vector<std::vector<std::vector<double>>>
GeometryDetector::BatchGeometryDetection(const std::vector<std::vector<std::vector<double>>>& paths,
										const TracingOptions& options)
{
	std::vector<std::vector<std::vector<double>>> detectedPaths;

	for (int i = 0; i < static_cast<int>(paths.size()); i++) {
		if (paths[i].empty()) {
			detectedPaths.push_back(paths[i]);
			continue;
		}

		// Convert traced segments back to points for analysis
		std::vector<std::vector<double>> pathPoints;
		for (int j = 0; j < static_cast<int>(paths[i].size()); j++) {
			const std::vector<double>& segment = paths[i][j];
			if (segment.size() >= 4) {
				if (pathPoints.empty()) {
					std::vector<double> start(2);
					start[0] = segment[1];
					start[1] = segment[2];
					pathPoints.push_back(start);
				}

				std::vector<double> end(2);
				if (segment[0] == 1.0) { // linear
					end[0] = segment[3];
					end[1] = segment[4];
				} else { // quadratic
					end[0] = segment[5];
					end[1] = segment[6];
				}
				pathPoints.push_back(end);
			}
		}

		if (pathPoints.size() < 3) {
			detectedPaths.push_back(paths[i]);
			continue;
		}

		// Try to detect line first
		Line line;
		if (DetectLine(pathPoints, options.fLineTolerance, line)) {
			detectedPaths.push_back(CreateLineSegment(line));
			continue;
		}

		// Try to detect circle
		Circle circle;
		if (DetectCircle(pathPoints, options.fCircleTolerance, 
						options.fMinCircleRadius, options.fMaxCircleRadius, circle)) {
			detectedPaths.push_back(CreateCircleSegment(circle));
			continue;
		}

		// Keep original path if no geometry detected
		detectedPaths.push_back(paths[i]);
	}

	return detectedPaths;
}

std::vector<std::vector<std::vector<std::vector<double>>>>
GeometryDetector::BatchLayerGeometryDetection(const std::vector<std::vector<std::vector<std::vector<double>>>>& layers,
											const TracingOptions& options)
{
	std::vector<std::vector<std::vector<std::vector<double>>>> detectedLayers;
	
	for (int k = 0; k < static_cast<int>(layers.size()); k++) {
		detectedLayers.push_back(BatchGeometryDetection(layers[k], options));
	}

	return detectedLayers;
}
