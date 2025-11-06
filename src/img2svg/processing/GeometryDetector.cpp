/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <cmath>
#include <algorithm>

#include "GeometryDetector.h"
#include "MathUtils.h"

static inline double _Clamp(double v, double lo, double hi)
{
	if (v < lo) return lo;
	if (v > hi) return hi;
	return v;
}

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
		return std::sqrt(deltaX * deltaX + deltaY * deltaY);
	}

	double normalLength = std::sqrt(deltaX * deltaX + deltaY * deltaY);
	return std::fabs((point[0] - lineStart[0]) * deltaY - (point[1] - lineStart[1]) * deltaX) / normalLength;
}

bool
GeometryDetector::_FindCircleCenter(const std::vector<std::vector<double> >& points,
									double& centerX, double& centerY, double& radius)
{
	if (points.size() < 3)
		return false;

	double cx = 0.0, cy = 0.0, r = 0.0;
	if (!_FitCircleKasa(points, cx, cy, r)) {
		double sumX = 0, sumY = 0;
		int n = static_cast<int>(points.size());
		for (int i = 0; i < n; i++) {
			sumX += points[i][0];
			sumY += points[i][1];
		}
		cx = sumX / n;
		cy = sumY / n;
		double radiusSum = 0;
		for (int i = 0; i < n; i++) {
			double dx = points[i][0] - cx;
			double dy = points[i][1] - cy;
			radiusSum += std::sqrt(dx * dx + dy * dy);
		}
		r = radiusSum / n;
	}

	_RefineCircleGaussNewton(points, cx, cy, r, 8);

	double maxDev = 0.0, avgDev = 0.0;
	for (size_t i = 0; i < points.size(); i++) {
		double dx = points[i][0] - cx;
		double dy = points[i][1] - cy;
		double dist = std::sqrt(dx * dx + dy * dy);
		double dev = std::fabs(dist - r);
		maxDev = std::max(maxDev, dev);
		avgDev += dev;
	}
	avgDev /= points.size();

	if (!std::isfinite(cx) || !std::isfinite(cy) || !std::isfinite(r) || r <= 0.0)
		return false;

	if (maxDev > r * 0.2)
		return false;

	centerX = cx;
	centerY = cy;
	radius  = r;
	return true;
}

bool
GeometryDetector::_FitCircleKasa(const std::vector<std::vector<double> >& points,
								 double& cx, double& cy, double& r) const
{
	int n = static_cast<int>(points.size());
	if (n < 3) return false;

	double Sx = 0.0, Sy = 0.0, Sxx = 0.0, Syy = 0.0, Sxy = 0.0, S1 = static_cast<double>(n);
	double Sxz = 0.0, Syz = 0.0, Sz = 0.0;

	for (int i = 0; i < n; i++) {
		double x = points[i][0];
		double y = points[i][1];
		double z = x * x + y * y;

		Sx  += x;
		Sy  += y;
		Sxx += x * x;
		Syy += y * y;
		Sxy += x * y;

		Sxz += x * z;
		Syz += y * z;
		Sz  += z;
	}

	double M[3][3] = {
		{ Sxx, Sxy, Sx },
		{ Sxy, Syy, Sy },
		{ Sx,  Sy,  S1 }
	};
	double B[3] = { -Sxz, -Syz, -Sz };
	double X[3] = { 0, 0, 0 };

	if (!MathUtils::Solve3x3(M, B, X))
		return false;

	double a = X[0];
	double b = X[1];
	double c = X[2];

	cx = -a / 2.0;
	cy = -b / 2.0;

	double t = cx * cx + cy * cy - c;
	if (t <= 0.0) return false;

	r = std::sqrt(t);
	if (!std::isfinite(cx) || !std::isfinite(cy) || !std::isfinite(r) || r <= 0.0)
		return false;

	return true;
}

bool
GeometryDetector::_RefineCircleGaussNewton(const std::vector<std::vector<double> >& points,
										   double& cx, double& cy, double& r,
										   int iterations) const
{
	int n = static_cast<int>(points.size());
	if (n < 3) return false;

	for (int it = 0; it < iterations; ++it) {
		double JTJ[3][3] = { {0,0,0}, {0,0,0}, {0,0,0} };
		double JTr[3] = { 0.0, 0.0, 0.0 };

		int used = 0;

		for (int i = 0; i < n; i++) {
			double xi = points[i][0];
			double yi = points[i][1];

			double dx = cx - xi;
			double dy = cy - yi;
			double ri = std::sqrt(dx * dx + dy * dy);
			if (ri < 1e-12 || !std::isfinite(ri)) continue;

			double fi = ri - r;

			double ja = dx / ri;
			double jb = dy / ri;
			double jR = -1.0;

			JTJ[0][0] += ja * ja; JTJ[0][1] += ja * jb; JTJ[0][2] += ja * jR;
			JTJ[1][0] += jb * ja; JTJ[1][1] += jb * jb; JTJ[1][2] += jb * jR;
			JTJ[2][0] += jR * ja; JTJ[2][1] += jR * jb; JTJ[2][2] += jR * jR;

			JTr[0] += ja * fi;
			JTr[1] += jb * fi;
			JTr[2] += jR * fi;

			++used;
		}

		if (used < 3) return false;

		double M[3][3] = {
			{ JTJ[0][0], JTJ[0][1], JTJ[0][2] },
			{ JTJ[1][0], JTJ[1][1], JTJ[1][2] },
			{ JTJ[2][0], JTJ[2][1], JTJ[2][2] }
		};
		double B[3] = { -JTr[0], -JTr[1], -JTr[2] };
		double X[3] = { 0, 0, 0 };

		if (!MathUtils::Solve3x3(M, B, X))
			return false;

		double dcx = X[0];
		double dcy = X[1];
		double dR  = X[2];

		cx += dcx;
		cy += dcy;
		r  += dR;

		if (!std::isfinite(cx) || !std::isfinite(cy) || !std::isfinite(r) || r <= 0.0)
			return false;

		double stepNorm = std::sqrt(dcx * dcx + dcy * dcy + dR * dR);
		if (stepNorm < 1e-6) break;
	}

	return true;
}

std::vector<std::vector<double> >
GeometryDetector::_ConvertSegmentsToPoints(const std::vector<std::vector<double> >& segments)
{
	std::vector<std::vector<double> > points;
	points.reserve(segments.size() * 5);

	for (int i = 0; i < static_cast<int>(segments.size()); i++) {
		if (segments[i].size() < 4) continue;

		if (i == 0) {
			std::vector<double> startPoint(2);
			startPoint[0] = segments[i][1];
			startPoint[1] = segments[i][2];
			points.push_back(startPoint);
		}

		if (segments[i][0] == 1.0) {
			std::vector<double> endPoint(2);
			endPoint[0] = segments[i][3];
			endPoint[1] = segments[i][4];
			points.push_back(endPoint);
		} else if (segments[i][0] == 2.0) {
			double x0 = segments[i][1], y0 = segments[i][2];
			double x1 = segments[i][3], y1 = segments[i][4];
			double x2 = segments[i][5], y2 = segments[i][6];

			double ts[3];
			ts[0] = 0.25;
			ts[1] = 0.5;
			ts[2] = 0.75;

			for (int ti = 0; ti < 3; ti++) {
				double t = ts[ti];
				double mt = 1.0 - t;
				double bx = mt * mt * x0 + 2.0 * mt * t * x1 + t * t * x2;
				double by = mt * mt * y0 + 2.0 * mt * t * y1 + t * t * y2;
				std::vector<double> quadPoint(2);
				quadPoint[0] = bx;
				quadPoint[1] = by;
				points.push_back(quadPoint);
			}

			std::vector<double> endPoint(2);
			endPoint[0] = x2;
			endPoint[1] = y2;
			points.push_back(endPoint);
		}
	}

	return points;
}

bool
GeometryDetector::_IsClosedPath(const std::vector<std::vector<double> >& points, double tolerance)
{
	if (points.size() < 4)
		return false;

	double dx = points[0][0] - points[points.size() - 1][0];
	double dy = points[0][1] - points[points.size() - 1][1];
	double distance = std::sqrt(dx * dx + dy * dy);

	return distance <= tolerance;
}

double
GeometryDetector::_SignedArea(const std::vector<std::vector<double> >& points) const
{
	if (points.size() < 3)
		return 0.0;

	double area2 = 0.0;
	int n = static_cast<int>(points.size());
	for (int i = 0; i < n; ++i) {
		int j = (i + 1) % n;
		area2 += points[i][0] * points[j][1] - points[j][0] * points[i][1];
	}
	return 0.5 * area2;
}

double
GeometryDetector::_AngleFromCenter(double cx, double cy, double x, double y) const
{
	return std::atan2(y - cy, x - cx);
}

double
GeometryDetector::_ComputeRelStdDevOfRadii(const std::vector<std::vector<double> >& points,
										   double cx, double cy, double r) const
{
	if (points.empty() || r <= 0.0) return 1.0;
	double sum = 0.0, sum2 = 0.0;
	int n = 0;
	for (size_t i = 0; i < points.size(); i++) {
		double dx = points[i][0] - cx;
		double dy = points[i][1] - cy;
		double d = std::sqrt(dx * dx + dy * dy);
		if (!std::isfinite(d)) continue;
		sum += d;
		sum2 += d * d;
		++n;
	}
	if (n < 2) return 1.0;
	double mean = sum / n;
	double var = std::max(0.0, (sum2 / n) - (mean * mean));
	double stddev = std::sqrt(var);
	return stddev / r;
}

double
GeometryDetector::_ComputeInlierRatio(const std::vector<std::vector<double> >& points,
									  double cx, double cy, double r,
									  double inlierThreshold) const
{
	if (points.empty() || r <= 0.0) return 0.0;
	double thr = std::max(1.0, inlierThreshold);
	int inliers = 0, total = 0;
	for (size_t i = 0; i < points.size(); i++) {
		double dx = points[i][0] - cx;
		double dy = points[i][1] - cy;
		double d = std::sqrt(dx * dx + dy * dy);
		if (!std::isfinite(d)) continue;
		if (std::fabs(d - r) <= thr) ++inliers;
		++total;
	}
	return total > 0 ? static_cast<double>(inliers) / total : 0.0;
}

double
GeometryDetector::_MaxAngleGap(const std::vector<std::vector<double> >& points,
							   double cx, double cy) const
{
	std::vector<double> angles;
	angles.reserve(points.size());
	for (size_t i = 0; i < points.size(); i++) {
		angles.push_back(std::atan2(points[i][1] - cy, points[i][0] - cx));
	}
	if (angles.size() < 2) return 2.0 * M_PI;

	std::sort(angles.begin(), angles.end());
	double maxGap = 0.0;
	for (size_t i = 1; i < angles.size(); ++i) {
		maxGap = std::max(maxGap, angles[i] - angles[i - 1]);
	}
	maxGap = std::max(maxGap, (angles.front() + 2.0 * M_PI) - angles.back());
	return maxGap;
}

double
GeometryDetector::_PolygonAreaAbs(const std::vector<std::vector<double> >& points) const
{
	if (points.size() < 3) return 0.0;
	double area2 = 0.0;
	int n = static_cast<int>(points.size());
	for (int i = 0; i < n; ++i) {
		int j = (i + 1) % n;
		area2 += points[i][0] * points[j][1] - points[j][0] * points[i][1];
	}
	return std::fabs(0.5 * area2);
}

bool
GeometryDetector::DetectLine(const std::vector<std::vector<double> >& path, float tolerance, Line& result)
{
	if (path.size() < 2) {
		return false;
	}

	if (path.size() == 2) {
		result = Line(path[0][0], path[0][1], path[1][0], path[1][1], 0.0);
		return true;
	}

	double maxError = 0.0;

	for (int i = 1; i < static_cast<int>(path.size()) - 1; i++) {
		double error = _PerpendicularDistance(path[i], path[0], path[path.size() - 1]);
		maxError = std::max(maxError, error);
	}

	if (maxError <= tolerance) {
		result = Line(path[0][0], path[0][1], path[path.size() - 1][0], path[path.size() - 1][1], maxError);
		return true;
	}

	return false;
}

bool
GeometryDetector::DetectCircle(const std::vector<std::vector<double> >& path,
							float tolerance, float minRadius, float maxRadius, Circle& result)
{
	if (path.size() < 6)
		return false;

	if (!_IsClosedPath(path, tolerance * 2))
		return false;

	double centerX, centerY, radius;
	if (!_FindCircleCenter(path, centerX, centerY, radius))
		return false;

	if (radius < minRadius || radius > maxRadius)
		return false;

	double maxError = 0;
	double avgError = 0;
	for (size_t i = 0; i < path.size(); i++) {
		double deltaX = path[i][0] - centerX;
		double deltaY = path[i][1] - centerY;
		double distance = std::sqrt(deltaX * deltaX + deltaY * deltaY);
		double error = std::fabs(distance - radius);
		maxError = std::max(maxError, error);
		avgError += error;
	}
	avgError /= path.size();

	if (maxError > tolerance || avgError > tolerance * 0.8)
		return false;

	double rSafe = std::max(1.0, radius);
	double s = std::min(0.25, std::max(0.0, tolerance / rSafe));

	double relMaxError = maxError / rSafe;
	double relAvgError = avgError / rSafe;
	double allowedRelMaxErr = 0.04 + 0.6 * s;
	double allowedRelAvgErr = 0.02 + 0.4 * s;
	if (relMaxError > allowedRelMaxErr || relAvgError > allowedRelAvgErr)
		return false;

	double relStd = _ComputeRelStdDevOfRadii(path, centerX, centerY, radius);
	double inlierThr = _Clamp(tolerance, 1.0, rSafe * 0.12);
	double inlierRatio = _ComputeInlierRatio(path, centerX, centerY, radius, inlierThr);
	double maxGap = _MaxAngleGap(path, centerX, centerY);

	double minX = path[0][0], maxX = path[0][0];
	double minY = path[0][1], maxY = path[0][1];
	for (size_t i = 0; i < path.size(); i++) {
		minX = std::min(minX, path[i][0]);
		maxX = std::max(maxX, path[i][0]);
		minY = std::min(minY, path[i][1]);
		maxY = std::max(maxY, path[i][1]);
	}
	double rangeX = std::max(1e-6, maxX - minX);
	double rangeY = std::max(1e-6, maxY - minY);
	double aspect = (rangeX > rangeY) ? (rangeX / rangeY) : (rangeY / rangeX);

	double areaAbs = _PolygonAreaAbs(path);
	double rArea = (areaAbs > 0.0) ? std::sqrt(areaAbs / M_PI) : radius;
	double relAreaDiff = std::fabs(rArea - radius) / rSafe;

	double allowedRelStd = std::min(0.12, 0.04 + 0.5 * s);					// 4%..12%
	double minInliers = std::max(0.65, 0.9 - 1.2 * s);						// 90%..65%
	double maxGapAllowed = M_PI * (0.9 + 0.4 * s);							// ~162°..180°
	double aspectAllowed = std::min(1.6, 1.1 + 4.0 * s + 3.0 * relStd);		// up to 1.6
	double relAreaAllowed = std::min(0.22, 0.08 + 0.5 * s + 0.5 * relStd);	// up to 22%

	if (relStd > allowedRelStd) return false;
	if (inlierRatio < minInliers) return false;
	if (maxGap > maxGapAllowed) return false;
	if (aspect > aspectAllowed) return false;
	if (relAreaDiff > relAreaAllowed) return false;

	result = Circle(centerX, centerY, radius, maxError);
	return true;
}

std::vector<std::vector<double> >
GeometryDetector::CreateLineSegment(const Line& line)
{
	std::vector<std::vector<double> > segment;

	std::vector<double> lineSegment(7);
	lineSegment[0] = 1.0;
	lineSegment[1] = line.startX;
	lineSegment[2] = line.startY;
	lineSegment[3] = line.endX;
	lineSegment[4] = line.endY;
	lineSegment[5] = 0.0;
	lineSegment[6] = 0.0;

	segment.push_back(lineSegment);
	return segment;
}

std::vector<std::vector<double> >
GeometryDetector::CreateCircleSegment(const Circle& circle, double startAngle, bool clockwise)
{
	std::vector<std::vector<double> > segments;

	int numSegments;
	if (circle.radius <= 10) {
		numSegments = 4;
	} else if (circle.radius <= 50) {
		numSegments = 6;
	} else if (circle.radius <= 100) {
		numSegments = 8;
	} else {
		numSegments = std::min(16, static_cast<int>(circle.radius / 20.0));
	}
	if (numSegments < 3) numSegments = 3;

	double angleStepAbs = 2.0 * M_PI / static_cast<double>(numSegments);
	double angleStep = clockwise ? angleStepAbs : -angleStepAbs;

	double xStart = circle.centerX + circle.radius * std::cos(startAngle);
	double yStart = circle.centerY + circle.radius * std::sin(startAngle);

	for (int i = 0; i < numSegments; i++) {
		double a1 = startAngle + i * angleStep;
		double a2 = a1 + angleStep;

		double x1 = circle.centerX + circle.radius * std::cos(a1);
		double y1 = circle.centerY + circle.radius * std::sin(a1);
		double x2 = circle.centerX + circle.radius * std::cos(a2);
		double y2 = circle.centerY + circle.radius * std::sin(a2);

		double half = 0.5 * angleStep;
		double midAngle = a1 + half;
		double c = std::cos(half);
		double controlRadius = (std::fabs(c) < 1e-6) ? circle.radius : (circle.radius / c);

		double cx = circle.centerX + controlRadius * std::cos(midAngle);
		double cy = circle.centerY + controlRadius * std::sin(midAngle);

		if (i == numSegments - 1) {
			x2 = xStart;
			y2 = yStart;
		}

		std::vector<double> segment(7);
		segment[0] = 2.0;
		segment[1] = x1;
		segment[2] = y1;
		segment[3] = cx;
		segment[4] = cy;
		segment[5] = x2;
		segment[6] = y2;

		segments.push_back(segment);
	}

	return segments;
}

std::vector<std::vector<std::vector<double> > >
GeometryDetector::BatchGeometryDetection(const std::vector<std::vector<std::vector<double> > >& paths,
										const TracingOptions& options)
{
	std::vector<std::vector<std::vector<double> > > detectedPaths;

	for (int i = 0; i < static_cast<int>(paths.size()); i++) {
		if (paths[i].empty()) {
			detectedPaths.push_back(paths[i]);
			continue;
		}

		std::vector<std::vector<double> > pathPoints = _ConvertSegmentsToPoints(paths[i]);

		if (pathPoints.size() < 3) {
			detectedPaths.push_back(paths[i]);
			continue;
		}

		double minX = pathPoints[0][0], maxX = pathPoints[0][0];
		double minY = pathPoints[0][1], maxY = pathPoints[0][1];

		for (size_t j = 0; j < pathPoints.size(); j++) {
			minX = std::min(minX, pathPoints[j][0]);
			maxX = std::max(maxX, pathPoints[j][0]);
			minY = std::min(minY, pathPoints[j][1]);
			maxY = std::max(maxY, pathPoints[j][1]);
		}

		double objectWidth = maxX - minX;
		double objectHeight = maxY - minY;
		double maxObjectSize = std::max(objectWidth, objectHeight);

		double closedTol = options.fCircleTolerance * 2.0;

		double signedArea = _SignedArea(pathPoints);
		bool clockwise = (signedArea > 0.0);

		Circle circle;
		if (_IsClosedPath(pathPoints, closedTol) && DetectCircle(pathPoints, options.fCircleTolerance,
						options.fMinCircleRadius, options.fMaxCircleRadius, circle)) {

			double startAngle = _AngleFromCenter(circle.centerX, circle.centerY,
													pathPoints.front()[0], pathPoints.front()[1]);

			double circleDiameter = circle.radius * 2.0;
			if (circleDiameter <= maxObjectSize * 1.5) {
				detectedPaths.push_back(CreateCircleSegment(circle, startAngle, clockwise));
				continue;
			}
		}

		Line line;
		if (DetectLine(pathPoints, options.fLineTolerance, line)) {
			detectedPaths.push_back(CreateLineSegment(line));
			continue;
		}

		detectedPaths.push_back(paths[i]);
	}

	return detectedPaths;
}

std::vector<std::vector<std::vector<std::vector<double> > > >
GeometryDetector::BatchLayerGeometryDetection(const std::vector<std::vector<std::vector<std::vector<double> > > >& layers,
											const TracingOptions& options)
{
	std::vector<std::vector<std::vector<std::vector<double> > > > detectedLayers;

	for (int k = 0; k < static_cast<int>(layers.size()); k++)
		detectedLayers.push_back(BatchGeometryDetection(layers[k], options));

	return detectedLayers;
}
