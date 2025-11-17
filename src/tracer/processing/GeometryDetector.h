/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef GEOMETRY_DETECTOR_H
#define GEOMETRY_DETECTOR_H

#include <vector>

#include "TracingOptions.h"

struct Circle {
	double					centerX, centerY, radius;
	double					error;

							Circle() : centerX(0), centerY(0), radius(0), error(0) {}
							Circle(double x, double y, double r, double e) 
								: centerX(x), centerY(y), radius(r), error(e) {}
};

struct Line {
	double					startX, startY, endX, endY;
	double					error;

							Line() : startX(0), startY(0), endX(0), endY(0), error(0) {}
							Line(double x1, double y1, double x2, double y2, double e)
								: startX(x1), startY(y1), endX(x2), endY(y2), error(e) {}
};

class GeometryDetector {
public:
							GeometryDetector();
							~GeometryDetector();

	bool					DetectLine(const std::vector<std::vector<double> >& path,
									float tolerance, Line& result);

	bool					DetectCircle(const std::vector<std::vector<double> >& path,
									float tolerance, float minRadius, float maxRadius, 
									Circle& result);
   
	std::vector<std::vector<double> >
							CreateLineSegment(const Line& line);

	std::vector<std::vector<double> >
							CreateCircleSegment(const Circle& circle, double startAngle, bool clockwise);

	std::vector<std::vector<std::vector<double> > >
							BatchGeometryDetection(const std::vector<std::vector<std::vector<double> > >& paths,
												const TracingOptions& options);
  
	std::vector<std::vector<std::vector<std::vector<double> > > >
							BatchLayerGeometryDetection(const std::vector<std::vector<std::vector<std::vector<double> > > >& layers,
												const TracingOptions& options);

private:
	double					_PerpendicularDistance(const std::vector<double>& point,
												const std::vector<double>& lineStart,
												const std::vector<double>& lineEnd);

	bool					_FindCircleCenter(const std::vector<std::vector<double> >& points,
											double& centerX, double& centerY, double& radius);

	std::vector<std::vector<double> >
							_ConvertSegmentsToPoints(const std::vector<std::vector<double> >& segments);

	bool					_IsClosedPath(const std::vector<std::vector<double> >& points, double tolerance = 2.0);

	double					_SignedArea(const std::vector<std::vector<double> >& points) const;
	double					_AngleFromCenter(double cx, double cy, double x, double y) const;

	bool					_FitCircleKasa(const std::vector<std::vector<double> >& points,
											double& cx, double& cy, double& r) const;

	bool					_RefineCircleGaussNewton(const std::vector<std::vector<double> >& points,
													double& cx, double& cy, double& r,
													int iterations = 5) const;

	bool					_Solve3x3(double M[3][3], double B[3], double X[3]) const;

	double					_ComputeRelStdDevOfRadii(const std::vector<std::vector<double> >& points,
													double cx, double cy, double r) const;

	double					_ComputeInlierRatio(const std::vector<std::vector<double> >& points,
												double cx, double cy, double r,
												double inlierThreshold) const;

	double					_MaxAngleGap(const std::vector<std::vector<double> >& points,
										double cx, double cy) const;

	double					_PolygonAreaAbs(const std::vector<std::vector<double> >& points) const;
};

#endif
