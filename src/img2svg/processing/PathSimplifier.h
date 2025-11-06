/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef PATH_SIMPLIFIER_H
#define PATH_SIMPLIFIER_H

#include <vector>

#include "TracingOptions.h"

class BoundaryTracker;
class SharedEdgeRegistry;

struct ObjectMetrics {
	double                  area;
	double                  perimeter;
	struct {
		double              minX, minY, maxX, maxY;
		double              width, height;
	} boundingBox;
	
							ObjectMetrics() : area(0), perimeter(0) {
								boundingBox.minX = boundingBox.minY = 0;
								boundingBox.maxX = boundingBox.maxY = 0;
								boundingBox.width = boundingBox.height = 0;
							}
};

class PathSimplifier {
public:
							PathSimplifier();
							~PathSimplifier();

	std::vector<std::vector<double> >
							DouglasPeuckerSimple(const std::vector<std::vector<double> >& path, float tolerance);

	std::vector<std::vector<double> >
							DouglasPeuckerWithProtection(const std::vector<std::vector<double> >& path,
											float tolerance,
											const std::vector<bool>& protectedPoints);

	std::vector<std::vector<double> >
							DouglasPeucker(const std::vector<std::vector<double> >& path,
										float tolerance,
										bool curveProtection = true,
										float curvatureThreshold = 0.5f);
 
	std::vector<std::vector<std::vector<std::vector<double> > > >
							BatchLayerDouglasPeucker(const std::vector<std::vector<std::vector<std::vector<double> > > >& layers,
													const TracingOptions& options);

	std::vector<std::vector<double> >
							SimplifyPath(const std::vector<std::vector<double> >& path,
										const TracingOptions& options,
										const std::vector<bool>* protectedPoints = NULL);

	std::vector<std::vector<double> >
							MergeCollinearSegments(const std::vector<std::vector<double> >& path, float tolerance);

	std::vector<std::vector<double> >
							RemoveShortSegments(const std::vector<std::vector<double> >& path, float minLength);

	std::vector<std::vector<double> >
							SmoothPath(const std::vector<std::vector<double> >& path, float smoothingFactor);

	ObjectMetrics			CalculateObjectMetrics(const std::vector<std::vector<double> >& path);
	bool					IsObjectTooSmall(const ObjectMetrics& metrics, const TracingOptions& options);

	std::vector<std::vector<std::vector<double> > >
							FilterSmallObjects(const std::vector<std::vector<std::vector<double> > >& paths,
											const TracingOptions& options);
 
	std::vector<std::vector<std::vector<std::vector<double> > > >
							BatchFilterSmallObjects(const std::vector<std::vector<std::vector<std::vector<double> > > >& layers,
											const TracingOptions& options);

	std::vector<std::vector<std::vector<std::vector<double> > > >
							BatchTracePathsWithSimplification(
											const std::vector<std::vector<std::vector<std::vector<double> > > >& layers,
											const TracingOptions& options,
											const SharedEdgeRegistry* registry = NULL);

private:
	double					_PerpendicularDistance(const std::vector<double>& point,
												const std::vector<double>& lineStart,
												const std::vector<double>& lineEnd);

	double					_CalculateCurvature(const std::vector<double>& prev,
												const std::vector<double>& curr,
												const std::vector<double>& next);

	double					_CalculatePathArea(const std::vector<std::vector<double> >& path);
	double					_CalculatePathPerimeter(const std::vector<std::vector<double> >& path);

	std::vector<bool>		_ConvertSegmentsToSharedMarks(
								const std::vector<std::vector<double> >& segments,
								const std::vector<bool>& sharedSegments);
};

#endif
