/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <cmath>
#include <algorithm>
#include <iostream>

#include "PathSimplifier.h"
#include "PathTracer.h"
#include "SharedEdgeRegistry.h"

PathSimplifier::PathSimplifier()
{
}

PathSimplifier::~PathSimplifier()
{
}

double
PathSimplifier::_PerpendicularDistance(const std::vector<double>& point,
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

std::vector<std::vector<double> >
PathSimplifier::DouglasPeuckerSimple(const std::vector<std::vector<double> >& path, float tolerance)
{
	if (path.size() <= 2)
		return path;

	// Find the point with maximum distance from line between start and end
	double maxDistance = 0.0;
	int index = 0;

	for (int i = 1; i < static_cast<int>(path.size()) - 1; i++) {
		double distance = _PerpendicularDistance(path[i], path[0], path[path.size() - 1]);
		if (distance > maxDistance) {
			index = i;
			maxDistance = distance;
		}
	}

	std::vector<std::vector<double> > result;

	if (maxDistance > tolerance) {
		std::vector<std::vector<double> > firstPart(path.begin(), path.begin() + index + 1);
		std::vector<std::vector<double> > firstResult = DouglasPeuckerSimple(firstPart, tolerance);

		std::vector<std::vector<double> > secondPart(path.begin() + index, path.end());
		std::vector<std::vector<double> > secondResult = DouglasPeuckerSimple(secondPart, tolerance);

		result.insert(result.end(), firstResult.begin(), firstResult.end() - 1);
		result.insert(result.end(), secondResult.begin(), secondResult.end());
	} else {
		result.push_back(path[0]);
		result.push_back(path[path.size() - 1]);
	}

	return result;
}

double
PathSimplifier::_CalculateCurvature(const std::vector<double>& prev,
									const std::vector<double>& curr,
									const std::vector<double>& next)
{
	double deltaX1 = prev[0] - curr[0];
	double deltaY1 = prev[1] - curr[1];
	double deltaX2 = next[0] - curr[0];
	double deltaY2 = next[1] - curr[1];

	double length1 = sqrt(deltaX1 * deltaX1 + deltaY1 * deltaY1);
	double length2 = sqrt(deltaX2 * deltaX2 + deltaY2 * deltaY2);

	if (length1 < 0.001 || length2 < 0.001)
		return 0.0;

	deltaX1 /= length1;
	deltaY1 /= length1;
	deltaX2 /= length2;
	deltaY2 /= length2;

	double dotProduct = deltaX1 * deltaX2 + deltaY1 * deltaY2;
	dotProduct = std::max(-1.0, std::min(1.0, dotProduct));

	return acos(dotProduct);
}

std::vector<std::vector<double> >
PathSimplifier::DouglasPeuckerWithProtection(const std::vector<std::vector<double> >& path,
											float tolerance,
											const std::vector<bool>& protectedPoints)
{
	if (path.size() <= 2 || protectedPoints.size() != path.size()) {
		return DouglasPeuckerSimple(path, tolerance);
	}

	std::vector<int> segments;
	segments.push_back(0);

	for (int i = 1; i < static_cast<int>(path.size()) - 1; i++) {
		if (protectedPoints[i]) {
			segments.push_back(i);
		}
	}
	segments.push_back(path.size() - 1);

	// Simplify each segment separately
	std::vector<std::vector<double> > result;

	for (int i = 0; i < static_cast<int>(segments.size()) - 1; i++) {
		int start = segments[i];
		int end = segments[i + 1];

		if (end - start <= 1) {
			// Single point or adjacent points
			if (result.empty() || (result.back()[0] != path[start][0] || result.back()[1] != path[start][1])) {
				result.push_back(path[start]);
			}
			continue;
		}

		std::vector<std::vector<double> > segment(path.begin() + start, path.begin() + end + 1);
		std::vector<std::vector<double> > simplified = DouglasPeuckerSimple(segment, tolerance);

		for (int j = 0; j < static_cast<int>(simplified.size()); j++) {
			if (result.empty() || 
				(result.back()[0] != simplified[j][0] || result.back()[1] != simplified[j][1])) {
				result.push_back(simplified[j]);
			}
		}
	}

	return result;
}

std::vector<std::vector<double> >
PathSimplifier::DouglasPeucker(const std::vector<std::vector<double> >& path,
							float tolerance,
							bool curveProtection,
							float curvatureThreshold)
{
	if (path.size() <= 2)
		return path;

	if (!curveProtection)
		return DouglasPeuckerSimple(path, tolerance);

	// Mark points with high curvature as protected
	std::vector<bool> protectedPoints(path.size(), false);
	protectedPoints[0] = true; // Always protect start
	protectedPoints[path.size() - 1] = true; // Always protect end

	for (int i = 1; i < static_cast<int>(path.size()) - 1; i++) {
		double curvature = _CalculateCurvature(path[i-1], path[i], path[i+1]);
		if (curvature > curvatureThreshold) {
			protectedPoints[i] = true;
		}
	}

	return DouglasPeuckerWithProtection(path, tolerance, protectedPoints);
}

std::vector<std::vector<std::vector<std::vector<double> > > >
PathSimplifier::BatchLayerDouglasPeucker(const std::vector<std::vector<std::vector<std::vector<double> > > >& layers,
										const TracingOptions& options)
{
	std::vector<std::vector<std::vector<std::vector<double> > > > simplifiedLayers;

	float tolerance = options.fDouglasPeuckerTolerance;
	bool curveProtection = (options.fDouglasPeuckerCurveProtection > 0.5f);
	float curvatureThreshold = 0.1f + (options.fDouglasPeuckerCurveProtection * 0.9f);

	for (int k = 0; k < static_cast<int>(layers.size()); k++) {
		std::vector<std::vector<std::vector<double> > > layerPaths;

		for (int i = 0; i < static_cast<int>(layers[k].size()); i++) {
			std::vector<std::vector<double> > pathPoints;

			for (int j = 0; j < static_cast<int>(layers[k][i].size()); j++) {
				const std::vector<double>& segment = layers[k][i][j];
				if (segment.size() >= 4) {
					if (pathPoints.empty()) {
						std::vector<double> start(2);
						start[0] = segment[1];
						start[1] = segment[2];
						pathPoints.push_back(start);
					}

					std::vector<double> end(2);
					if (segment[0] == 1.0) {
						end[0] = segment[3];
						end[1] = segment[4];
					} else {
						end[0] = segment[5];
						end[1] = segment[6];
					}
					pathPoints.push_back(end);
				}
			}

			if (pathPoints.size() > 2) {
				std::vector<std::vector<double> > simplified =
					DouglasPeucker(pathPoints, tolerance, curveProtection, curvatureThreshold);

				if (simplified.size() >= 2) {
					std::vector<std::vector<double> > tracedSegments;

					for (int p = 0; p < static_cast<int>(simplified.size()) - 1; p++) {
						std::vector<double> segment(7);
						segment[0] = 1.0; // linear segment
						segment[1] = simplified[p][0]; // start x
						segment[2] = simplified[p][1]; // start y
						segment[3] = simplified[p + 1][0]; // end x
						segment[4] = simplified[p + 1][1]; // end y
						segment[5] = 0.0; // control point x
						segment[6] = 0.0; // control point y
						tracedSegments.push_back(segment);
					}

					layerPaths.push_back(tracedSegments);
				}
			} else if (pathPoints.size() == 2) {
				std::vector<std::vector<double> > tracedSegments;
				std::vector<double> segment(7);
				segment[0] = 1.0; // linear
				segment[1] = pathPoints[0][0]; // start x
				segment[2] = pathPoints[0][1]; // start y
				segment[3] = pathPoints[1][0]; // end x
				segment[4] = pathPoints[1][1]; // end y
				segment[5] = 0.0;
				segment[6] = 0.0;
				tracedSegments.push_back(segment);
				layerPaths.push_back(tracedSegments);
			}
		}

		simplifiedLayers.push_back(layerPaths);
	}

	return simplifiedLayers;
}

std::vector<std::vector<double> >
PathSimplifier::MergeCollinearSegments(const std::vector<std::vector<double> >& path, float tolerance)
{
	if (path.size() < 3)
		return path;

	std::vector<std::vector<double> > result;
	result.push_back(path[0]);

	for (int i = 1; i < static_cast<int>(path.size()) - 1; i++) {
		const std::vector<double>& prev = result.back();
		const std::vector<double>& curr = path[i];
		const std::vector<double>& next = path[i + 1];

		// Check if three points are approximately collinear
		double area = fabs((curr[0] - prev[0]) * (next[1] - prev[1]) - 
						  (next[0] - prev[0]) * (curr[1] - prev[1]));

		// Calculate the base length for relative tolerance
		double baseLength = sqrt((next[0] - prev[0]) * (next[0] - prev[0]) + 
							   (next[1] - prev[1]) * (next[1] - prev[1]));

		// If points are collinear within tolerance, skip the middle point
		if (area / std::max(baseLength, 1.0) <= tolerance) {
			continue; // Skip current point, it's on the line between prev and next
		}

		result.push_back(curr);
	}

	// Always add the last point
	if (!path.empty())
		result.push_back(path.back());

	return result;
}

std::vector<std::vector<double> >
PathSimplifier::RemoveShortSegments(const std::vector<std::vector<double> >& path, float minLength)
{
	if (path.size() < 3)
		return path;

	std::vector<std::vector<double> > result;
	result.push_back(path[0]);

	for (int i = 1; i < static_cast<int>(path.size()); i++) {
		const std::vector<double>& prev = result.back();
		const std::vector<double>& curr = path[i];

		double deltaX = curr[0] - prev[0];
		double deltaY = curr[1] - prev[1];
		double distance = sqrt(deltaX * deltaX + deltaY * deltaY);

		if (distance >= minLength || i == static_cast<int>(path.size()) - 1)
			result.push_back(curr);
	}

	return result;
}

std::vector<std::vector<double> >
PathSimplifier::SmoothPath(const std::vector<std::vector<double> >& path, float smoothingFactor)
{
	if (path.size() < 3 || smoothingFactor <= 0)
		return path;

	std::vector<std::vector<double> > result = path;

	for (int iter = 0; iter < static_cast<int>(smoothingFactor * 3); iter++) {
		std::vector<std::vector<double> > smoothed = result;

		for (int i = 1; i < static_cast<int>(result.size()) - 1; i++) {
			double weight = 0.3f; // Smoothing weight
			smoothed[i][0] = (1.0f - 2*weight) * result[i][0] + 
							weight * result[i-1][0] + weight * result[i+1][0];
			smoothed[i][1] = (1.0f - 2*weight) * result[i][1] + 
							weight * result[i-1][1] + weight * result[i+1][1];
		}

		result = smoothed;
	}

	return result;
}

std::vector<bool>
PathSimplifier::_ConvertSegmentsToSharedMarks(
	const std::vector<std::vector<double> >& segments,
	const std::vector<bool>& sharedSegments)
{
	std::vector<bool> result;
	if (segments.empty()) return result;

	result.push_back(sharedSegments.empty() ? false : sharedSegments[0]);

	for (size_t i = 0; i < segments.size(); i++) {
		bool isShared = (i < sharedSegments.size()) ? sharedSegments[i] : false;
		result.push_back(isShared);
	}

	return result;
}

std::vector<std::vector<double> >
PathSimplifier::SimplifyPath(const std::vector<std::vector<double> >& path,
							  const TracingOptions& options,
							  const std::vector<bool>* protectedPoints)
{
	if (path.size() < 3) {
		return path;
	}

	std::vector<std::vector<double> > result = path;

	if (options.fMinSegmentLength > 0) {
		std::vector<std::vector<double> > temp;
		temp.push_back(result[0]);

		for (size_t i = 1; i < result.size(); i++) {
			bool isProtected = (protectedPoints && i < protectedPoints->size())
							   ? (*protectedPoints)[i] : false;

			const std::vector<double>& prev = temp.back();
			const std::vector<double>& curr = result[i];

			double deltaX = curr[0] - prev[0];
			double deltaY = curr[1] - prev[1];
			double distance = sqrt(deltaX * deltaX + deltaY * deltaY);

			if (distance >= options.fMinSegmentLength || isProtected || i == result.size() - 1) {
				temp.push_back(curr);
			}
		}
		result = temp;
	}

	if (options.fCurveSmoothing > 0) {
		std::vector<std::vector<double> > temp = result;

		for (int iter = 0; iter < static_cast<int>(options.fCurveSmoothing * 3); iter++) {
			std::vector<std::vector<double> > smoothed = temp;

			for (size_t i = 1; i < temp.size() - 1; i++) {
				bool isProtected = (protectedPoints && i < protectedPoints->size())
								   ? (*protectedPoints)[i] : false;

				if (isProtected) continue;

				double weight = 0.3f; // Smoothing weight
				smoothed[i][0] = (1.0f - 2*weight) * temp[i][0] +
								weight * temp[i-1][0] + weight * temp[i+1][0];
				smoothed[i][1] = (1.0f - 2*weight) * temp[i][1] +
								weight * temp[i-1][1] + weight * temp[i+1][1];
			}

			temp = smoothed;
		}
		result = temp;
	}

	if (options.fCollinearTolerance > 0) {
		std::vector<std::vector<double> > temp;
		temp.push_back(result[0]);

		for (size_t i = 1; i < result.size() - 1; i++) {
			bool isProtected = (protectedPoints && i < protectedPoints->size())
							   ? (*protectedPoints)[i] : false;

			if (isProtected) {
				temp.push_back(result[i]);
				continue;
			}

			const std::vector<double>& prev = temp.back();
			const std::vector<double>& curr = result[i];
			const std::vector<double>& next = result[i + 1];

			double area = fabs((curr[0] - prev[0]) * (next[1] - prev[1]) -
							  (next[0] - prev[0]) * (curr[1] - prev[1]));

			double baseLength = sqrt((next[0] - prev[0]) * (next[0] - prev[0]) +
									(next[1] - prev[1]) * (next[1] - prev[1]));

			if (area / std::max(baseLength, 1.0) > options.fCollinearTolerance) {
				temp.push_back(curr);
			}
		}

		if (!result.empty())
			temp.push_back(result.back());

		result = temp;
	}

	if (options.fDouglasPeuckerEnabled && protectedPoints) {
		bool curveProtection = (options.fDouglasPeuckerCurveProtection > 0.5f);
		float curvatureThreshold = 0.1f + (options.fDouglasPeuckerCurveProtection * 0.9f);
		float tolerance = options.fDouglasPeuckerTolerance;

		if (options.fAggressiveSimplification) {
			tolerance *= 1.5f;
		}

		result = DouglasPeuckerWithProtection(result, tolerance, *protectedPoints);
	}

	return result;
}

ObjectMetrics
PathSimplifier::CalculateObjectMetrics(const std::vector<std::vector<double> >& path)
{
	ObjectMetrics metrics;

	if (path.size() < 3)
		return metrics;

	if (!path.empty()) {
		metrics.boundingBox.minX = metrics.boundingBox.maxX = path[0][0];
		metrics.boundingBox.minY = metrics.boundingBox.maxY = path[0][1];

		for (int i = 1; i < static_cast<int>(path.size()); i++) {
			if (path[i].size() >= 2) {
				metrics.boundingBox.minX = std::min(metrics.boundingBox.minX, path[i][0]);
				metrics.boundingBox.minY = std::min(metrics.boundingBox.minY, path[i][1]);
				metrics.boundingBox.maxX = std::max(metrics.boundingBox.maxX, path[i][0]);
				metrics.boundingBox.maxY = std::max(metrics.boundingBox.maxY, path[i][1]);
			}
		}

		metrics.boundingBox.width = metrics.boundingBox.maxX - metrics.boundingBox.minX;
		metrics.boundingBox.height = metrics.boundingBox.maxY - metrics.boundingBox.minY;
	}

	metrics.area = _CalculatePathArea(path);
	metrics.perimeter = _CalculatePathPerimeter(path);

	return metrics;
}

double
PathSimplifier::_CalculatePathArea(const std::vector<std::vector<double> >& path)
{
	if (path.size() < 3)
		return 0.0;

	// Use shoelace formula to calculate polygon area
	double area = 0.0;
	int pointCount = path.size();

	for (int i = 0; i < pointCount; i++) {
		int j = (i + 1) % pointCount;
		if (path[i].size() >= 2 && path[j].size() >= 2) {
			area += path[i][0] * path[j][1];
			area -= path[j][0] * path[i][1];
		}
	}

	return fabs(area) / 2.0;
}

double
PathSimplifier::_CalculatePathPerimeter(const std::vector<std::vector<double> >& path)
{
	if (path.size() < 2)
		return 0.0;

	double perimeter = 0.0;
	int pointCount = path.size();

	for (int i = 0; i < pointCount; i++) {
		int j = (i + 1) % pointCount;
		if (path[i].size() >= 2 && path[j].size() >= 2) {
			double deltaX = path[j][0] - path[i][0];
			double deltaY = path[j][1] - path[i][1];
			perimeter += sqrt(deltaX * deltaX + deltaY * deltaY);
		}
	}

	return perimeter;
}

bool
PathSimplifier::IsObjectTooSmall(const ObjectMetrics& metrics, const TracingOptions& options)
{
	if (!options.fFilterSmallObjects)
		return false;
	
	if (metrics.area < options.fMinObjectArea)
		return true;

	if (metrics.boundingBox.width < options.fMinObjectWidth ||
		metrics.boundingBox.height < options.fMinObjectHeight) {
		return true;
	}

	if (metrics.perimeter < options.fMinObjectPerimeter)
		return true;

	return false;
}

std::vector<std::vector<std::vector<double> > >
PathSimplifier::FilterSmallObjects(const std::vector<std::vector<std::vector<double> > >& paths,
								const TracingOptions& options)
{
	if (!options.fFilterSmallObjects)
		return paths;

	std::vector<std::vector<std::vector<double> > > filteredPaths;
	int removedCount = 0;

	for (int i = 0; i < static_cast<int>(paths.size()); i++) {
		if (paths[i].empty()) {
			continue;
		}

		std::vector<std::vector<double> > pathPoints;
		
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
				if (segment[0] == 1.0) {
					end[0] = segment[3];
					end[1] = segment[4];
				} else {
					end[0] = segment[5];
					end[1] = segment[6];
				}
				pathPoints.push_back(end);
			}
		}

		if (pathPoints.size() < 3) {
			removedCount++;
			continue;
		}

		ObjectMetrics metrics = CalculateObjectMetrics(pathPoints);

		if (!IsObjectTooSmall(metrics, options)) {
			filteredPaths.push_back(paths[i]);
		} else {
			removedCount++;
		}
	}

	return filteredPaths;
}

std::vector<std::vector<std::vector<std::vector<double> > > >
PathSimplifier::BatchFilterSmallObjects(const std::vector<std::vector<std::vector<std::vector<double> > > >& layers,
										const TracingOptions& options)
{
	std::vector<std::vector<std::vector<std::vector<double> > > > filteredLayers;

	for (int k = 0; k < static_cast<int>(layers.size()); k++) {
		filteredLayers.push_back(FilterSmallObjects(layers[k], options));
	}

	return filteredLayers;
}

std::vector<std::vector<std::vector<std::vector<double> > > >
PathSimplifier::BatchTracePathsWithSimplification(
	const std::vector<std::vector<std::vector<std::vector<double> > > >& layers,
	const TracingOptions& options,
	const SharedEdgeRegistry* registry)
{
	std::vector<std::vector<std::vector<std::vector<double> > > > simplifiedLayers;

	for (int k = 0; k < static_cast<int>(layers.size()); k++) {
		std::vector<std::vector<std::vector<double> > > layerPaths;

		for (int i = 0; i < static_cast<int>(layers[k].size()); i++) {
			std::vector<bool> sharedSegs;
			if (registry) {
				registry->GetSharedSegmentMask(k, i, sharedSegs);
			}

			std::vector<std::vector<double> > pathPoints;

			for (int j = 0; j < static_cast<int>(layers[k][i].size()); j++) {
				const std::vector<double>& segment = layers[k][i][j];
				if (segment.size() >= 4) {
					if (pathPoints.empty()) {
						std::vector<double> start(2);
						start[0] = segment[1];
						start[1] = segment[2];
						pathPoints.push_back(start);
					}

					std::vector<double> end(2);
					if (segment[0] == 1.0) {
						end[0] = segment[3];
						end[1] = segment[4];
					} else {
						end[0] = segment[5];
						end[1] = segment[6];
					}
					pathPoints.push_back(end);
				}
			}

			if (pathPoints.size() >= 2) {
				std::vector<bool> protectedPoints =
					_ConvertSegmentsToSharedMarks(layers[k][i], sharedSegs);

				std::vector<std::vector<double> > simplified =
					SimplifyPath(pathPoints, options, &protectedPoints);

				if (simplified.size() >= 2) {
					PathTracer tracer;
					std::vector<std::vector<double> > tracedPath = tracer.TracePath(simplified,
										options.fLineThreshold,	options.fQuadraticThreshold);
					layerPaths.push_back(tracedPath);
				}
			}
		}

		simplifiedLayers.push_back(layerPaths);
	}

	return simplifiedLayers;
}
