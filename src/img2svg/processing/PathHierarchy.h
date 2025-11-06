/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef PATH_HIERARCHY_H
#define PATH_HIERARCHY_H

#include <vector>

#include "IndexedBitmap.h"

class PathHierarchy {
public:
							PathHierarchy();
							~PathHierarchy();

	void					AnalyzeHierarchy(IndexedBitmap& indexed);
	void					ReversePathSegments(std::vector<std::vector<double> >& path);

private:
	struct PathBounds {
		double minX, minY, maxX, maxY;
		double area;
		
		PathBounds() : minX(0), minY(0), maxX(0), maxY(0), area(0) {}
	};

	void					_BuildBoundsForLayer(
								const std::vector<std::vector<std::vector<double> > >& paths,
								std::vector<PathBounds>& bounds);

	bool					_IsPathInsidePath(
								const std::vector<std::vector<double> >& innerPath,
								const std::vector<std::vector<double> >& outerPath,
								const PathBounds& innerBounds,
								const PathBounds& outerBounds);

	bool					_PointInPolygon(double x, double y,
								const std::vector<std::vector<double> >& polygon);

	double					_CalculateSignedArea(
								const std::vector<std::vector<double> >& path);

	void					_BuildNestingTree(
								const std::vector<std::vector<std::vector<double> > >& paths,
								std::vector<IndexedBitmap::PathMetadata>& metadata);
};

#endif
