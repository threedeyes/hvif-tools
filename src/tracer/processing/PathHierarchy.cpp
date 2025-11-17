/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include "PathHierarchy.h"
#include <cmath>
#include <algorithm>

PathHierarchy::PathHierarchy()
{
}

PathHierarchy::~PathHierarchy()
{
}

void
PathHierarchy::AnalyzeHierarchy(IndexedBitmap& indexed)
{
	const std::vector<std::vector<std::vector<std::vector<double> > > >& layers = indexed.Layers();

	std::vector<std::vector<IndexedBitmap::PathMetadata> > allMetadata;
	allMetadata.resize(layers.size());

	for (size_t k = 0; k < layers.size(); k++) {
		allMetadata[k].resize(layers[k].size());
		_BuildNestingTree(layers[k], allMetadata[k]);
	}

	indexed.SetPathMetadata(allMetadata);
}

void
PathHierarchy::_BuildBoundsForLayer(
	const std::vector<std::vector<std::vector<double> > >& paths,
	std::vector<PathBounds>& bounds)
{
	bounds.resize(paths.size());

	for (size_t i = 0; i < paths.size(); i++) {
		if (paths[i].empty()) continue;

		bool first = true;

		for (size_t j = 0; j < paths[i].size(); j++) {
			const std::vector<double>& seg = paths[i][j];
			if (seg.size() < 4) continue;

			double x1 = seg[1];
			double y1 = seg[2];
			double x2 = (seg[0] == 1.0) ? seg[3] : seg[5];
			double y2 = (seg[0] == 1.0) ? seg[4] : seg[6];

			if (first) {
				bounds[i].minX = bounds[i].maxX = x1;
				bounds[i].minY = bounds[i].maxY = y1;
				first = false;
			}

			if (x1 < bounds[i].minX) bounds[i].minX = x1;
			if (x1 > bounds[i].maxX) bounds[i].maxX = x1;
			if (y1 < bounds[i].minY) bounds[i].minY = y1;
			if (y1 > bounds[i].maxY) bounds[i].maxY = y1;

			if (x2 < bounds[i].minX) bounds[i].minX = x2;
			if (x2 > bounds[i].maxX) bounds[i].maxX = x2;
			if (y2 < bounds[i].minY) bounds[i].minY = y2;
			if (y2 > bounds[i].maxY) bounds[i].maxY = y2;

			if (seg[0] == 2.0 && seg.size() >= 7) {
				double cx = seg[3];
				double cy = seg[4];
				if (cx < bounds[i].minX) bounds[i].minX = cx;
				if (cx > bounds[i].maxX) bounds[i].maxX = cx;
				if (cy < bounds[i].minY) bounds[i].minY = cy;
				if (cy > bounds[i].maxY) bounds[i].maxY = cy;
			}
		}

		bounds[i].area = (bounds[i].maxX - bounds[i].minX) * (bounds[i].maxY - bounds[i].minY);
	}
}

double
PathHierarchy::_CalculateSignedArea(const std::vector<std::vector<double> >& path)
{
	if (path.empty()) return 0.0;

	double area = 0.0;

	for (size_t i = 0; i < path.size(); i++) {
		const std::vector<double>& seg = path[i];
		if (seg.size() < 4) continue;

		double x1 = seg[1];
		double y1 = seg[2];
		double x2 = (seg[0] == 1.0) ? seg[3] : seg[5];
		double y2 = (seg[0] == 1.0) ? seg[4] : seg[6];

		area += (x1 * y2 - x2 * y1);
	}

	return area * 0.5;
}

bool
PathHierarchy::_PointInPolygon(double px, double py,
	const std::vector<std::vector<double> >& polygon)
{
	bool inside = false;
	size_t n = polygon.size();

	for (size_t i = 0; i < n; i++) {
		size_t j = (i == 0) ? n - 1 : i - 1;

		const std::vector<double>& segi = polygon[i];
		const std::vector<double>& segj = polygon[j];

		if (segi.size() < 4 || segj.size() < 4) continue;

		double xi = segi[1];
		double yi = segi[2];
		double xj = segj[1];
		double yj = segj[2];

		if (((yi > py) != (yj > py)) &&
			(px < (xj - xi) * (py - yi) / (yj - yi) + xi)) {
			inside = !inside;
		}
	}

	return inside;
}

bool
PathHierarchy::_IsPathInsidePath(
	const std::vector<std::vector<double> >& innerPath,
	const std::vector<std::vector<double> >& outerPath,
	const PathBounds& innerBounds,
	const PathBounds& outerBounds)
{
	if (innerBounds.minX < outerBounds.minX - 0.5 ||
		innerBounds.maxX > outerBounds.maxX + 0.5 ||
		innerBounds.minY < outerBounds.minY - 0.5 ||
		innerBounds.maxY > outerBounds.maxY + 0.5) {
		return false;
	}

	int testCount = 0;
	int insideCount = 0;
	int maxTests = 5;

	for (size_t i = 0; i < innerPath.size() && testCount < maxTests; i++) {
		if (innerPath[i].size() < 4) continue;

		double px = innerPath[i][1];
		double py = innerPath[i][2];

		if (_PointInPolygon(px, py, outerPath)) {
			insideCount++;
		}
		testCount++;
	}

	if (testCount > 0 && insideCount >= (testCount + 1) / 2)
		return true;

	return false;
}

void
PathHierarchy::ReversePathSegments(std::vector<std::vector<double> >& path)
{
	if (path.size() < 2)
		return;

	size_t n = path.size();
	for (size_t i = 0; i < n / 2; i++) {
		std::vector<double> temp = path[i];
		path[i] = path[n - 1 - i];
		path[n - 1 - i] = temp;
	}

	for (size_t i = 0; i < n; i++) {
		if (path[i].size() < 4)
			continue;

		if (path[i][0] == 1.0 && path[i].size() >= 5) {
			double tx = path[i][1];
			double ty = path[i][2];
			path[i][1] = path[i][3];
			path[i][2] = path[i][4];
			path[i][3] = tx;
			path[i][4] = ty;
		} else if (path[i][0] == 2.0 && path[i].size() >= 7) {
			double tx = path[i][1];
			double ty = path[i][2];
			path[i][1] = path[i][5];
			path[i][2] = path[i][6];
			path[i][5] = tx;
			path[i][6] = ty;
		}
	}
}

void
PathHierarchy::_BuildNestingTree(
	const std::vector<std::vector<std::vector<double> > >& paths,
	std::vector<IndexedBitmap::PathMetadata>& metadata)
{
	int pathCount = paths.size();
	metadata.resize(pathCount);

	if (pathCount == 0)
		return;

	std::vector<PathBounds> bounds;
	_BuildBoundsForLayer(paths, bounds);

	for (int i = 0; i < pathCount; i++) {
		if (paths[i].empty())
			continue;

		int directParent = -1;
		double minParentArea = 1e30;

		for (int j = 0; j < pathCount; j++) {
			if (i == j || paths[j].empty())
				continue;

			if (bounds[j].area > bounds[i].area && bounds[j].area < minParentArea) {
				if (_IsPathInsidePath(paths[i], paths[j], bounds[i], bounds[j])) {
					directParent = j;
					minParentArea = bounds[j].area;
				}
			}
		}

		metadata[i].parentPathIndex = directParent;
	}

	for (int i = 0; i < pathCount; i++) {
		int level = 0;
		int current = i;
		int safety = 0;

		while (metadata[current].parentPathIndex != -1 && safety < 100) {
			level++;
			current = metadata[current].parentPathIndex;
			safety++;
		}

		metadata[i].nestingLevel = level;
		metadata[i].isHole = (level % 2 == 1);

		double area = _CalculateSignedArea(paths[i]);
		metadata[i].clockwise = (area < 0);
	}
}
