/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <cmath>

#include "PathTracer.h"

int PathTracer::sRecursionDepth = 0;

PathTracer::PathTracer()
{
}

PathTracer::~PathTracer()
{
}

std::vector<std::vector<double>>
PathTracer::TracePath(const std::vector<std::vector<double>>& path, float lineThreshold, float quadraticThreshold)
{
	std::vector<std::vector<double>> segments;
	int pathLength = path.size();

	if (pathLength < 3) {
		if (pathLength == 2) {
			std::vector<double> segment(7);
			segment[0] = 1.0; // linear
			segment[1] = path[0][0];
			segment[2] = path[0][1];
			segment[3] = path[1][0];
			segment[4] = path[1][1];
			segment[5] = 0.0;
			segment[6] = 0.0;
			segments.push_back(segment);
		}
		return segments;
	}

	return _FitSequence(path, lineThreshold, quadraticThreshold, 0, pathLength);
}

std::vector<std::vector<std::vector<double>>>
PathTracer::BatchTracePaths(const std::vector<std::vector<std::vector<double>>>& internodePaths,
							float lineThreshold, float quadraticThreshold)
{
	std::vector<std::vector<std::vector<double>>> tracedPaths;
	for (int k = 0; k < static_cast<int>(internodePaths.size()); k++) {
		if (!internodePaths[k].empty()) {
			tracedPaths.push_back(TracePath(internodePaths[k], lineThreshold, quadraticThreshold));
		} else {
			tracedPaths.push_back(std::vector<std::vector<double>>());
		}
	}
	return tracedPaths;
}

std::vector<std::vector<double>>
PathTracer::_FitSequence(const std::vector<std::vector<double>>& path,
						float lineThreshold, float quadraticThreshold,
						int sequenceStart, int sequenceEnd)
{
	std::vector<std::vector<double>> segment;
	int pathLength = path.size();

	if (sequenceStart < 0 || sequenceEnd <= sequenceStart || sequenceStart >= pathLength)
		return segment;

	if (sequenceEnd > pathLength)
		sequenceEnd = pathLength;

	if ((sequenceEnd - sequenceStart) < 2)
		return segment;

	int errorPoint = sequenceStart;
	bool curvePass = true;
	double pointX, pointY, distance2, errorValue = 0;
	double totalLength = static_cast<double>(sequenceEnd - sequenceStart);
	double velocityX = (path[(sequenceEnd - 1) % pathLength][0] - path[sequenceStart][0]) / totalLength;
	double velocityY = (path[(sequenceEnd - 1) % pathLength][1] - path[sequenceStart][1]) / totalLength;

	// Try to fit straight line
	for (int pointIndex = sequenceStart + 1; pointIndex < sequenceEnd - 1; pointIndex++) {
		double pointLength = pointIndex - sequenceStart;
		pointX = path[sequenceStart][0] + (velocityX * pointLength);
		pointY = path[sequenceStart][1] + (velocityY * pointLength);
		distance2 = ((path[pointIndex][0] - pointX) * (path[pointIndex][0] - pointX)) +
				   ((path[pointIndex][1] - pointY) * (path[pointIndex][1] - pointY));
		if (distance2 > lineThreshold) {
			curvePass = false;
		}
		if (distance2 > errorValue) {
			errorPoint = pointIndex;
			errorValue = distance2;
		}
	}

	// Return straight line if it fits
	if (curvePass) {
		std::vector<double> thisSegment(7);
		thisSegment[0] = 1.0; // linear
		thisSegment[1] = path[sequenceStart][0];
		thisSegment[2] = path[sequenceStart][1];
		thisSegment[3] = path[sequenceEnd - 1][0];
		thisSegment[4] = path[sequenceEnd - 1][1];
		thisSegment[5] = 0.0;
		thisSegment[6] = 0.0;
		segment.push_back(thisSegment);
		return segment;
	}

	// Try quadratic spline if sequence is long enough
	if ((sequenceEnd - sequenceStart) < 4) {
		int midPoint = (sequenceStart + sequenceEnd) / 2;
		std::vector<std::vector<double>> result1 = _FitSequence(path, lineThreshold, quadraticThreshold, sequenceStart, midPoint + 1);
		std::vector<std::vector<double>> result2 = _FitSequence(path, lineThreshold, quadraticThreshold, midPoint, sequenceEnd);
		segment.insert(segment.end(), result1.begin(), result1.end());
		segment.insert(segment.end(), result2.begin(), result2.end());
		return segment;
	}

	int fitPoint = errorPoint;
	curvePass = true;
	errorValue = 0;

	double t = static_cast<double>(fitPoint - sequenceStart) / totalLength;
	double t1 = (1.0 - t) * (1.0 - t);
	double t2 = 2.0 * (1.0 - t) * t;
	double t3 = t * t;

	if (fabs(t2) < 0.001) {
		int midPoint = (sequenceStart + sequenceEnd) / 2;
		std::vector<std::vector<double>> result1 = _FitSequence(path, lineThreshold, quadraticThreshold, sequenceStart, midPoint + 1);
		std::vector<std::vector<double>> result2 = _FitSequence(path, lineThreshold, quadraticThreshold, midPoint, sequenceEnd);
		segment.insert(segment.end(), result1.begin(), result1.end());
		segment.insert(segment.end(), result2.begin(), result2.end());
		return segment;
	}

	double controlPointX = (((t1 * path[sequenceStart][0]) + (t3 * path[sequenceEnd - 1][0])) - path[fitPoint][0]) / (-t2);
	double controlPointY = (((t1 * path[sequenceStart][1]) + (t3 * path[sequenceEnd - 1][1])) - path[fitPoint][1]) / (-t2);

	// Check every point against quadratic curve
	for (int pointIndex = sequenceStart + 1; pointIndex < sequenceEnd - 1; pointIndex++) {
		t = static_cast<double>(pointIndex - sequenceStart) / totalLength;
		t1 = (1.0 - t) * (1.0 - t);
		t2 = 2.0 * (1.0 - t) * t;
		t3 = t * t;
		pointX = (t1 * path[sequenceStart][0]) + (t2 * controlPointX) + (t3 * path[sequenceEnd - 1][0]);
		pointY = (t1 * path[sequenceStart][1]) + (t2 * controlPointY) + (t3 * path[sequenceEnd - 1][1]);

		distance2 = ((path[pointIndex][0] - pointX) * (path[pointIndex][0] - pointX)) +
				   ((path[pointIndex][1] - pointY) * (path[pointIndex][1] - pointY));

		if (distance2 > quadraticThreshold)
			curvePass = false;

		if (distance2 > errorValue) {
			errorPoint = pointIndex;
			errorValue = distance2;
		}
	}

	// Return spline if it fits
	if (curvePass) {
		std::vector<double> thisSegment(7);
		thisSegment[0] = 2.0; // quadratic
		thisSegment[1] = path[sequenceStart][0];
		thisSegment[2] = path[sequenceStart][1];
		thisSegment[3] = controlPointX;
		thisSegment[4] = controlPointY;
		thisSegment[5] = path[sequenceEnd - 1][0];
		thisSegment[6] = path[sequenceEnd - 1][1];
		segment.push_back(thisSegment);
		return segment;
	}

	// Split sequence and recursively apply fitting
	if (sRecursionDepth > 50) {
		std::vector<double> thisSegment(7);
		thisSegment[0] = 1.0;
		thisSegment[1] = path[sequenceStart][0];
		thisSegment[2] = path[sequenceStart][1];
		thisSegment[3] = path[sequenceEnd - 1][0];
		thisSegment[4] = path[sequenceEnd - 1][1];
		thisSegment[5] = 0.0;
		thisSegment[6] = 0.0;
		segment.push_back(thisSegment);
		return segment;
	}

	int splitPoint = (sequenceStart + errorPoint) / 2;
	if (splitPoint <= sequenceStart) splitPoint = sequenceStart + 1;
	if (splitPoint >= sequenceEnd - 1) splitPoint = sequenceEnd - 2;

	if (splitPoint <= sequenceStart || splitPoint >= sequenceEnd - 1) {
		std::vector<double> thisSegment(7);
		thisSegment[0] = 1.0;
		thisSegment[1] = path[sequenceStart][0];
		thisSegment[2] = path[sequenceStart][1];
		thisSegment[3] = path[sequenceEnd - 1][0];
		thisSegment[4] = path[sequenceEnd - 1][1];
		thisSegment[5] = 0.0;
		thisSegment[6] = 0.0;
		segment.push_back(thisSegment);
		return segment;
	}

	sRecursionDepth++;
	std::vector<std::vector<double>> result1 = _FitSequence(path, lineThreshold, quadraticThreshold, sequenceStart, splitPoint + 1);
	std::vector<std::vector<double>> result2 = _FitSequence(path, lineThreshold, quadraticThreshold, splitPoint, sequenceEnd);
	sRecursionDepth--;

	segment.insert(segment.end(), result1.begin(), result1.end());
	segment.insert(segment.end(), result2.begin(), result2.end());
	return segment;
}
