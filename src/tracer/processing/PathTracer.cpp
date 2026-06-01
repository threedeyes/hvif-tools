/*
 * Copyright 2025-2026, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <cmath>
#include "PathTracer.h"
#include "SharedEdgeRegistry.h"

PathTracer::PathTracer()
{
}

PathTracer::~PathTracer()
{
}

std::vector<std::vector<double> >
PathTracer::TracePath(const std::vector<std::vector<double> >& path, float lineThreshold, float quadraticThreshold)
{
    std::vector<std::vector<double> > segments;
    int pathLength = path.size();

    if (pathLength < 3) {
	if (pathLength == 2) {
	    std::vector<double> segment(7);
	    segment[0] = 1.0;
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

    return _FitSequence(path, lineThreshold, quadraticThreshold, 0, pathLength, 0, NULL);
}

std::vector<std::vector<std::vector<std::vector<double> > > >
PathTracer::BatchTracePaths(const std::vector<std::vector<std::vector<std::vector<double> > > >& internodePaths,
			    float lineThreshold, float quadraticThreshold, SharedEdgeRegistry* registry)
{
    std::vector<std::vector<std::vector<std::vector<double> > > > tracedLayers(internodePaths.size());
    for (size_t k = 0; k < internodePaths.size(); k++) {
	for (size_t i = 0; i < internodePaths[k].size(); i++) {
	    if (!internodePaths[k][i].empty()) {
		tracedLayers[k].push_back(_FitSequence(internodePaths[k][i], lineThreshold, quadraticThreshold, 0, internodePaths[k][i].size(), 0, registry));
	    } else {
		tracedLayers[k].push_back(std::vector<std::vector<double> >());
	    }
	}
    }
    return tracedLayers;
}

std::vector<std::vector<double> >
PathTracer::_FitSequence(const std::vector<std::vector<double> >& path,
			float lineThreshold, float quadraticThreshold,
			int sequenceStart, int sequenceEnd, int depth, SharedEdgeRegistry* registry)
{
    std::vector<std::vector<double> > segment;
    int pathLength = path.size();

    if (sequenceStart < 0 || sequenceEnd <= sequenceStart || sequenceStart >= pathLength)
	return segment;

    if (sequenceEnd > pathLength)
	sequenceEnd = pathLength;

    if ((sequenceEnd - sequenceStart) < 2)
	return segment;

    bool isClosed = false;
    if (sequenceStart == 0 && sequenceEnd == pathLength) {
	double dx = path[0][0] - path[pathLength-1][0];
	double dy = path[0][1] - path[pathLength-1][1];
	if (dx*dx + dy*dy < 1.0) {
	    isClosed = true;
	}
    }

    int errorPoint = sequenceStart;
    bool curvePass = true;
    double pointX, pointY, distance2, errorValue = 0;
    double totalLength = static_cast<double>(sequenceEnd - sequenceStart);
    double velocityX = (path[(sequenceEnd - 1) % pathLength][0] - path[sequenceStart][0]) / totalLength;
    double velocityY = (path[(sequenceEnd - 1) % pathLength][1] - path[sequenceStart][1]) / totalLength;

    for (int pointIndex = sequenceStart + 1; pointIndex < sequenceEnd - 1; pointIndex++) {
	double pointLength = pointIndex - sequenceStart;
	pointX = path[sequenceStart][0] + (velocityX * pointLength);
	pointY = path[sequenceStart][1] + (velocityY * pointLength);
	distance2 = ((path[pointIndex][0] - pointX) * (path[pointIndex][0] - pointX)) +
		   ((path[pointIndex][1] - pointY) * (path[pointIndex][1] - pointY));
	if (distance2 > lineThreshold) {
	    curvePass = false;
	}
	if (distance2 > errorValue + 1e-6) {
	    errorPoint = pointIndex;
	    errorValue = distance2;
	} else if (std::fabs(distance2 - errorValue) <= 1e-6) {
	    if (path[pointIndex][0] < path[errorPoint][0] || 
	       (std::fabs(path[pointIndex][0] - path[errorPoint][0]) < 1e-6 && path[pointIndex][1] < path[errorPoint][1])) {
		errorPoint = pointIndex;
	    }
	}
    }

    if (curvePass) {
	std::vector<double> thisSegment(7);
	thisSegment[0] = 1.0;
	thisSegment[1] = path[sequenceStart][0];
	thisSegment[2] = path[sequenceStart][1];
	thisSegment[3] = path[sequenceEnd - 1][0];
	thisSegment[4] = path[sequenceEnd - 1][1];

	if (isClosed) {
	    thisSegment[3] = thisSegment[1];
	    thisSegment[4] = thisSegment[2];
	}

	thisSegment[5] = 0.0;
	thisSegment[6] = 0.0;
	segment.push_back(thisSegment);
	return segment;
    }

    if ((sequenceEnd - sequenceStart) < 4) {
	int midPoint = (sequenceStart + sequenceEnd) / 2;
	std::vector<std::vector<double> > result1 = _FitSequence(path, lineThreshold, quadraticThreshold, sequenceStart, midPoint + 1, depth + 1, registry);
	std::vector<std::vector<double> > result2 = _FitSequence(path, lineThreshold, quadraticThreshold, midPoint, sequenceEnd, depth + 1, registry);
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

    if (std::fabs(t2) < 0.001) {
	int midPoint = (sequenceStart + sequenceEnd) / 2;
	std::vector<std::vector<double> > result1 = _FitSequence(path, lineThreshold, quadraticThreshold, sequenceStart, midPoint + 1, depth + 1, registry);
	std::vector<std::vector<double> > result2 = _FitSequence(path, lineThreshold, quadraticThreshold, midPoint, sequenceEnd, depth + 1, registry);
	segment.insert(segment.end(), result1.begin(), result1.end());
	segment.insert(segment.end(), result2.begin(), result2.end());
	return segment;
    }

    double controlPointX = (((t1 * path[sequenceStart][0]) + (t3 * path[sequenceEnd - 1][0])) - path[fitPoint][0]) / (-t2);
    double controlPointY = (((t1 * path[sequenceStart][1]) + (t3 * path[sequenceEnd - 1][1])) - path[fitPoint][1]) / (-t2);

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

	if (distance2 > errorValue + 1e-6) {
	    errorPoint = pointIndex;
	    errorValue = distance2;
	} else if (std::fabs(distance2 - errorValue) <= 1e-6) {
	    if (path[pointIndex][0] < path[errorPoint][0] || 
	       (std::fabs(path[pointIndex][0] - path[errorPoint][0]) < 1e-6 && path[pointIndex][1] < path[errorPoint][1])) {
		errorPoint = pointIndex;
	    }
	}
    }

    if (curvePass) {
	double cx = controlPointX;
	double cy = controlPointY;

	if (registry) {
	    if (!registry->LoadCurve(path[sequenceStart][0], path[sequenceStart][1], path[sequenceEnd - 1][0], path[sequenceEnd - 1][1], cx, cy)) {
		registry->SaveCurve(path[sequenceStart][0], path[sequenceStart][1], path[sequenceEnd - 1][0], path[sequenceEnd - 1][1], cx, cy);
	    }
	}

	std::vector<double> thisSegment(7);
	thisSegment[0] = 2.0;
	thisSegment[1] = path[sequenceStart][0];
	thisSegment[2] = path[sequenceStart][1];
	thisSegment[3] = cx;
	thisSegment[4] = cy;
	thisSegment[5] = path[sequenceEnd - 1][0];
	thisSegment[6] = path[sequenceEnd - 1][1];
	segment.push_back(thisSegment);
	return segment;
    }

    if (depth > 50) {
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

    std::vector<std::vector<double> > result1 = _FitSequence(path, lineThreshold, quadraticThreshold, sequenceStart, splitPoint + 1, depth + 1, registry);
    std::vector<std::vector<double> > result2 = _FitSequence(path, lineThreshold, quadraticThreshold, splitPoint, sequenceEnd, depth + 1, registry);

    segment.insert(segment.end(), result1.begin(), result1.end());
    segment.insert(segment.end(), result2.begin(), result2.end());
    return segment;
}
