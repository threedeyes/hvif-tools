/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef VISVALINGAM_WHYATT_H
#define VISVALINGAM_WHYATT_H

#include <vector>

struct VWPoint {
	double					x, y;
	double					area;
	bool					removed;

							VWPoint() : x(0), y(0), area(0), removed(false) {}
							VWPoint(double px, double py) : x(px), y(py), area(0), removed(false) {}
};

class VisvalingamWhyatt {
public:
							VisvalingamWhyatt();
							~VisvalingamWhyatt();

	std::vector<std::vector<double> >
							SimplifyPath(const std::vector<std::vector<double> >& path, 
										double tolerance);

	std::vector<std::vector<double> >
							SimplifyPath(const std::vector<std::vector<double> >& path, 
										double tolerance,
										const std::vector<bool>* protectedPoints);

	std::vector<std::vector<std::vector<double> > >
							BatchSimplifyInternodes(const std::vector<std::vector<std::vector<double> > >& internodes,
													double tolerance);

	std::vector<std::vector<std::vector<std::vector<double> > > >
							BatchSimplifyLayerInternodes(const std::vector<std::vector<std::vector<std::vector<double> > > >& layerInternodes,
														 double tolerance);

private:
	double					_CalculateTriangleArea(double x1, double y1, double x2, double y2, double x3, double y3);
	bool					_IsValidPath(const std::vector<std::vector<double> >& path);
	std::vector<std::vector<double> >
							_SimplifyClosedPath(const std::vector<std::vector<double> >& path, double tolerance);
	std::vector<std::vector<double> >
							_SimplifyOpenPath(const std::vector<std::vector<double> >& path, double tolerance);

	double					fMinTriangleArea;
	int						fMinPointCount;
};

#endif
